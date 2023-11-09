#include <exports.h>

// Project
#include <internal/tap_protocol_thread.h>
#include <internal/utils.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstdlib>
#include <cstring>

// ----------------------------------------------
// Core Bindings:

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_initializeLibrary() {
    if (g_protocolThread == nullptr) {
        g_protocolThread.reset(TapProtocolThread::createNew());

        if (g_protocolThread == nullptr) {
            return CKTapInterfaceErrorCode::threadAllocationFailed;
        }
    }
    return CKTapInterfaceErrorCode::success;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_newOperation() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }
    if (g_protocolThread->isThreadActive()) {
        return CKTapInterfaceErrorCode::threadAlreadyInUse;
    }

    return g_protocolThread->reset();
}

FFI_PLUGIN_EXPORT CKTapOperationResponse Core_endOperation() {
    if (g_protocolThread == nullptr) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::libraryNotInitialized);
    }
    if (!g_protocolThread->hasStarted()) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::threadNotYetStarted);
    }
    if (g_protocolThread->isThreadActive()) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::operationStillInProgress);
    }
    if (g_protocolThread->getRecentErrorCode() == CKTapInterfaceErrorCode::pending) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::threadNotYetFinalized);
    }
    if (g_protocolThread->hasFailed() || !g_protocolThread->isTapsigner().has_value()) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::operationFailed);
    }

    auto response = makeTapOperationResponse(g_protocolThread->getRecentErrorCode());
    auto index = invalidIndex;
    if (g_protocolThread->isTapsigner().value()) {
        auto tapsigner = g_protocolThread->releaseTapsigner();
        if (!tapsigner) {
            return makeTapOperationResponse(CKTapInterfaceErrorCode::expectedTapsignerButReceivedNothing);
        }

        index = updateVectorWithTapCard(g_tapsigners, tapsigner);
        response.handle.type = CKTapCardType::tapsigner;
    } else {
        auto satscard = g_protocolThread->releaseSatscard();
        if (!satscard) {
            return makeTapOperationResponse(CKTapInterfaceErrorCode::expectedSatscardButReceivedNothing);
        }

        index = updateVectorWithTapCard(g_satscards, satscard);
        response.handle.type = CKTapCardType::satscard;
    }

    if (index == invalidIndex) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::invalidHandlingOfTapCardDuringFinalization);
    }

    response.handle.index = static_cast<int32_t>(index);
    return response;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_requestCancelOperation() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }

    g_protocolThread->requestCancel();
    return CKTapInterfaceErrorCode::success;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_beginAsyncHandshake() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }
    if (g_protocolThread->hasStarted()) {
        return CKTapInterfaceErrorCode::threadNotResetForHandshake;
    }

    if (!g_protocolThread->beginCardHandshake()) {
        // The thread failed to start so we should diagnose why
        return g_protocolThread->finalizeOperation() ?
            g_protocolThread->getRecentErrorCode() :
            CKTapInterfaceErrorCode::unknownErrorDuringHandshake;
    }

    return CKTapInterfaceErrorCode::success;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_finalizeAsyncAction() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }
    if (!g_protocolThread->hasStarted()) {
        return CKTapInterfaceErrorCode::threadNotYetStarted;
    }
    if (g_protocolThread->isThreadActive()) {
        return CKTapInterfaceErrorCode::attemptToFinalizeActiveThread;
    }

    return g_protocolThread->finalizeOperation() ?
        g_protocolThread->getRecentErrorCode() :
        CKTapInterfaceErrorCode::unableToFinalizeAsyncAction;
}

FFI_PLUGIN_EXPORT const uint8_t* Core_getTransportRequestPointer() {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }

    const auto optionalBytes = g_protocolThread->getTransportRequest();
    return optionalBytes.has_value() ? optionalBytes.value()->data() : nullptr;
}

FFI_PLUGIN_EXPORT int32_t Core_getTransportRequestLength() {
    if (g_protocolThread == nullptr) {
        return 0;
    }

    const auto optionalBytes = g_protocolThread->getTransportRequest();
    return optionalBytes.has_value() ? static_cast<int32_t>(optionalBytes.value()->size()) : 0;
}

FFI_PLUGIN_EXPORT uint8_t* Core_allocateTransportResponseBuffer(int32_t sizeInBytes) {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }
    if (sizeInBytes <= 0) {
        return nullptr;
    }

    auto optionalBuffer = g_protocolThread->allocateTransportResponseBuffer(static_cast<size_t>(sizeInBytes));
    return optionalBuffer.value_or(nullptr);
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_finalizeTransportResponse() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::threadNotYetStarted;
    }
    if (g_protocolThread->getState() != CKTapThreadState::transportRequestReady) {
        return CKTapInterfaceErrorCode::threadNotReadyForResponse;
    }

    return g_protocolThread->finalizeTransportResponse() ?
        CKTapInterfaceErrorCode::success :
        CKTapInterfaceErrorCode::threadResponseFinalizationFailed;
}

FFI_PLUGIN_EXPORT CKTapThreadState Core_getThreadState() {
    if (g_protocolThread == nullptr) {
        return CKTapThreadState::notStarted;
    }

    return g_protocolThread->getState();
}

FFI_PLUGIN_EXPORT CKTapProtoException Core_getTapProtoException() {
    if (g_protocolThread != nullptr) {
        auto e = CKTapProtoException{ };
        if (g_protocolThread->getTapProtocolException(e)) {
            return e;
        }
    }

    return { };
}

// ----------------------------------------------
// CKTapCard:

FFI_PLUGIN_EXPORT char* CKTapCard_getIdentCString(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard, char*>(handle, type, nullptr, [](const auto& card) {
            return allocateCStringFromCpp(card.GetIdent());
        }
    );
}

FFI_PLUGIN_EXPORT char* CKTapCard_getAppletVersionCString(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard, char*>(handle, type, nullptr, [](const auto& card) {
            return allocateCStringFromCpp(card.GetAppletVersion());
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_getBirthHeight(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.GetBirthHeight();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_isTestnet(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsTestnet() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_getAuthDelay(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.GetAuthDelay();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_isTampered(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsTampered() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_isCertsChecked(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsCertsChecked() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_needSetup(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.NeedSetup() ? 1 : 0;
        }
    );
}

// ----------------------------------------------
// Satscard:

FFI_PLUGIN_EXPORT IntermediateSatscardSlot Satscard_getActiveSlot(int32_t handle, int32_t type) {
    IntermediateSatscardSlot intermediary;
    std::memset(&intermediary, 0, sizeof(IntermediateSatscardSlot));

    // Set an invalid index of -1 to indicate failure
    constexpr int32_t invalidSlotIndex{ -1 };
    intermediary.index = getFromTapCard<tap_protocol::Satscard>(handle, type, invalidSlotIndex, [&intermediary](const auto& card) {
            const tap_protocol::Satscard::Slot slot = card.GetActiveSlot();

            intermediary.status = static_cast<int32_t>(slot.status);
            intermediary.address = allocateCStringFromCpp(slot.address);
            intermediary.privkey = allocateBinaryArrayFromJSON(slot.privkey);
            intermediary.pubkey = allocateBinaryArrayFromJSON(slot.pubkey);
            intermediary.masterPK = allocateBinaryArrayFromJSON(slot.master_pk);
            intermediary.chainCode = allocateBinaryArrayFromJSON(slot.chain_code);

            return slot.index;
        }
    );

    return intermediary;
}

FFI_PLUGIN_EXPORT int32_t Satscard_getActiveSlotIndex(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.GetActiveSlotIndex();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_getNumSlots(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.GetNumSlots();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_hasUnusedSlots(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.HasUnusedSlots() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_isUsedUp(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.IsUsedUp() ? 1 : 0;
        }
    );
}

// ----------------------------------------------
// Tapsigner:

FFI_PLUGIN_EXPORT int32_t Tapsigner_getNumberOfBackups(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Tapsigner>(handle, type, -1, [](const auto& card) {
            return card.GetNumberOfBackups();
        }
    );
}

FFI_PLUGIN_EXPORT char* Tapsigner_getDerivationPath(int32_t handle, int32_t type) {
    return getFromTapCard<tap_protocol::Tapsigner, char*>(handle, type, nullptr, [](const auto& card) {
            char* value = nullptr;
            if (const auto optionalPath = card.GetDerivationPath()) {
                if (optionalPath.has_value()) {
                    value = allocateCStringFromCpp(optionalPath.value());
                }
            }

            return value;
        }
    );
}

// ----------------------------------------------
// Utility:

FFI_PLUGIN_EXPORT void Utility_freeBinaryArray(CBinaryArray array) {
    freeAllocatedBinaryArray(array);
}

FFI_PLUGIN_EXPORT void Utility_freeIntermediateSatscardSlot(IntermediateSatscardSlot slot) {
    freeAllocatedCString(slot.address);
    freeAllocatedBinaryArray(slot.privkey);
    freeAllocatedBinaryArray(slot.pubkey);
    freeAllocatedBinaryArray(slot.masterPK);
    freeAllocatedBinaryArray(slot.chainCode);
}

FFI_PLUGIN_EXPORT void Utility_freeString(char* cString) {
    freeAllocatedCString(cString);
}

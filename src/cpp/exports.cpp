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

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_InitializeLibrary() {
    if (g_protocolThread == nullptr) {
        g_protocolThread.reset(TapProtocolThread::CreateNew());

        if (g_protocolThread == nullptr) {
            return CKTapInterfaceErrorCode::ThreadAllocationFailed;
        }
    }
    return CKTapInterfaceErrorCode::Success;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_NewOperation() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::LibraryNotInitialized;
    }
    if (g_protocolThread->IsThreadActive()) {
        return CKTapInterfaceErrorCode::ThreadAlreadyInUse;
    }

    return g_protocolThread->Reset();
}

FFI_PLUGIN_EXPORT CKTapOperationResponse Core_EndOperation() {
    if (g_protocolThread == nullptr) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::LibraryNotInitialized);
    }
    if (!g_protocolThread->HasStarted()) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::ThreadNotYetStarted);
    }
    if (g_protocolThread->IsThreadActive()) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::OperationStillInProgress);
    }
    if (g_protocolThread->GetRecentErrorCode() == CKTapInterfaceErrorCode::Pending) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::ThreadNotYetFinalized);
    }
    if (g_protocolThread->HasFailed() || !g_protocolThread->IsTapsigner().has_value()) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::OperationFailed);
    }

    auto response = MakeTapOperationResponse(g_protocolThread->GetRecentErrorCode());
    auto index = invalidIndex;
    if (g_protocolThread->IsTapsigner().value()) {
        auto tapsigner = g_protocolThread->ReleaseTapsigner();
        if (!tapsigner) {
            return MakeTapOperationResponse(CKTapInterfaceErrorCode::ExpectedTapsignerButReceivedNothing);
        }

        index = UpdateVectorWithTapCard(g_tapsigners, tapsigner);
        response.handle.type = CKTapCardType::Tapsigner;
    } else {
        auto satscard = g_protocolThread->ReleaseSatscard();
        if (!satscard) {
            return MakeTapOperationResponse(CKTapInterfaceErrorCode::ExpectedSatscardButReceivedNothing);
        }

        index = UpdateVectorWithTapCard(g_satscards, satscard);
        response.handle.type = CKTapCardType::Satscard;
    }

    if (index == invalidIndex) {
        return MakeTapOperationResponse(CKTapInterfaceErrorCode::InvalidHandlingOfTapCardDuringFinalization);
    }

    response.handle.index = static_cast<int32_t>(index);
    return response;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_BeginAsyncHandshake() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::LibraryNotInitialized;
    }
    if (g_protocolThread->HasStarted()) {
        return CKTapInterfaceErrorCode::ThreadNotResetForHandshake;
    }

    if (!g_protocolThread->BeginCardHandshake()) {
        // The thread failed to start so we should diagnose why
        return g_protocolThread->FinalizeCardHandshake() ?
            g_protocolThread->GetRecentErrorCode() :
            CKTapInterfaceErrorCode::UnknownErrorDuringHandshake;
    }

    return CKTapInterfaceErrorCode::Success;
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeAsyncAction() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::LibraryNotInitialized;
    }
    if (!g_protocolThread->HasStarted()) {
        return CKTapInterfaceErrorCode::ThreadNotYetStarted;
    }
    if (g_protocolThread->IsThreadActive()) {
        return CKTapInterfaceErrorCode::AttemptToFinalizeActiveThread;
    }

    return g_protocolThread->FinalizeCardHandshake() ?
        g_protocolThread->GetRecentErrorCode() :
        CKTapInterfaceErrorCode::UnableToFinalizeAsyncAction;
}

FFI_PLUGIN_EXPORT const uint8_t* Core_GetTransportRequestPointer() {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    return optionalBytes.has_value() ? optionalBytes.value()->data() : nullptr;
}

FFI_PLUGIN_EXPORT int32_t Core_GetTransportRequestLength() {
    if (g_protocolThread == nullptr) {
        return 0;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    return optionalBytes.has_value() ? static_cast<int32_t>(optionalBytes.value()->size()) : 0;
}

FFI_PLUGIN_EXPORT uint8_t* Core_AllocateTransportResponseBuffer(const int32_t sizeInBytes) {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }
    if (sizeInBytes <= 0) {
        return nullptr;
    }

    auto optionalBuffer = g_protocolThread->AllocateTransportResponseBuffer(static_cast<size_t>(sizeInBytes));
    return optionalBuffer.value_or(nullptr);
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeTransportResponse() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::ThreadNotYetStarted;
    }
    if (g_protocolThread->GetState() != CKTapThreadState::TransportRequestReady) {
        return CKTapInterfaceErrorCode::ThreadNotReadyForResponse;
    }

    return g_protocolThread->FinalizeTransportResponse() ?
        CKTapInterfaceErrorCode::Success :
        CKTapInterfaceErrorCode::ThreadResponseFinalizationFailed;
}

FFI_PLUGIN_EXPORT CKTapThreadState Core_GetThreadState() {
    if (g_protocolThread == nullptr) {
        return CKTapThreadState::NotStarted;
    }

    return g_protocolThread->GetState();
}

FFI_PLUGIN_EXPORT CKTapProtoException Core_GetTapProtoException() {
    if (g_protocolThread != nullptr) {
        auto e = CKTapProtoException{ };
        if (g_protocolThread->GetTapProtocolException(e)) {
            return e;
        }
    }

    return { };
}

// ----------------------------------------------
// CKTapCard:

FFI_PLUGIN_EXPORT char* CKTapCard_GetIdentCString(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard, char*>(handle, type, nullptr, [](const auto& card) {
            return AllocateCStringFromCpp(card.GetIdent());
        }
    );
}

FFI_PLUGIN_EXPORT char* CKTapCard_GetAppletVersionCString(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard, char*>(handle, type, nullptr, [](const auto& card) {
            return AllocateCStringFromCpp(card.GetAppletVersion());
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_GetBirthHeight(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.GetBirthHeight();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTestnet(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsTestnet() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_GetAuthDelay(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.GetAuthDelay();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTampered(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsTampered() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsCertsChecked(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.IsCertsChecked() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_NeedSetup(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::CKTapCard>(handle, type, 0, [](const auto& card) {
            return card.NeedSetup() ? 1 : 0;
        }
    );
}

// ----------------------------------------------
// Satscard:

FFI_PLUGIN_EXPORT IntermediateSatscardSlot Satscard_GetActiveSlot(const int32_t handle, const int32_t type) {
    IntermediateSatscardSlot intermediary;
    std::memset(&intermediary, 0, sizeof(IntermediateSatscardSlot));

    // Set an invalid index of -1 to indicate failure
    constexpr int32_t invalidSlotIndex{ -1 };
    intermediary.index = GetFromTapCard<tap_protocol::Satscard>(handle, type, invalidSlotIndex, [&intermediary](const auto& card) {
            const tap_protocol::Satscard::Slot slot = card.GetActiveSlot();

            intermediary.status = static_cast<int32_t>(slot.status);
            intermediary.address = AllocateCStringFromCpp(slot.address);
            intermediary.privkey = AllocateBinaryArrayFromJSON(slot.privkey);
            intermediary.pubkey = AllocateBinaryArrayFromJSON(slot.pubkey);
            intermediary.masterPK = AllocateBinaryArrayFromJSON(slot.master_pk);
            intermediary.chainCode = AllocateBinaryArrayFromJSON(slot.chain_code);

            return slot.index;
        }
    );

    return intermediary;
}

FFI_PLUGIN_EXPORT int32_t Satscard_GetActiveSlotIndex(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.GetActiveSlotIndex();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_GetNumSlots(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.GetNumSlots();
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_HasUnusedSlots(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.HasUnusedSlots() ? 1 : 0;
        }
    );
}

FFI_PLUGIN_EXPORT int32_t Satscard_IsUsedUp(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Satscard>(handle, type, 0, [](const auto& card) {
            return card.IsUsedUp() ? 1 : 0;
        }
    );
}

// ----------------------------------------------
// Tapsigner:

FFI_PLUGIN_EXPORT int32_t Tapsigner_GetNumberOfBackups(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Tapsigner>(handle, type, -1, [](const auto& card) {
            return card.GetNumberOfBackups();
        }
    );
}

FFI_PLUGIN_EXPORT char* Tapsigner_GetDerivationPath(const int32_t handle, const int32_t type) {
    return GetFromTapCard<tap_protocol::Tapsigner, char*>(handle, type, nullptr, [](const auto& card) {
            char* value = nullptr;
            if (const auto optionalPath = card.GetDerivationPath()) {
                if (optionalPath.has_value()) {
                    value = AllocateCStringFromCpp(optionalPath.value());
                }
            }

            return value;
        }
    );
}

// ----------------------------------------------
// Utility:

FFI_PLUGIN_EXPORT void Utility_FreeBinaryArray(CBinaryArray array) {
    FreeAllocatedBinaryArray(array);
}

FFI_PLUGIN_EXPORT void Utility_FreeIntermediateSatscardSlot(IntermediateSatscardSlot slot) {
    FreeAllocatedCString(slot.address);
    FreeAllocatedBinaryArray(slot.privkey);
    FreeAllocatedBinaryArray(slot.pubkey);
    FreeAllocatedBinaryArray(slot.masterPK);
    FreeAllocatedBinaryArray(slot.chainCode);
}

FFI_PLUGIN_EXPORT void Utility_FreeString(char* cString) {
    FreeAllocatedCString(cString);
}

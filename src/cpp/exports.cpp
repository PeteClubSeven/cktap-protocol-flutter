#include <exports.h>

// Project
#include <internal/globals.h>
#include <internal/tap_protocol_thread.h>
#include <internal/utils.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstdlib>
#include <cstring>

// ----------------------------------------------
// Core Bindings:

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_initializeLibrary() {
    if (g_protocolThread == nullptr) {
        g_protocolThread.reset(TapProtocolThread::createNew());

        if (g_protocolThread == nullptr) {
            return CKTapInterfaceErrorCode::threadAllocationFailed;
        }
    }
    return CKTapInterfaceErrorCode::success;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_newOperation() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }
    if (g_protocolThread->isThreadActive()) {
        return CKTapInterfaceErrorCode::threadAlreadyInUse;
    }

    return g_protocolThread->reset();
}

FFI_FUNC_EXPORT CKTapOperationResponse Core_endOperation() {
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
    if (g_protocolThread->hasFailed() || !g_protocolThread->getConstructedCardType().has_value()) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::operationFailed);
    }

    auto response = makeTapOperationResponse(g_protocolThread->getRecentErrorCode());
    response.handle.type = g_protocolThread->getConstructedCardType().value();
    size_t index;
    if (response.handle.type == CKTapCardType::tapsigner) {
        auto tapsigner = g_protocolThread->releaseConstructedTapsigner();
        if (!tapsigner) {
            return makeTapOperationResponse(CKTapInterfaceErrorCode::expectedTapsignerButReceivedNothing);
        }

        index = updateVectorWithCard(g_tapsigners, tapsigner);
        response.handle.type = CKTapCardType::tapsigner;
    } else if (response.handle.type == CKTapCardType::satscard) {
        auto satscard = g_protocolThread->releaseConstructedSatscard();
        if (!satscard) {
            return makeTapOperationResponse(CKTapInterfaceErrorCode::expectedSatscardButReceivedNothing);
        }

        index = updateVectorWithCard(g_satscards, satscard);
        response.handle.type = CKTapCardType::satscard;
    } else {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::invalidCardDuringHandshake);
    }

    if (index == invalidIndex) {
        return makeTapOperationResponse(CKTapInterfaceErrorCode::invalidHandlingOfCardDuringFinalization);
    }

    response.handle.index = static_cast<int32_t>(index);
    return response;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_requestCancelOperation() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }

    g_protocolThread->requestCancel();
    return CKTapInterfaceErrorCode::success;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_prepareCardOperation(const int32_t handle, const int32_t cardType) {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    } else if (g_protocolThread->isThreadActive()) {
        return CKTapInterfaceErrorCode::threadAlreadyInUse;
    }
    switch (cardType) {
        case CKTapCardType::satscard:
            if (handle < 0 || handle >= g_satscards.size() ||
                !g_protocolThread->prepareCardOperation(g_satscards[handle].card)) {
                return CKTapInterfaceErrorCode::unknownSatscardHandle;
            }
            break;
        case CKTapCardType::tapsigner:
            if (handle < 0 || handle >= g_tapsigners.size() ||
                !g_protocolThread->prepareCardOperation(g_tapsigners[handle].card)) {
                return CKTapInterfaceErrorCode::unknownTapsignerHandle;
            }
            break;
        default:
            return CKTapInterfaceErrorCode::invalidCardOperation;
    }
    return CKTapInterfaceErrorCode::success;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_beginAsyncHandshake(const int32_t cardType) {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    }
    if (g_protocolThread->hasStarted()) {
        return CKTapInterfaceErrorCode::threadNotResetForHandshake;
    }

    if (!g_protocolThread->beginCardHandshake(cardType)) {
        // The thread failed to start so we should diagnose why
        return g_protocolThread->finalizeOperation() ?
            g_protocolThread->getRecentErrorCode() :
            CKTapInterfaceErrorCode::unknownErrorDuringHandshake;
    }

    return CKTapInterfaceErrorCode::success;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_finalizeAsyncAction() {
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

FFI_FUNC_EXPORT const uint8_t* Core_getTransportRequestPointer() {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }

    const auto optionalBytes = g_protocolThread->getTransportRequest();
    return optionalBytes.has_value() ? optionalBytes.value()->data() : nullptr;
}

FFI_FUNC_EXPORT int32_t Core_getTransportRequestLength() {
    if (g_protocolThread == nullptr) {
        return 0;
    }

    const auto optionalBytes = g_protocolThread->getTransportRequest();
    return optionalBytes.has_value() ? static_cast<int32_t>(optionalBytes.value()->size()) : 0;
}

FFI_FUNC_EXPORT uint8_t* Core_allocateTransportResponseBuffer(const int32_t sizeInBytes) {
    if (g_protocolThread == nullptr) {
        return nullptr;
    }
    if (sizeInBytes <= 0) {
        return nullptr;
    }

    auto optionalBuffer = g_protocolThread->allocateTransportResponseBuffer(static_cast<size_t>(sizeInBytes));
    return optionalBuffer.value_or(nullptr);
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_finalizeTransportResponse() {
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

FFI_FUNC_EXPORT CKTapThreadState Core_getThreadState() {
    if (g_protocolThread == nullptr) {
        return CKTapThreadState::notStarted;
    }

    return g_protocolThread->getState();
}

FFI_FUNC_EXPORT CKTapProtoException Core_getTapProtoException() {
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

FFI_FUNC_EXPORT CKTapInterfaceErrorCode CKTapCard_beginWait() {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    } else if (g_protocolThread->getState() != CKTapThreadState::awaitingCardOperation) {
        return CKTapInterfaceErrorCode::threadNotAwaitingCardOperation;
    }
    return g_protocolThread->beginCKTapCard_Wait() ?
        CKTapInterfaceErrorCode::success :
        CKTapInterfaceErrorCode::invalidCardOperation;
}

FFI_FUNC_EXPORT WaitResponseParams CKTapCard_getWaitResponse() {
    WaitResponseParams params;
    std::memset(&params, 0, sizeof(params));

    if (g_protocolThread == nullptr) {
        params.status.errorCode = CKTapInterfaceErrorCode::libraryNotInitialized;
    } else {
        if (auto optional = g_protocolThread->getResponse<CardOperation::CKTapCard_Wait>()) {
            params.status.errorCode = CKTapInterfaceErrorCode::success;
            params.success = optional.value().success ? 1 : 0;
            params.authDelay = optional.value().auth_delay;
        } else {
            params.status.errorCode = CKTapInterfaceErrorCode::invalidResponseFromCardOperation;
        }
    }
    return params;
}

// ----------------------------------------------
// Satscard:

FFI_FUNC_EXPORT SatscardConstructorParams Satscard_createConstructorParams(const int32_t handle) {
    SatscardConstructorParams params;
    std::memset(&params, 0, sizeof(params));

    params.status = accessTapCard<tap_protocol::Satscard>(handle, [handle, &params](const auto& wrapper) {
        const auto& card = *wrapper.card;
        fillConstructorParams(params.base, handle, card);
        params.activeSlotIndex = card.GetActiveSlotIndex();
        params.numSlots = card.GetNumSlots();
        params.hasUnusedSlots = card.HasUnusedSlots() ? 1 : 0;
        params.isUsedUp = card.IsUsedUp() ? 1 : 0;
        return CKTapInterfaceErrorCode::success;
    });
    return params;
}

FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getActiveSlot(const int32_t handle) {
    SatscardSlotResponse response;
    std::memset(&response, 0, sizeof(response));

    response.status = accessTapCard<tap_protocol::Satscard>(handle, [=, &response](auto& wrapper) {
        auto slot = wrapper.card->GetActiveSlot();
        const auto index = slot.index;
        if (index >= wrapper.slots.size()) {
            wrapper.slots.resize(slot.index + 1);
        }
        fillConstructorParams(response.params, handle, slot);
        wrapper.slots[index] = std::make_unique<tap_protocol::Satscard::Slot>(std::move(slot));
        return CKTapInterfaceErrorCode::success;
    });
    return response;
}

FFI_FUNC_EXPORT SlotToWifResponse Satscard_slotToWif(const int32_t handle, const int32_t index) {
    SlotToWifResponse response;
    std::memset(&response, 0, sizeof(response));

    response.status = accessTapCard<tap_protocol::Satscard>(handle, [=, &response](const auto& wrapper) {
        if (index < 0 || index >= wrapper.slots.size() || !wrapper.slots[index]) {
            return CKTapInterfaceErrorCode::unknownSlotForGivenSatscardHandle;
        }
        response.wif = allocateCStringFromCpp(wrapper.slots[index]->to_wif());
        return CKTapInterfaceErrorCode::success;
    });
    return response;
}

// ----------------------------------------------
// Tapsigner:

FFI_FUNC_EXPORT TapsignerConstructorParams Tapsigner_createConstructorParams(const int32_t handle) {
    TapsignerConstructorParams params;
    std::memset(&params, 0, sizeof(params));

    params.status = accessTapCard<tap_protocol::Tapsigner>(handle, [handle, &params](const auto& wrapper) {
        const auto& card = *wrapper.card;
        fillConstructorParams(params.base, handle, card);
        params.numberOfBackups = card.GetNumberOfBackups();
        if (const auto path = card.GetDerivationPath()) {
            if (path.has_value()) {
                params.derivationPath = allocateCStringFromCpp(path.value());
            }
        }
        return CKTapInterfaceErrorCode::success;
    });
    return params;
}

// ----------------------------------------------
// Utility:

FFI_FUNC_EXPORT void Utility_freeCBinaryArray(CBinaryArray array) {
    freeCBinaryArray(array);
}

FFI_FUNC_EXPORT void Utility_freeCKTapInterfaceStatus(CKTapInterfaceStatus status) {
    freeCKTapInterfaceStatus(status);
}

FFI_FUNC_EXPORT void Utility_freeCKTapProtoException(CKTapProtoException exception) {
    freeCKTapProtoException(exception);
}

FFI_FUNC_EXPORT void Utility_freeCString(char* cString) {
    freeCString(cString);
}

FFI_FUNC_EXPORT void Utility_freeSatscardConstructorParams(SatscardConstructorParams params) {
    freeSatscardConstructorParams(params);
}

FFI_FUNC_EXPORT void Utility_freeSatscardSlotResponse(SatscardSlotResponse response) {
    freeSatscardGetSlotResponse(response);
}

FFI_FUNC_EXPORT void Utility_freeSlotConstructorParams(SlotConstructorParams params) {
    freeSlotConstructorParams(params);
}

FFI_FUNC_EXPORT void Utility_freeSlotToWifResponse(SlotToWifResponse response) {
    freeSlotToWifResponse(response);
}

FFI_FUNC_EXPORT void Utility_freeTapsignerConstructorParams(TapsignerConstructorParams params) {
    freeTapsignerConstructorParams(params);
}

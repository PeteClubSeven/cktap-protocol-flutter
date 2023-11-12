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
    size_t index;
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

FFI_PLUGIN_EXPORT uint8_t* Core_allocateTransportResponseBuffer(const int32_t sizeInBytes) {
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
// Satscard:

FFI_PLUGIN_EXPORT SatscardConstructorParams Satscard_createConstructorParams(const int32_t handle) {
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

FFI_PLUGIN_EXPORT SatscardGetSlotResponse Satscard_getActiveSlot(const int32_t handle) {
    SatscardGetSlotResponse response;
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

FFI_PLUGIN_EXPORT SlotToWifResponse Satscard_slotToWif(const int32_t handle, const int32_t index) {
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

FFI_PLUGIN_EXPORT TapsignerConstructorParams Tapsigner_createConstructorParams(const int32_t handle) {
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

FFI_PLUGIN_EXPORT void Utility_freeCBinaryArray(CBinaryArray array) {
    freeCBinaryArray(array);
}

FFI_PLUGIN_EXPORT void Utility_freeCKTapInterfaceStatus(CKTapInterfaceStatus status) {
    freeCKTapInterfaceStatus(status);
}

FFI_PLUGIN_EXPORT void Utility_freeCKTapProtoException(CKTapProtoException exception) {
    freeCKTapProtoException(exception);
}

FFI_PLUGIN_EXPORT void Utility_freeCString(char* cString) {
    freeCString(cString);
}

FFI_PLUGIN_EXPORT void Utility_freeSatscardConstructorParams(SatscardConstructorParams params) {
    freeSatscardConstructorParams(params);
}

FFI_PLUGIN_EXPORT void Utility_freeSatscardGetSlotResponse(SatscardGetSlotResponse response) {
    freeSatscardGetSlotResponse(response);
}

FFI_PLUGIN_EXPORT void Utility_freeSlotConstructorParams(SlotConstructorParams params) {
    freeSlotConstructorParams(params);
}

FFI_PLUGIN_EXPORT void Utility_freeSlotToWifResponse(SlotToWifResponse response) {
    freeSlotToWifResponse(response);
}

FFI_PLUGIN_EXPORT void Utility_freeTapsignerConstructorParams(TapsignerConstructorParams params) {
    freeTapsignerConstructorParams(params);
}

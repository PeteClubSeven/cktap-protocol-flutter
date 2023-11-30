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
// Helpers:

static CKTapInterfaceErrorCode beginCardOp(const std::function<bool ()>& func) noexcept {
    if (g_protocolThread == nullptr) {
        return CKTapInterfaceErrorCode::libraryNotInitialized;
    } else if (g_protocolThread->getState() != CKTapThreadState::awaitingCardOperation) {
        return CKTapInterfaceErrorCode::threadNotAwaitingCardOperation;
    }
    try {
        return func() ?
            CKTapInterfaceErrorCode::success :
            CKTapInterfaceErrorCode::invalidCardOperation;
    }
    catch (...) {
        return CKTapInterfaceErrorCode::unexpectedExceptionWhenStartingCardOperation;
    }
}

template <typename Return, CardOperation op>
static Return getCardOpResponse(const std::function<void (Return&, CardResponseType<op>)>& func) noexcept {
    Return r { };
    std::memset(&r, 0, sizeof(r));

    if (g_protocolThread == nullptr) {
        r.status.errorCode = CKTapInterfaceErrorCode::libraryNotInitialized;
    } else if (g_protocolThread->isThreadActive()) {
        r.status.errorCode = CKTapInterfaceErrorCode::threadAlreadyInUse;
    } else if (g_protocolThread->getState() == CKTapThreadState::tapProtocolError) {
        r.status.errorCode = CKTapInterfaceErrorCode::caughtTapProtocolException;
        g_protocolThread->getTapProtocolException(r.status.exception);
    } else {
        r.status.errorCode = g_protocolThread->getRecentErrorCode();
    }

    if (r.status.errorCode == CKTapInterfaceErrorCode::success) {
        try {
            if (const auto optional = g_protocolThread->getResponse<op>()) {
                func(r, std::move(optional.value()));
            } else {
                r.status.errorCode = CKTapInterfaceErrorCode::invalidResponseFromCardOperation;
            }
        } catch (...) {
            r.status.errorCode = CKTapInterfaceErrorCode::unexpectedExceptionWhenGettingCardOperationResult;
        }
    }
    return r;
}

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
    } else if (g_protocolThread->hasStarted()) {
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
    return beginCardOp([=]() {
        return g_protocolThread->beginCKTapCard_Wait();
    });
}

FFI_FUNC_EXPORT WaitResponseParams CKTapCard_getWaitResponse() {
    return getCardOpResponse<WaitResponseParams, CardOperation::CKTapCard_Wait>([](auto& result, auto response) {
        result.success = response.success ? 1 : 0;
        result.authDelay = response.auth_delay;
    });
}

// ----------------------------------------------
// Satscard:

FFI_FUNC_EXPORT SatscardConstructorParams Satscard_createConstructorParams(const int32_t handle) {
    SatscardConstructorParams params;
    std::memset(&params, 0, sizeof(params));

    params.status = accessCard<tap_protocol::Satscard>(handle, [handle, &params](const auto& wrapper) {
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

    response.status = accessCard<tap_protocol::Satscard>(handle, [=, &response](auto& wrapper) {
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

    response.status = accessCard<tap_protocol::Satscard>(handle, [=, &response](const auto& wrapper) {
        if (index < 0 || index >= wrapper.slots.size() || !wrapper.slots[index]) {
            return CKTapInterfaceErrorCode::unknownSlotForGivenSatscardHandle;
        }
        response.wif = allocateCStringFromCpp(wrapper.slots[index]->to_wif());
        return CKTapInterfaceErrorCode::success;
    });
    return response;
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginCertificateCheck() {
    return beginCardOp([=]() {
        return g_protocolThread->beginSatscard_CertificateCheck();
    });
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginGetSlot(const int32_t slot, const char* spendCode) {
    return beginCardOp([=]() {
        return g_protocolThread->beginSatscard_GetSlot(slot, spendCode);
    });
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginListSlots(const char* spendCode, const int32_t limit) {
    return beginCardOp([=]() {
        return g_protocolThread->beginSatscard_ListSlots(spendCode, limit);
    });
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginNew(const char* chainCode, const char* spendCode) {
    return beginCardOp([=]() {
        return g_protocolThread->beginSatscard_New(chainCode, spendCode);
    });
}

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginUnseal(const char* spendCode) {
    return beginCardOp([=]() {
        return g_protocolThread->beginSatscard_Unseal(spendCode);
    });
}

FFI_FUNC_EXPORT CertificateCheckParams Satscard_getCertificateCheckResponse() {
    return getCardOpResponse<CertificateCheckParams, CardOperation::Satscard_CertificateCheck>([](auto& result, auto response) {
        result.isCertsChecked = response ? 1 : 0;
    });
}

FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getGetSlotResponse(const int32_t handle) {
    return getCardOpResponse<SatscardSlotResponse, CardOperation::Satscard_GetSlot>([=](auto& result, auto slot) {
        fillConstructorParams(result.params, handle, slot);
        storeSatscardSlot(handle, std::move(slot));
    });
}

FFI_FUNC_EXPORT SatscardListSlotsParams Satscard_getListSlotsResponse(const int32_t handle) {
    return getCardOpResponse<SatscardListSlotsParams, CardOperation::Satscard_ListSlots>([=](auto& result, auto slots) {
        result.array = allocateCArray<SlotConstructorParams>(slots.size());
        result.length = static_cast<int32_t>(slots.size());
        for (size_t i { 0 }; i < slots.size(); ++i) {
            fillConstructorParams(result.array[i], handle, slots[i]);
            storeSatscardSlot(handle, std::move(slots[i]));
        }
    });
}
FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getNewResponse(const int32_t handle) {
    return getCardOpResponse<SatscardSlotResponse, CardOperation::Satscard_New>([=](auto& result, auto slot) {
        fillConstructorParams(result.params, handle, slot);
        storeSatscardSlot(handle, std::move(slot));
    });
}

FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getUnsealResponse(const int32_t handle) {
    return getCardOpResponse<SatscardSlotResponse, CardOperation::Satscard_Unseal>([=](auto& result, auto slot) {
        fillConstructorParams(result.params, handle, slot);
        storeSatscardSlot(handle, std::move(slot));
    });
}

// ----------------------------------------------
// Tapsigner:

FFI_FUNC_EXPORT TapsignerConstructorParams Tapsigner_createConstructorParams(const int32_t handle) {
    TapsignerConstructorParams params;
    std::memset(&params, 0, sizeof(params));

    params.status = accessCard<tap_protocol::Tapsigner>(handle, [handle, &params](const auto& wrapper) {
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
    freePointer(cString);
}

FFI_FUNC_EXPORT void Utility_freeSatscardConstructorParams(SatscardConstructorParams params) {
    freeSatscardConstructorParams(params);
}

FFI_FUNC_EXPORT void Utility_freeSatscardListSlotsParams(SatscardListSlotsParams params) {
    freeSatscardListSlotsParams(params);
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

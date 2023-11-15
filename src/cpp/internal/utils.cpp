#include <internal/utils.h>

// Project
#include <internal/tap_protocol_thread.h>

// Third party
#include <tap_protocol/cktapcard.h>
#include <tap_protocol/utils.h>

// STL
#include <cstring>

CBinaryArray allocateCBinaryArrayFromJSON(const nlohmann::json::binary_t& binary) {
    CBinaryArray array;
    std::memset(&array, 0, sizeof(CBinaryArray));

    if (binary.empty()) {
        return array;
    }

    array.ptr = static_cast<uint8_t*>(std::malloc(binary.size()));
    array.length = static_cast<int32_t>(binary.size());
    if (array.ptr != nullptr) {
        std::memcpy(array.ptr, binary.data(), binary.size());
    }

    return array;
}

CKTapProtoException allocateCKTapProtoException(const tap_protocol::TapProtoException& e) noexcept {
    CKTapProtoException result = {
        .code = e.code(),
        .message = strdup(e.what()),
    };

    return result;
}

char* allocateCStringFromCpp(const std::string& cppString) {
    if (cppString.empty()) {
        return nullptr;
    }

    char* cString = strdup(cppString.c_str());
    return cString;
}

void fillConstructorParams(CKTapCardConstructorParams& params, const size_t index, const tap_protocol::CKTapCard& card) {
    params.handle = static_cast<int32_t>(index);
    params.type = card.IsTapsigner() ? CKTapCardType::tapsigner : CKTapCardType::satscard;
    params.ident = allocateCStringFromCpp(card.GetIdent());
    params.appletVersion = allocateCStringFromCpp(card.GetAppletVersion());
    params.authDelay = card.GetAuthDelay();
    params.birthHeight = card.GetBirthHeight();
    params.isCertsChecked = card.IsCertsChecked() ? 1 : 0;
    params.isTampered = card.IsTampered() ? 1 : 0;
    params.isTestnet = card.IsTestnet() ? 1 : 0;
    params.needsSetup = card.NeedSetup() ? 1 : 0;
}

void fillConstructorParams(SlotConstructorParams& params, const int32_t handle, const tap_protocol::Satscard::Slot& slot) {
    params.satscardHandle = handle;
    params.status = static_cast<int32_t>(slot.status);
    params.address = allocateCStringFromCpp(slot.address);
    params.privkey = allocateCBinaryArrayFromJSON(slot.privkey);
    params.pubkey = allocateCBinaryArrayFromJSON(slot.pubkey);
    params.masterPK = allocateCBinaryArrayFromJSON(slot.master_pk);
    params.chainCode = allocateCBinaryArrayFromJSON(slot.chain_code);
}

void freeCBinaryArray(CBinaryArray& array) {
    if (array.ptr != nullptr) {
        std::free(array.ptr);
        array.ptr = nullptr;
        array.length = 0;
    }
}

void freeCKTapCardConstructorParams(CKTapCardConstructorParams& params) {
    freePointer(params.ident);
    freePointer(params.appletVersion);
}

void freeCKTapInterfaceStatus(CKTapInterfaceStatus& status) {
    freeCKTapProtoException(status.exception);
}

void freeCKTapProtoException(CKTapProtoException& exception) {
    freePointer(exception.message);
}

void freeSatscardConstructorParams(SatscardConstructorParams& params) {
    freeCKTapCardConstructorParams(params.base);
}

void freeSatscardGetSlotResponse(SatscardSlotResponse& response) {
    freeCKTapInterfaceStatus(response.status);
    freeSlotConstructorParams(response.params);
}

void freeSatscardListSlotsParams(SatscardListSlotsParams& params) {
    freeCKTapInterfaceStatus(params.status);
    freePointer(params.array);
}

void freeSlotConstructorParams(SlotConstructorParams& params) {
    freePointer(params.address);
    freeCBinaryArray(params.privkey);
    freeCBinaryArray(params.pubkey);
    freeCBinaryArray(params.masterPK);
    freeCBinaryArray(params.chainCode);
}

void freeSlotToWifResponse(SlotToWifResponse response) {
    freeCKTapInterfaceStatus(response.status);
    freePointer(response.wif);
}

void freeTapsignerConstructorParams(TapsignerConstructorParams& params) {
    freeCKTapCardConstructorParams(params.base);
    freePointer(params.derivationPath);
}

tap_protocol::Bytes makeChainCode(const char* cString) {
    if (cString == nullptr) {
        return tap_protocol::RandomChainCode();
    }
    return tap_protocol::Hex2Bytes(cString);
}

std::string makeCvc(const char* cString) {
    if (cString == nullptr) {
        return { };
    }
    return { cString };
}

CKTapCardHandle makeTapCardHandle(const int32_t index, const int32_t type) {
    CKTapCardHandle handle = {
        .index = index,
        .type = makeTapCardType(type),
    };

    return handle;
}

CKTapCardType makeTapCardType(const int32_t type) {
    switch (type) {
        case CKTapCardType::satscard:
            return CKTapCardType::satscard;
        case CKTapCardType::tapsigner:
            return CKTapCardType::tapsigner;
        default:
            return CKTapCardType::unknownCard;
    }
}

CKTapInterfaceStatus makeTapInterfaceStatus(CKTapInterfaceErrorCode errorCode) noexcept {
    CKTapInterfaceStatus status = {
        .errorCode = errorCode,
    };
    std::memset(&status.exception, 0, sizeof(status.exception));
    return status;
}

CKTapInterfaceStatus makeTapInterfaceStatus(CKTapInterfaceErrorCode errorCode, const tap_protocol::TapProtoException& e) noexcept {
    CKTapInterfaceStatus status = {
        .errorCode = errorCode,
        .exception = allocateCKTapProtoException(e),
    };
    return status;
}

CKTapOperationResponse makeTapOperationResponse(CKTapInterfaceErrorCode errorCode, int32_t index, CKTapCardType type) {
    CKTapOperationResponse response = {
        .handle = makeTapCardHandle(index, type),
        .errorCode = errorCode,
    };

    return response;
}

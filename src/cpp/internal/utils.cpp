#include <internal/utils.h>

// Project
#include <internal/tap_protocol_thread.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstring>

std::unique_ptr<TapProtocolThread> g_protocolThread{ };
std::vector<std::unique_ptr<tap_protocol::Satscard>> g_satscards{ };
std::vector<std::unique_ptr<tap_protocol::Tapsigner>> g_tapsigners{ };

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

void fillConstructorParams(SlotConstructorParams& params, const tap_protocol::Satscard::Slot& slot) {
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
    freeCString(params.ident);
    freeCString(params.appletVersion);
}

void freeCString(char*& cString) {
    if (cString != nullptr) {
        std::free(cString);
        cString = nullptr;
    }
}

void freeSatscardConstructorParams(SatscardConstructorParams& params) {
    freeCKTapCardConstructorParams(params.base);
    freeSlotConstructorParams(params.activeSlot);
}

void freeSlotConstructorParams(SlotConstructorParams& params) {
    freeCString(params.address);
    freeCBinaryArray(params.privkey);
    freeCBinaryArray(params.pubkey);
    freeCBinaryArray(params.masterPK);
    freeCBinaryArray(params.chainCode);
}

void freeTapsignerConstructorParams(TapsignerConstructorParams& params) {
    freeCKTapCardConstructorParams(params.base);
    freeCString(params.derivationPath);
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

CKTapOperationResponse makeTapOperationResponse(CKTapInterfaceErrorCode errorCode, int32_t index, CKTapCardType type) {
    CKTapOperationResponse response = {
        .handle.index = index,
        .handle.type = type,
        .errorCode = errorCode,
    };

    return response;
}

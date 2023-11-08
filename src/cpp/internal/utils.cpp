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

char* AllocateCStringFromCpp(const std::string& cppString) {
    if (cppString.empty()) {
        return nullptr;
    }

    char* cString = strdup(cppString.c_str());
    return cString;
}

CBinaryArray AllocateBinaryArrayFromJSON(const nlohmann::json::binary_t& binary) {
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

void FreeAllocatedCString(char*& cString) {
    if (cString != nullptr) {
        std::free(cString);
        cString = nullptr;
    }
}

void FreeAllocatedBinaryArray(CBinaryArray& array) {
    if (array.ptr != nullptr) {
        std::free(array.ptr);
        array.ptr = nullptr;
        array.length = 0;
    }
}

CKTapCardHandle ConstructTapCardHandle(const int32_t index, const int32_t type) {
    CKTapCardHandle handle = {
        .index = index,
        .type = IntToTapCardType(type),
    };

    return handle;
}

CKTapCardType IntToTapCardType(const int32_t type) {
    switch (type) {
        case CKTapCardType::Satscard:
            return CKTapCardType::Satscard;
        case CKTapCardType::Tapsigner:
            return CKTapCardType::Tapsigner;
        default:
            return CKTapCardType::UnknownCard;
    }
}

CKTapOperationResponse MakeTapOperationResponse(CKTapInterfaceErrorCode errorCode, int32_t index, CKTapCardType type) {
    CKTapOperationResponse response = {
        .handle.index = index,
        .handle.type = type,
        .errorCode = errorCode,
    };

    return response;
}
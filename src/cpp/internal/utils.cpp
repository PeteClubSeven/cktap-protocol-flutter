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

char* allocateCStringFromCpp(const std::string& cppString) {
    if (cppString.empty()) {
        return nullptr;
    }

    char* cString = strdup(cppString.c_str());
    return cString;
}

CBinaryArray allocateBinaryArrayFromJSON(const nlohmann::json::binary_t& binary) {
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

void freeAllocatedCString(char*& cString) {
    if (cString != nullptr) {
        std::free(cString);
        cString = nullptr;
    }
}

void freeAllocatedBinaryArray(CBinaryArray& array) {
    if (array.ptr != nullptr) {
        std::free(array.ptr);
        array.ptr = nullptr;
        array.length = 0;
    }
}

CKTapCardHandle constructTapCardHandle(const int32_t index, const int32_t type) {
    CKTapCardHandle handle = {
        .index = index,
        .type = intToTapCardType(type),
    };

    return handle;
}

CKTapCardType intToTapCardType(const int32_t type) {
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
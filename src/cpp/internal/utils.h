#ifndef __CKTAP_PROTOCOL__INTERNAL_UTILS_H__
#define __CKTAP_PROTOCOL__INTERNAL_UTILS_H__

// Project
#include <enums.h>
#include <structs.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <type_traits>

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

CBinaryArray allocateCBinaryArrayFromJSON(const nlohmann::json::binary_t& binary);
CKTapProtoException allocateCKTapProtoException(const tap_protocol::TapProtoException& e) noexcept;
char* allocateCStringFromCpp(const std::string& cppString);

void fillConstructorParams(CKTapCardConstructorParams& params, size_t index, const tap_protocol::CKTapCard& card);
void fillConstructorParams(SlotConstructorParams& params, int32_t handle, const tap_protocol::Satscard::Slot& slot);

void freeCBinaryArray(CBinaryArray& array);
void freeCKTapCardConstructorParams(CKTapCardConstructorParams& params);
void freeCKTapInterfaceStatus(CKTapInterfaceStatus& status);
void freeCKTapProtoException(CKTapProtoException& exception);
void freeCString(char*& cString);
void freeSatscardConstructorParams(SatscardConstructorParams& params);
void freeSatscardGetSlotResponse(SatscardSlotResponse& response);
void freeSlotConstructorParams(SlotConstructorParams& params);
void freeSlotToWifResponse(SlotToWifResponse response);
void freeTapsignerConstructorParams(TapsignerConstructorParams& params);

CKTapCardHandle makeTapCardHandle(int32_t index, int32_t type);
CKTapCardType makeTapCardType(const int32_t type);
CKTapInterfaceStatus makeTapInterfaceStatus(CKTapInterfaceErrorCode errorCode) noexcept;
CKTapInterfaceStatus makeTapInterfaceStatus(
    CKTapInterfaceErrorCode errorCode,
    const tap_protocol::TapProtoException& e) noexcept;
CKTapOperationResponse makeTapOperationResponse(
    CKTapInterfaceErrorCode errorCode,
    int32_t index = -1,
    CKTapCardType type = CKTapCardType::unknownCard
);

/// @brief Special overload for unique pointers
template <typename T, typename U>
std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<U>& pointer) {
    if (T* castPointer = dynamic_cast<T*>(pointer.get())) {
        pointer.release();
        return std::unique_ptr<T>{ castPointer };
    }

    return nullptr;
}

/// @brief Special overload for unique pointers
template <typename T, typename U>
std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U>& pointer) {
    if (T* castPointer = static_cast<T*>(pointer.get())) {
        pointer.release();
        return std::unique_ptr<T>{ castPointer };
    }

    return nullptr;
}

#endif // __CKTAP_PROTOCOL__INTERNAL_UTILS_H__
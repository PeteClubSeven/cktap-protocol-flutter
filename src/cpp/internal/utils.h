#ifndef __CKTAP_PROTOCOL__INTERNAL_UTILS_H__
#define __CKTAP_PROTOCOL__INTERNAL_UTILS_H__

// Project
#include <enums.h>
#include <structs.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Forward declarations
class TapProtocolThread;

// Globals
constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();
extern std::unique_ptr<TapProtocolThread> g_protocolThread;
extern std::vector<std::unique_ptr<tap_protocol::Satscard>> g_satscards;
extern std::vector<std::unique_ptr<tap_protocol::Tapsigner>> g_tapsigners;

CBinaryArray allocateCBinaryArrayFromJSON(const nlohmann::json::binary_t& binary);
char* allocateCStringFromCpp(const std::string& cppString);

void fillConstructorParams(CKTapCardConstructorParams& params, size_t index, const tap_protocol::CKTapCard& card);
void fillConstructorParams(SlotConstructorParams& params, const tap_protocol::Satscard::Slot& slot);

void freeCBinaryArray(CBinaryArray& array);
void freeCKTapCardConstructorParams(CKTapCardConstructorParams& params);
void freeCString(char*& cString);
void freeSatscardConstructorParams(SatscardConstructorParams& params);
void freeSlotConstructorParams(SlotConstructorParams& params);
void freeTapsignerConstructorParams(TapsignerConstructorParams& params);

CKTapCardHandle makeTapCardHandle(int32_t index, int32_t type);
CKTapCardType makeTapCardType(const int32_t type);
CKTapOperationResponse makeTapOperationResponse(
    CKTapInterfaceErrorCode errorCode,
    int32_t index = -1,
    CKTapCardType type = CKTapCardType::unknownCard
);

template <typename CardType, typename Func>
bool readTapCard(const int32_t index, const Func& readFunction) {
    constexpr bool isSatscard = std::is_same_v<CardType, tap_protocol::Satscard>;
    constexpr bool isTapsigner = std::is_same_v<CardType, tap_protocol::Tapsigner>;
    const auto processCard = [index, &readFunction](const auto& vector) {
        if (index < 0 || index >= vector.size()) {
            return false;
        }
        const auto& card = vector[static_cast<size_t>(index)];
        if (!card) {
            return false;
        }
        if (card->IsTapsigner() && !isTapsigner) {
            return false;
        }
        readFunction(*card);
        return true;
    };

    if constexpr(isSatscard) {
        return processCard(g_satscards);
    }
    else if constexpr(isTapsigner) {
        return processCard(g_tapsigners);
    }
    else {
        static_assert("Unsupported CKTapCard");
    }
}

template <typename CardType>
size_t updateVectorWithTapCard(std::vector<std::unique_ptr<CardType>>& vector, std::unique_ptr<CardType>& card) {
    if (!card) {
        return invalidIndex;
    }

    for (size_t index{ 0 }; index < vector.size(); ++index) {
        if (vector[index] && vector[index]->GetIdent() == card->GetIdent()) {
            vector[index] = std::move(card);
            return index;
        }
    }

    const auto index = vector.size();
    vector.emplace_back(std::move(card));

    return index;
}

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
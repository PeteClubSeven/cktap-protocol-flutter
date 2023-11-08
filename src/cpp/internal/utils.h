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


char* allocateCStringFromCpp(const std::string& cppString);
CBinaryArray allocateBinaryArrayFromJSON(const nlohmann::json::binary_t& binary);

void freeAllocatedCString(char*& cString);
void freeAllocatedBinaryArray(CBinaryArray& array);

CKTapCardHandle constructTapCardHandle(int32_t index, int32_t type);

template <typename CardType, typename R, typename Func>
R getFromTapCard(const int32_t index, const int32_t type, R&& defaultReturn, const Func& getterFunction) {
    const auto handle = constructTapCardHandle(index, type);
    const auto processCard = [&handle, &defaultReturn, &getterFunction](const auto& vector) {
        if (handle.index < 0 || handle.index >= vector.size()) {
            return std::move(defaultReturn);
        }

        const auto& card = vector[static_cast<size_t>(handle.index)];
        if (!card) {
            return std::move(defaultReturn);
        }
        if ((card->IsTapsigner() && handle.type != CKTapCardType::tapsigner) ||
            (!card->IsTapsigner() && handle.type != CKTapCardType::satscard)) {
            return std::move(defaultReturn);
        }

        return std::move(getterFunction(*card));
    };

    if constexpr(std::is_same_v<CardType, tap_protocol::Satscard>)
    {
        return std::move(processCard(g_satscards));
    }
    else if constexpr(std::is_same_v<CardType, tap_protocol::Tapsigner>)
    {
        return std::move(processCard(g_tapsigners));
    }
    else if constexpr(std::is_same_v<CardType, tap_protocol::CKTapCard>)
    {
        switch (handle.type) {
            case CKTapCardType::satscard:
                return std::move(processCard(g_satscards));
            case CKTapCardType::tapsigner:
                return std::move(processCard(g_tapsigners));
            default:
                return std::move(defaultReturn);
        }
    }
}

CKTapCardType intToTapCardType(const int32_t type);

CKTapOperationResponse makeTapOperationResponse(
    CKTapInterfaceErrorCode errorCode,
    int32_t index = -1,
    CKTapCardType type = CKTapCardType::unknownCard
);

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
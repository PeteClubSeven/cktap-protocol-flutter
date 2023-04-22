#ifndef __CKTAP_PROTOCOL__INTERNAL_UTILS_H__
#define __CKTAP_PROTOCOL__INTERNAL_UTILS_H__

// Project
#include <Enums.h>
#include <Structs.h>

// STL
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward declarations
class TapProtocolThread;
namespace tap_protocol
{
    class Satscard;
    class Tapsigner;
}

// Globals
constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();

std::unique_ptr<TapProtocolThread> g_protocolThread { };
std::vector<std::unique_ptr<tap_protocol::Satscard>> g_satscards { };
std::vector<std::unique_ptr<tap_protocol::Tapsigner>> g_tapsigners { };


char* AllocateCStringFromCpp(const std::string& cppString);

CKTapCardHandle ConstructTapCardHandle(const int32_t index, const int32_t type);

template <typename R, typename Func>
auto GetFromTapCard(const int32_t index, const int32_t type, R&& defaultReturn, const Func& getterFunction)
{
    const auto handle = ConstructTapCardHandle(index, type);
    const auto processCard = [&handle, &defaultReturn, &getterFunction](const auto& vector)
    {
        if (handle.index < 0 || handle.index >= vector.size() )
        {
            return std::move(defaultReturn);
        }

        const auto& card = vector[static_cast<size_t>(handle.index)];
        if (!card)
        {
            return std::move(defaultReturn);
        }
        if ((card->IsTapsigner() && handle.type != CKTapCardType::Tapsigner) ||
            (!card->IsTapsigner() && handle.type != CKTapCardType::Satscard))
        {
            return std::move(defaultReturn);
        }
        
        return getterFunction(*card);
    };

    switch (handle.type)
    {
        case CKTapCardType::Satscard:
            return processCard(g_satscards);
        case CKTapCardType::Tapsigner:
            return processCard(g_tapsigners);
        default:
            return std::move(defaultReturn);
    }
}

CKTapCardType IntToTapCardType(const int32_t type);

template <typename TapCardType>
size_t UpdateVectorWithTapCard(std::vector<std::unique_ptr<TapCardType>>& vector, std::unique_ptr<TapCardType>& card)
{
    if (!card)
    {
        return invalidIndex;
    }
    
    for (size_t index { 0 }; index < vector.size(); ++index)
    {
        if (vector[index] && vector[index]->GetIdent() == card->GetIdent())
        {
    
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
std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<U>& pointer)
{
    if (T* castPointer = dynamic_cast<T*>(pointer.get()))
    {
        pointer.release();
        return std::unique_ptr<T>{ castPointer };
    }

    return nullptr;
}

/// @brief Special overload for unique pointers
template <typename T, typename U>
std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U>& pointer)
{
    if (T* castPointer = static_cast<T*>(pointer.get()))
    {
        pointer.release();
        return std::unique_ptr<T>{ castPointer };
    }

    return nullptr;
}

#endif // __CKTAP_PROTOCOL__INTERNAL_UTILS_H__
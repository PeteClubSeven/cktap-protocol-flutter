#ifndef __CKTAP_PROTOCOL__INTERNAL_GLOBALS_H__
#define __CKTAP_PROTOCOL__INTERNAL_GLOBALS_H__

// Project
#include <internal/utils.h>
#include <structs.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <numeric>
#include <memory>

// Types
class TapProtocolThread;
struct SatscardWrapper {
    std::unique_ptr<tap_protocol::Satscard> card { };
    std::vector<std::unique_ptr<tap_protocol::Satscard::Slot>> slots { };

    SatscardWrapper(std::unique_ptr<tap_protocol::Satscard> satscard)
        : card{ std::move(satscard) }, slots{ } {
    }
};
struct TapsignerWrapper {
    std::unique_ptr<tap_protocol::Tapsigner> card { };

    TapsignerWrapper(std::unique_ptr<tap_protocol::Tapsigner> tapsigner)
        : card{ std::move(tapsigner) } {
    }
};

// Globals
extern std::unique_ptr<TapProtocolThread> g_protocolThread;
extern std::vector<SatscardWrapper> g_satscards;
extern std::vector<TapsignerWrapper> g_tapsigners;

// Constants
constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();

template <typename CardType, typename Func>
CKTapInterfaceStatus accessTapCard(const int32_t index, const Func& readFunction) {
    constexpr bool isSatscard = std::is_same_v<CardType, tap_protocol::Satscard>;
    constexpr bool isTapsigner = std::is_same_v<CardType, tap_protocol::Tapsigner>;

    const auto processCard = [index, &readFunction](auto& vector) {
        CKTapInterfaceStatus status;
        std::memset(&status, 0, sizeof(CKTapInterfaceStatus));
        if (index < 0 || index >= vector.size()) {
            status.errorCode = isSatscard ?
            CKTapInterfaceErrorCode::unknownSatscardHandle :
            CKTapInterfaceErrorCode::unknownTapsignerHandle;
        } else {
            auto& wrapper = vector[static_cast<size_t>(index)];
            try {
                status.errorCode = readFunction(wrapper);
            } catch (const std::exception& e) {
                // TODO: Make this much safer. There seems to be a bug either with FFI or with the
                // compiler where trying to catch a tap_protocol::TapProtoException doesn't work. This
                // causes a full app crash. This is replicable with the following code:
                /* void triggerException() {
                        tap_protocol::CKTapCard(tap_protocol::MakeDefaultTransport([](const auto& bytes) {
                            return tap_protocol::Bytes{ }; // <- This triggers a TapProtoException as expected
                        }));
                    }
                    extern "C" void myCrashingFunction() {
                        try {
                            triggerException();
                        } catch (const tap_protocol::TapProtoException& e) {
                            // This never catches the exception. It's completely ignored and the app will
                            // hard crash with no warning.
                        }
                    }
                    extern "C" void myWorkingFunction() {
                        try {
                            triggerException();
                        } catch (const std::exception& e) {
                            // This will always catch the exception! Success!
                        }
                    }
                 */
                const auto& tapProtoException = static_cast<const tap_protocol::TapProtoException&>(e);
                status.errorCode = CKTapInterfaceErrorCode::caughtTapProtocolException;
                status.exception = allocateCKTapProtoException(tapProtoException);
            } catch (...) {
                status.errorCode = CKTapInterfaceErrorCode::unknownErrorDuringTapProtocolFunction;
            }
        }
        return status;
    };

    if constexpr (isSatscard) {
        return processCard(g_satscards);
    }
    else if constexpr (isTapsigner) {
        return processCard(g_tapsigners);
    }
    else {
        static_assert("Unsupported CKTapCard");
    }
}

template <typename WrapperType, typename CardType>
size_t updateVectorWithTapCard(std::vector<WrapperType>& vector, std::unique_ptr<CardType>& card) {
    static_assert(std::is_same_v<
        std::remove_reference_t<decltype(card)>,
        std::remove_reference_t<decltype(vector[0].card)>
    >);

    if (!card) {
        return invalidIndex;
    }
    for (size_t index{ 0 }; index < vector.size(); ++index) {
        auto& stored = vector[index];
        if (stored.card && stored.card->GetIdent() == card->GetIdent()) {
            stored = WrapperType { std::move(card) };
            return index;
        }
    }

    const auto index = vector.size();
    vector.emplace_back(std::move(card));
    return index;
}

#endif //__CKTAP_PROTOCOL__INTERNAL_GLOBALS_H__

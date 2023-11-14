#ifndef __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
#define __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__

// Project
#include <enums.h>
#include <structs.h>

// Third party
#include <tap_protocol/tap_protocol.h>

// STL
#include <atomic>
#include <future>
#include <optional>

// Forward declarations
namespace tap_protocol {
    class CKTapCard;
    class Satscard;
    class Tapsigner;
}

class TapProtocolThread {
public:

    static TapProtocolThread* createNew() noexcept;
    CKTapInterfaceErrorCode reset() noexcept;
    void requestCancel() noexcept;

    bool prepareCardOperation(std::weak_ptr<tap_protocol::Satscard> satscard) noexcept;
    bool prepareCardOperation(std::weak_ptr<tap_protocol::Tapsigner> tapsigner) noexcept;
    bool beginCardHandshake(int32_t cardType) noexcept;
    bool beginSatscard_unseal(const char* cvc) noexcept;
    bool beginSatscard_new(const uint8_t* chainCode, int32_t chainCodeLength, const char* cvc) noexcept;
    bool beginSatscard_getSlot(int32_t slot, const char* cvc) noexcept;
    bool beginSatscard_listSlots(const char* cvc, int32_t limit) noexcept;
    bool finalizeOperation() noexcept;

    bool hasStarted() const;
    bool hasFailed() const;
    bool hasFinished() const;
    bool isThreadActive() const;
    CKTapThreadState getState() const;
    CKTapInterfaceErrorCode getRecentErrorCode() const;
    bool getTapProtocolException(CKTapProtoException& outException) const;

    std::optional<const tap_protocol::Bytes*> getTransportRequest() const;
    std::optional<uint8_t*> allocateTransportResponseBuffer(size_t sizeInBytes);
    bool finalizeTransportResponse();

    std::optional<CKTapCardType> getConstructedCardType() const;
    std::unique_ptr<tap_protocol::Satscard> releaseConstructedSatscard();
    std::unique_ptr<tap_protocol::Tapsigner> releaseConstructedTapsigner();

private:

    template <typename Func>
    bool _startAsyncCardOperation(Func&& func) noexcept;

    void _cancelIfNecessary();
    std::unique_ptr<tap_protocol::CKTapCard> _performHandshake(int32_t cardType);
    void _signalTransportRequestReady(const tap_protocol::Bytes& bytes);

    std::future<CKTapInterfaceErrorCode> _future{ };

    std::atomic<CKTapThreadState> _state{ CKTapThreadState::notStarted };
    std::atomic<bool> _shouldCancel { false };
    std::atomic<CKTapInterfaceErrorCode> _recentError{ CKTapInterfaceErrorCode::threadNotYetStarted };

    tap_protocol::TapProtoException _tapProtoException{ 0, { } };
    tap_protocol::Bytes _pendingTransportRequest{ };
    tap_protocol::Bytes _transportResponse{ };

    std::unique_ptr<tap_protocol::CKTapCard> _constructedCard{ };
    std::weak_ptr<tap_protocol::Satscard> _satscard{ };
    std::weak_ptr<tap_protocol::Tapsigner> _tapsigner{ };
};

#endif // __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
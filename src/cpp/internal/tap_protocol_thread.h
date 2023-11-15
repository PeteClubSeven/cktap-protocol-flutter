#ifndef __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
#define __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__

// Project
#include <enums.h>
#include <internal/card_operation.h>
#include <structs.h>

// Third party
#include <tap_protocol/tap_protocol.h>

// STL
#include <atomic>
#include <future>
#include <optional>

class TapProtocolThread {
public:

    static TapProtocolThread* createNew() noexcept;
    CKTapInterfaceErrorCode reset() noexcept;
    void requestCancel() noexcept;

    bool prepareCardOperation(std::weak_ptr<tap_protocol::Satscard> satscard) noexcept;
    bool prepareCardOperation(std::weak_ptr<tap_protocol::Tapsigner> tapsigner) noexcept;
    bool beginCardHandshake(int32_t cardType) noexcept;
    bool beginCKTapCard_Wait();
    bool beginSatscard_CertificateCheck();
    bool beginSatscard_GetSlot(int32_t slot, const char* cvc);
    bool beginSatscard_ListSlots(const char* cvc, int32_t limit);
    bool beginSatscard_New(const char* chainCode, const char* cvc);
    bool beginSatscard_Unseal(const char* cvc);
    bool finalizeOperation() noexcept;

    template <CardOperation op, typename R = CardResponseType<op>>
    std::optional<R> getResponse() const noexcept;

    bool hasStarted() const noexcept;
    bool hasFailed() const noexcept;
    bool hasFinished() const noexcept;
    bool isThreadActive() const noexcept;
    CKTapThreadState getState() const noexcept;
    CKTapInterfaceErrorCode getRecentErrorCode() const noexcept;
    bool getTapProtocolException(CKTapProtoException& outException) const noexcept;

    std::optional<const tap_protocol::Bytes*> getTransportRequest() const;
    std::optional<uint8_t*> allocateTransportResponseBuffer(size_t sizeInBytes);
    bool finalizeTransportResponse();

    std::optional<CKTapCardType> getConstructedCardType() const;
    std::unique_ptr<tap_protocol::Satscard> releaseConstructedSatscard();
    std::unique_ptr<tap_protocol::Tapsigner> releaseConstructedTapsigner();

private:

    template <CardOperation op, typename... Args>
    auto _setResponse(Args&&... args);

    template <typename Func>
    bool _startAsyncCardOperation(Func&& func) noexcept;
    void _cancelIfNecessary();

    std::shared_ptr<tap_protocol::CKTapCard> _lockCardForOperation() const noexcept;
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

    /// Allows for us to store the response types to any CKTapCard/Satscard/Tapsigner function in a
    /// type-safe manner
    CardResponseVariant _cardOperationResponse{ };
};

template <CardOperation op, typename R>
std::optional<R> TapProtocolThread::getResponse() const noexcept {
    try {
        if (!isThreadActive()) {
            constexpr size_t index = static_cast<size_t>(op);
            return { std::get<index>(_cardOperationResponse) };
        }
    } catch (...) {
    }
    return { };
}

template <CardOperation op, typename... Args>
auto TapProtocolThread::_setResponse(Args&&... args) {
    constexpr auto index = static_cast<size_t>(op);
    return _cardOperationResponse.emplace<index>(std::forward<Args>(args)...);
}

#endif // __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
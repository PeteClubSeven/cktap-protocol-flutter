#ifndef __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
#define __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__

// Project
#include <Enums.h>
#include <Structs.h>

// Third party
#include <tap_protocol/tap_protocol.h>

// STL
#include <atomic>
#include <future>
#include <optional>

// Forward declarations
namespace tap_protocol
{
    class CKTapCard;
    class Satscard;
    class Tapsigner;
}

class TapProtocolThread
{
public:

    static TapProtocolThread* CreateNew();
    CKTapInterfaceErrorCode Reset();

    bool BeginCardHandshake();
    bool FinalizeCardHandshake();

    bool HasStarted() const;
    bool HasFailed() const;
    bool HasFinished() const;
    bool IsThreadActive() const;
    CKTapThreadState GetState() const;
    CKTapInterfaceErrorCode GetRecentErrorCode() const;
    bool GetTapProtocolException(CKTapProtoException& outException) const;

    std::optional<const tap_protocol::Bytes*> GetTransportRequest() const;
    std::optional<uint8_t*> AllocateTransportResponseBuffer(const size_t sizeInBytes);
    bool FinalizeTransportResponse();

    std::optional<bool> IsTapsigner() const;
    std::unique_ptr<tap_protocol::Satscard> ReleaseSatscard();
    std::unique_ptr<tap_protocol::Tapsigner> ReleaseTapsigner();

private:

    std::unique_ptr<tap_protocol::CKTapCard> PerformHandshake();

    void SignalTransportRequestReady(const tap_protocol::Bytes& bytes);

    std::future<CKTapInterfaceErrorCode> _future { };

    std::atomic<CKTapThreadState> _state { CKTapThreadState::NotStarted };
    std::atomic<CKTapInterfaceErrorCode> _recentError { CKTapInterfaceErrorCode::ThreadNotYetStarted };
    
    tap_protocol::TapProtoException _tapProtoException { 0, { } };
    
    tap_protocol::Bytes _pendingTransportRequest { };
    tap_protocol::Bytes _transportResponse { };

    std::unique_ptr<tap_protocol::CKTapCard> _card { };
};

#endif // __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
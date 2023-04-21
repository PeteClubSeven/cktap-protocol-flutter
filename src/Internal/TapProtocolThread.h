#ifndef __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
#define __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__

// Project
#include <Enums.h>

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

    bool HasStarted() const;
    bool HasFailed() const;
    bool HasFinished() const;
    CKTapThreadState GetState() const;

    std::optional<const tap_protocol::Bytes*> GetTransportRequest() const;
    std::optional<uint8_t*> AllocateTransportResponseBuffer(const size_t sizeInBytes);
    bool FinalizeTransportResponse();

    std::optional<bool> IsTapsigner() const;
    std::shared_ptr<tap_protocol::Satscard> GetSatscard() const;
    std::shared_ptr<tap_protocol::Tapsigner> GetTapsigner() const;

private:

    std::shared_ptr<tap_protocol::CKTapCard> Initialize();

    void SignalTransportRequestReady(const tap_protocol::Bytes& bytes);

    std::future<CKTapInterfaceErrorCode> m_future { };
    std::atomic<CKTapThreadState> m_state { CKTapThreadState::NotStarted };
    
    int m_tapProtocolErrorCode { 0 };
    std::string m_tapProtocolErrorMessage { };
    
    tap_protocol::Bytes m_awaitingTransport { };
    tap_protocol::Bytes m_transportResponse { };

    std::shared_ptr<tap_protocol::CKTapCard> m_tapcard { };
};

#endif // __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
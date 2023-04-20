#ifndef __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
#define __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__

// Project
#include <Enums.h>

// Third party
#include <tap_protocol/tap_protocol.h>

// STL
#include <atomic>
#include <optional>
#include <thread>

// Forward declarations
namespace tap_protocol
{
    class Tapsigner;
    class Satscard;
}

class TapProtocolThread
{
public:

    static TapProtocolThread* CreateNew();

    bool HasStarted() const;
    bool HasFinished() const;
    CKTapThreadState GetState() const;

    std::optional<const tap_protocol::Bytes*> GetTransportRequest() const;
    
    std::optional<uint8_t*> AllocateTransportResponseBuffer(const size_t sizeInBytes);
    bool FinalizeTransportResponse();

private:

    void Initialize();

    void SignalTransportRequestReady(const tap_protocol::Bytes& bytes);

    std::thread m_thread { };
    std::atomic<CKTapThreadState> m_state { CKTapThreadState::NotStarted };
    
    tap_protocol::Bytes m_awaitingTransport { };
    tap_protocol::Bytes m_transportResponse { };

    std::unique_ptr<tap_protocol::Tapsigner> m_pTapsigner { };
    std::unique_ptr<tap_protocol::Satscard> m_pSatscard { };
};

#endif // __CKTAP_PROTOCOL__INTERNAL_TAPPROTOCOLTHREAD_H__
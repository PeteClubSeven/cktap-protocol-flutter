#include "TapProtocolThread.h"

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <chrono>

using namespace std::chrono_literals;

TapProtocolThread* TapProtocolThread::CreateNew()
{
    auto protocol = new TapProtocolThread { };
    if (protocol)
    {
        protocol->m_thread = std::thread([protocol]()
        { 
            protocol->Initialize();
        });
    }

    return protocol;
}

bool TapProtocolThread::HasStarted() const
{
    return m_state != CKTapThreadState::NotStarted;
}

bool TapProtocolThread::HasFinished() const
{
    return m_state == CKTapThreadState::Finished;
}

CKTapThreadState TapProtocolThread::GetState() const
{
    return m_state;
}

std::optional<const tap_protocol::Bytes*> TapProtocolThread::GetTransportRequest() const
{
    if (m_state != CKTapThreadState::TransportRequestReady)
    {
        return { };
    }

    return &m_awaitingTransport;
}

std::optional<uint8_t*> TapProtocolThread::AllocateTransportResponseBuffer(const size_t sizeInBytes)
{
    if (m_state != CKTapThreadState::TransportRequestReady)
    {
        return { };
    }

    m_transportResponse.resize(sizeInBytes);
    return m_transportResponse.data();
}

bool TapProtocolThread::FinalizeTransportResponse()
{
    if (m_state != CKTapThreadState::TransportRequestReady)
    {
        return false;
    }

    m_state = CKTapThreadState::TransportResponseReady;
    return m_state == CKTapThreadState::TransportResponseReady;
}

void TapProtocolThread::Initialize()
{
    m_state = CKTapThreadState::AwaitingTransportRequest;

    auto tapcard = tap_protocol::CKTapCard(tap_protocol::MakeDefaultTransport([this](const tap_protocol::Bytes& bytes)
    {
        if (m_state == CKTapThreadState::ProcessingTransportResponse)
        {
            // Sometimes we may need to send multiple messages during a single transmission
            m_state = CKTapThreadState::AwaitingTransportRequest;
        }
        else if (m_state != CKTapThreadState::AwaitingTransportRequest)
        {
            return tap_protocol::Bytes { };
        }

        // Allow the library to take our transport data
        SignalTransportRequestReady(bytes);

        // Wait for our library to be transmitted through Flutter
        auto currentTime = std::chrono::high_resolution_clock::now();
        const auto endTime = currentTime + 10min;
        do 
        {
            std::this_thread::sleep_for(100ns);
            std::this_thread::yield();
            currentTime = std::chrono::high_resolution_clock::now();
        } while (m_state != CKTapThreadState::TransportResponseReady && currentTime < endTime);

        // Handle timeouts
        if (m_state != CKTapThreadState::TransportResponseReady)
        {
            throw std::runtime_error("TapProtocolThread::Initialize::MakeDefaultTransport(): Timed out waiting for transport response, current state is \"" + 
                std::to_string(m_state) + '"');
        }

        m_state = CKTapThreadState::ProcessingTransportResponse;
        return m_transportResponse;
    }));

    m_state = CKTapThreadState::AwaitingTransportRequest;
    if (tapcard.IsTapsigner())
    {
        m_pTapsigner = tap_protocol::ToTapsigner(std::move(tapcard));
    }
    else
    {
        m_pSatscard = tap_protocol::ToSatscard(std::move(tapcard));
    }
    
    m_state = CKTapThreadState::Finished;
}

void TapProtocolThread::SignalTransportRequestReady(const tap_protocol::Bytes& bytes)
{
    if (m_state != CKTapThreadState::AwaitingTransportRequest)
    {
        throw std::runtime_error("TapProtocolThread::SetTransportRequest(): Attempt to set transport request when state was \"" + 
            std::to_string(m_state) + '\"');
    }
    
    m_awaitingTransport = bytes;
    m_state = CKTapThreadState::TransportRequestReady;
}
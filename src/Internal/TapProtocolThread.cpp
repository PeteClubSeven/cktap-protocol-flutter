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
        protocol->m_future = std::async(std::launch::async, [protocol]()
        { 
            try
            {
                protocol->m_tapProtocolErrorCode = 0;
                protocol->m_tapProtocolErrorMessage.resize(0);

                protocol->m_tapcard = std::move(protocol->Initialize());
                protocol->m_state = CKTapThreadState::Finished;

                return CKTapInterfaceErrorCode::Success;
            }
            catch (const tap_protocol::TapProtoException& e)
            {
                protocol->m_tapcard.reset();
                protocol->m_tapProtocolErrorCode = e.code();
                protocol->m_tapProtocolErrorMessage = e.what();
                protocol->m_state = CKTapThreadState::TapProtocolError;
                return CKTapInterfaceErrorCode::ThreadEncounterTapProtocolError;
            }
            catch (const std::runtime_error& e)
            {
                protocol->m_tapcard.reset();
                protocol->m_state = CKTapThreadState::Timeout;
                return CKTapInterfaceErrorCode::ThreadTimeoutDuringTransport;
            }
            catch (...)
            {
                return CKTapInterfaceErrorCode::UnknownErrorDuringInitialization;
            }
        });
    }

    return protocol;
}

bool TapProtocolThread::HasStarted() const
{
    return m_state != CKTapThreadState::NotStarted;
}

bool TapProtocolThread::HasFailed() const
{
    return m_state == CKTapThreadState::Timeout || m_state == CKTapThreadState::TapProtocolError;
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

std::shared_ptr<tap_protocol::CKTapCard> TapProtocolThread::Initialize()
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
        const auto endTime = currentTime + 1min;
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

    // More transport operations may need to be performed when we turn the card into a Tapsigner or Satscard
    m_state = CKTapThreadState::AwaitingTransportRequest;
    if (tapcard.IsTapsigner())
    {
        return std::shared_ptr<tap_protocol::Tapsigner>(std::move(tap_protocol::ToTapsigner(std::move(tapcard))));
    }
    else
    {
        return std::shared_ptr<tap_protocol::Satscard>(std::move(tap_protocol::ToSatscard(std::move(tapcard))));
    }
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

std::optional<bool> TapProtocolThread::IsTapsigner() const {
    if (m_future.valid() && m_state == CKTapThreadState::Finished)
    {
        return m_tapcard->IsTapsigner();
    }

    return { };
}

std::shared_ptr<tap_protocol::Satscard> TapProtocolThread::GetSatscard() const {
    if (m_future.valid() && m_state == CKTapThreadState::Finished && m_tapcard && !m_tapcard->IsTapsigner())
    {
        return std::static_pointer_cast<tap_protocol::Satscard>(m_tapcard);
    }

    return { };
}

std::shared_ptr<tap_protocol::Tapsigner> TapProtocolThread::GetTapsigner() const {
    if (m_future.valid() && m_state == CKTapThreadState::Finished && m_tapcard && m_tapcard->IsTapsigner())
    {
        return std::static_pointer_cast<tap_protocol::Tapsigner>(m_tapcard);
    }

    return { };
}

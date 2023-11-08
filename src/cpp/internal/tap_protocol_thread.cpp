#include <internal/tap_protocol_thread.h>

// Project
#include <internal/exceptions.h>
#include <internal/utils.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <chrono>
#include <cstring>

using namespace std::chrono_literals;

TapProtocolThread* TapProtocolThread::CreateNew() {
    if (auto thread = new TapProtocolThread{ }) {
        if (thread->Reset() == CKTapInterfaceErrorCode::Success) {
            return thread;
        }

        delete thread;
    }
    return nullptr;
}

CKTapInterfaceErrorCode TapProtocolThread::Reset() {
    if (IsThreadActive()) {
        return CKTapInterfaceErrorCode::ThreadAlreadyInUse;
    }

    _future = std::future<CKTapInterfaceErrorCode>{ };
    _state = CKTapThreadState::NotStarted;
    _recentError = CKTapInterfaceErrorCode::Pending;
    _tapProtoException = tap_protocol::TapProtoException{ 0, { } };
    _pendingTransportRequest.clear();
    _transportResponse.clear();
    _card.reset();

    return CKTapInterfaceErrorCode::Success;
};

bool TapProtocolThread::BeginCardHandshake() {
    _future = std::async(std::launch::async, [this]() {
    try {
        _state = CKTapThreadState::AwaitingTransportRequest;
        if (auto card = PerformHandshake()) {
            _card = std::move(card);
            _state = CKTapThreadState::Finished;
            return CKTapInterfaceErrorCode::Success;
        } else {
            _state = CKTapThreadState::Failed;
            return CKTapInterfaceErrorCode::FailedToPerformHandshake;
        }
    } catch (tap_protocol::TapProtoException e) {
        _tapProtoException = std::move(e);
        _state = CKTapThreadState::TapProtocolError;
        return CKTapInterfaceErrorCode::CaughtTapProtocolException;
    } catch (const TimeoutException& e) {
        _state = CKTapThreadState::Timeout;
        return CKTapInterfaceErrorCode::TimeoutDuringTransport;
    } catch (...) {
        // TODO: Add more exception handling and message extraction
        return CKTapInterfaceErrorCode::UnknownErrorDuringHandshake;
    }});

    return _future.valid();
}


bool TapProtocolThread::FinalizeCardHandshake() {
    if ((HasFinished() || HasFailed()) && _future.valid()) {
        _recentError = _future.get();
        return true;
    }

    return false;
}

bool TapProtocolThread::HasStarted() const {
    return _state != CKTapThreadState::NotStarted;
}

bool TapProtocolThread::HasFailed() const {
    return _state > CKTapThreadState::Finished;
}

bool TapProtocolThread::HasFinished() const {
    return _state == CKTapThreadState::Finished;
}

bool TapProtocolThread::IsThreadActive() const {
    return HasStarted() && !HasFinished() && !HasFailed();
}

CKTapThreadState TapProtocolThread::GetState() const {
    return _state;
}

CKTapInterfaceErrorCode TapProtocolThread::GetRecentErrorCode() const {
    return _recentError;
}

bool TapProtocolThread::GetTapProtocolException(CKTapProtoException& outException) const {
    if (_state == CKTapThreadState::TapProtocolError) {
        outException.code = _tapProtoException.code();
        outException.message = strdup(_tapProtoException.what());
        return _state == CKTapThreadState::TapProtocolError;
    }

    return false;
}

std::optional<const tap_protocol::Bytes*> TapProtocolThread::GetTransportRequest() const {
    if (_state != CKTapThreadState::TransportRequestReady) {
        return { };
    }

    return &_pendingTransportRequest;
}

std::optional<uint8_t*> TapProtocolThread::AllocateTransportResponseBuffer(const size_t sizeInBytes) {
    if (_state != CKTapThreadState::TransportRequestReady) {
        return { };
    }

    _transportResponse.resize(sizeInBytes);
    return _transportResponse.data();
}

bool TapProtocolThread::FinalizeTransportResponse() {
    if (_state != CKTapThreadState::TransportRequestReady) {
        return false;
    }

    _state = CKTapThreadState::TransportResponseReady;
    return _state == CKTapThreadState::TransportResponseReady;
}

std::unique_ptr<tap_protocol::CKTapCard> TapProtocolThread::PerformHandshake() {
    auto card = tap_protocol::CKTapCard(tap_protocol::MakeDefaultTransport([this](const tap_protocol::Bytes& bytes) {
        if (_state == CKTapThreadState::ProcessingTransportResponse) {
            // Sometimes we may need to send multiple messages during a single transmission
            _state = CKTapThreadState::AwaitingTransportRequest;
        }
        if (_state != CKTapThreadState::AwaitingTransportRequest) {
            return tap_protocol::Bytes{ };
        }

        // Allow the library to take our transport data
        SignalTransportRequestReady(bytes);

        // Wait for our library to be transmitted through Flutter
        auto currentTime = std::chrono::high_resolution_clock::now();
        const auto endTime = currentTime + 1min;
        do {
            std::this_thread::sleep_for(100ns);
            std::this_thread::yield();
            currentTime = std::chrono::high_resolution_clock::now();
        } while (_state != CKTapThreadState::TransportResponseReady && currentTime < endTime);

        // Handle timeouts
        if (_state != CKTapThreadState::TransportResponseReady) {
            throw TimeoutException(
                "TapProtocolThread::PerformHandshake::MakeDefaultTransport(): Timed out waiting for transport response, current state is \"" +
                    std::to_string(_state) + '"'
            );
        }

        _state = CKTapThreadState::ProcessingTransportResponse;
        return _transportResponse;
    }));

    // More transport operations may need to be performed when we turn the card into a Tapsigner or Satscard
    _state = CKTapThreadState::AwaitingTransportRequest;
    if (card.IsTapsigner()) {
        return tap_protocol::ToTapsigner(std::move(card));
    } else {
        return tap_protocol::ToSatscard(std::move(card));
    }
}

void TapProtocolThread::SignalTransportRequestReady(const tap_protocol::Bytes& bytes) {
    if (_state != CKTapThreadState::AwaitingTransportRequest) {
        throw std::runtime_error("TapProtocolThread::SetTransportRequest(): Attempt to set transport request when state was \"" +
            std::to_string(_state) + '\"'
        );
    }

    _pendingTransportRequest = bytes;
    _state = CKTapThreadState::TransportRequestReady;
}

std::optional<bool> TapProtocolThread::IsTapsigner() const {
    if (!IsThreadActive()) {
        return _card->IsTapsigner();
    }

    return { };
}

std::unique_ptr<tap_protocol::Satscard> TapProtocolThread::ReleaseSatscard() {
    return !IsThreadActive() ?
        std::move(dynamic_pointer_cast<tap_protocol::Satscard>(_card)) :
        nullptr;
}

std::unique_ptr<tap_protocol::Tapsigner> TapProtocolThread::ReleaseTapsigner() {
    return !IsThreadActive() ?
        std::move(dynamic_pointer_cast<tap_protocol::Tapsigner>(_card)) :
        nullptr;
}

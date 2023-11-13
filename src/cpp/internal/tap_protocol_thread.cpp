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

TapProtocolThread* TapProtocolThread::createNew() noexcept {
    try {
        auto thread = new TapProtocolThread{ };
        if (thread->reset() == CKTapInterfaceErrorCode::success) {
            return thread;
        }

        delete thread;
    } catch (...) { }
    return nullptr;
}

CKTapInterfaceErrorCode TapProtocolThread::reset() noexcept {
    if (isThreadActive()) {
        return CKTapInterfaceErrorCode::threadAlreadyInUse;
    }

    _future = std::future<CKTapInterfaceErrorCode>{ };
    _state = CKTapThreadState::notStarted;
    _shouldCancel = false;
    _recentError = CKTapInterfaceErrorCode::pending;
    _tapProtoException = tap_protocol::TapProtoException{ 0, { } };
    _pendingTransportRequest.clear();
    _transportResponse.clear();
    _constructedCard.reset();

    return CKTapInterfaceErrorCode::success;
};

void TapProtocolThread::requestCancel() {
    _shouldCancel = true;
}

bool TapProtocolThread::beginCardHandshake(const int32_t cardType) {
    _state = CKTapThreadState::asyncActionStarting;
    _future = std::async(std::launch::async, [this, cardType]() {
    try {
        _state = CKTapThreadState::awaitingTransportRequest;
        if (auto card = _performHandshake(cardType)) {
            // Perform additional validation because tap_protocol doesn't detect an error when constructing a Tapsigner
            // and communicating with a Satscard
            if ((cardType == CKTapCardType::satscard && !card->IsTapsigner()) ||
                (cardType == CKTapCardType::tapsigner && card->IsTapsigner()) ||
                (cardType == CKTapCardType::unknownCard)) {
                _constructedCard = std::move(card);
                _state = CKTapThreadState::finished;
                return CKTapInterfaceErrorCode::success;
            }
        }
        _state = CKTapThreadState::invalidCardProduced;
        return CKTapInterfaceErrorCode::failedToPerformHandshake;
    } catch (const CancelationException& e) {
        _state = CKTapThreadState::canceled;
        return CKTapInterfaceErrorCode::operationCanceled;
    } catch (const TimeoutException& e) {
        _state = CKTapThreadState::timeout;
        return CKTapInterfaceErrorCode::timeoutDuringTransport;
    } catch (const std::exception& e) {
        // TODO: Fix weird TapProtoException issue. See "globals.h"
        auto exception = static_cast<const tap_protocol::TapProtoException&>(e);
        _tapProtoException = exception;
        _state = CKTapThreadState::tapProtocolError;
        return CKTapInterfaceErrorCode::caughtTapProtocolException;
    } catch (...) {
        // TODO: Add more exception handling and message extraction
        _state = CKTapThreadState::failed;
        return CKTapInterfaceErrorCode::unknownErrorDuringHandshake;
    }});

    return _future.valid();
}


bool TapProtocolThread::finalizeOperation() {
    if ((hasFinished() || hasFailed()) && _future.valid()) {
        _recentError = _future.get();
        return true;
    }

    return false;
}

bool TapProtocolThread::hasStarted() const {
    return _state != CKTapThreadState::notStarted;
}

bool TapProtocolThread::hasFailed() const {
    return _state > CKTapThreadState::finished;
}

bool TapProtocolThread::hasFinished() const {
    return _state == CKTapThreadState::finished;
}

bool TapProtocolThread::isThreadActive() const {
    return hasStarted() && !hasFinished() && !hasFailed();
}

CKTapThreadState TapProtocolThread::getState() const {
    return _state;
}

CKTapInterfaceErrorCode TapProtocolThread::getRecentErrorCode() const {
    return _recentError;
}

bool TapProtocolThread::getTapProtocolException(CKTapProtoException& outException) const {
    if (_state == CKTapThreadState::tapProtocolError) {
        outException = allocateCKTapProtoException(_tapProtoException);
        return true;
    }

    return false;
}

std::optional<const tap_protocol::Bytes*> TapProtocolThread::getTransportRequest() const {
    if (_state != CKTapThreadState::transportRequestReady) {
        return { };
    }

    return &_pendingTransportRequest;
}

std::optional<uint8_t*> TapProtocolThread::allocateTransportResponseBuffer(size_t sizeInBytes) {
    if (_state != CKTapThreadState::transportRequestReady) {
        return { };
    }

    _transportResponse.resize(sizeInBytes);
    return _transportResponse.data();
}

bool TapProtocolThread::finalizeTransportResponse() {
    if (_state != CKTapThreadState::transportRequestReady) {
        return false;
    }

    _state = CKTapThreadState::transportResponseReady;
    return true;
}

std::optional<CKTapCardType> TapProtocolThread::getConstructedCardType() const {
    if (!isThreadActive()) {
        if (_constructedCard == nullptr) {
            return CKTapCardType::unknownCard;
        }
        return _constructedCard->IsTapsigner() ?
            CKTapCardType::tapsigner :
            CKTapCardType::satscard;
    }
    return { };
}

std::unique_ptr<tap_protocol::Satscard> TapProtocolThread::releaseConstructedSatscard() {
    return !isThreadActive() ?
        std::move(dynamic_pointer_cast<tap_protocol::Satscard>(_constructedCard)) :
        nullptr;
}

std::unique_ptr<tap_protocol::Tapsigner> TapProtocolThread::releaseConstructedTapsigner() {
    return !isThreadActive() ?
        std::move(dynamic_pointer_cast<tap_protocol::Tapsigner>(_constructedCard)) :
        nullptr;
}

void TapProtocolThread::_cancelIfNecessary() {
    if (_shouldCancel) {
        throw CancelationException("Canceling operation in TapProtocolThread");
    }
}

std::unique_ptr<tap_protocol::CKTapCard> TapProtocolThread::_performHandshake(const int32_t cardType) {
    auto transport = tap_protocol::MakeDefaultTransport([this](const tap_protocol::Bytes& bytes) {
        if (_state == CKTapThreadState::processingTransportResponse) {
            // Sometimes we may need to send multiple messages during a single transmission
            _state = CKTapThreadState::awaitingTransportRequest;
        }
        if (_state != CKTapThreadState::awaitingTransportRequest) {
            return tap_protocol::Bytes{ };
        }

        // Allow the library to take our transport data
        _signalTransportRequestReady(bytes);

        // Wait for our library to be transmitted through Flutter
        auto currentTime = std::chrono::high_resolution_clock::now();
        const auto endTime = currentTime + 1min;
        do {
            _cancelIfNecessary();
            std::this_thread::sleep_for(100ns);
            std::this_thread::yield();
            currentTime = std::chrono::high_resolution_clock::now();
        } while (_state != CKTapThreadState::transportResponseReady && currentTime < endTime);

        // Handle timeouts
        if (_state != CKTapThreadState::transportResponseReady) {
            throw TimeoutException(
                "TapProtocolThread::_performHandshake::MakeDefaultTransport(): Timed out waiting for transport response, current state is \"" +
                    std::to_string(_state) + '"'
            );
        }

        _cancelIfNecessary();
        _state = CKTapThreadState::processingTransportResponse;
        return _transportResponse;
    });

    // Construct the classes directly if we've been given a hint
    _cancelIfNecessary();
    if (cardType == CKTapCardType::satscard) {
        return std::make_unique<tap_protocol::Satscard>(std::move(transport));
    } else if (cardType == CKTapCardType::tapsigner) {
        return std::make_unique<tap_protocol::Tapsigner>(std::move(transport));
    }

    // We will have to manually figure out what the card is
    auto card = tap_protocol::CKTapCard(std::move(transport));

    // More transport operations may need to be performed when we turn the card into a Tapsigner or Satscard
    _cancelIfNecessary();
    _state = CKTapThreadState::awaitingTransportRequest;
    if (card.IsTapsigner()) {
        return tap_protocol::ToTapsigner(std::move(card));
    } else {
        return tap_protocol::ToSatscard(std::move(card));
    }
}

void TapProtocolThread::_signalTransportRequestReady(const tap_protocol::Bytes& bytes) {
    if (_state != CKTapThreadState::awaitingTransportRequest) {
        throw std::runtime_error("TapProtocolThread::SetTransportRequest(): Attempt to set transport request when state was \"" +
            std::to_string(_state) + '\"'
        );
    }

    _pendingTransportRequest = bytes;
    _state = CKTapThreadState::transportRequestReady;
}

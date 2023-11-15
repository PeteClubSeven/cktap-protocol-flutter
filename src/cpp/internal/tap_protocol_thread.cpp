#include <internal/tap_protocol_thread.h>

// Project
#include <internal/exceptions.h>
#include <internal/macros.h>
#include <internal/utils.h>

// Third party
#include <tap_protocol/cktapcard.h>
#include <tap_protocol/utils.h>

// STL
#include <chrono>
#include <cstring>

using namespace std::chrono_literals;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-static-cast-downcast"
template <typename Func>
bool TapProtocolThread::_startAsyncCardOperation(Func&& func) noexcept {
    try {
        _state = CKTapThreadState::asyncActionStarting;
        _future = std::async(std::launch::async, [this, func=std::forward<Func>(func)]() {
            try {
                const CKTapInterfaceErrorCode errorCode = func();
                _state = errorCode == CKTapInterfaceErrorCode::success ?
                    CKTapThreadState::finished :
                    CKTapThreadState::failed;

                return errorCode;
            } catch (const CancelationException& e) {
                _state = CKTapThreadState::canceled;
                return CKTapInterfaceErrorCode::operationCanceled;
            } catch (const TimeoutException& e) {
                _state = CKTapThreadState::timeout;
                return CKTapInterfaceErrorCode::timeoutDuringTransport;
            } catch (const TransportException& e) {
                _state = CKTapThreadState::transportException;
                return CKTapInterfaceErrorCode::invalidThreadStateDuringTransportSignaling;
            } CATCH_TAP_PROTO_EXCEPTION(e, {
                _tapProtoException = e;
                _state = CKTapThreadState::tapProtocolError;
                return CKTapInterfaceErrorCode::caughtTapProtocolException;
            }) catch (...) { }
            _state = CKTapThreadState::failed;
            return CKTapInterfaceErrorCode::unknownErrorDuringAsyncOperation;
        });

        return _future.valid();
    } catch (...) {}

    return false;
}
#pragma clang diagnostic pop

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
    _satscard.reset();
    _tapsigner.reset();
    _cardOperationResponse = CardResponseVariant{ };

    return CKTapInterfaceErrorCode::success;
}

void TapProtocolThread::requestCancel() noexcept {
    _shouldCancel = true;
}

bool TapProtocolThread::prepareCardOperation(std::weak_ptr<tap_protocol::Satscard> satscard) noexcept {
    if (isThreadActive() || satscard.expired()) {
        return false;
    }
    _satscard = std::move(satscard);
    _state = CKTapThreadState::awaitingCardOperation;
    return true;
}

bool TapProtocolThread::prepareCardOperation(std::weak_ptr<tap_protocol::Tapsigner> tapsigner) noexcept {
    if (isThreadActive() || tapsigner.expired()) {
        return false;
    }
    _tapsigner = std::move(tapsigner);
    _state = CKTapThreadState::awaitingCardOperation;
    return true;
}

bool TapProtocolThread::beginCardHandshake(const int32_t cardType) noexcept {
    return _startAsyncCardOperation([this, cardType]() {
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
    });
}

bool TapProtocolThread::beginCKTapCard_Wait() {
    if (auto card = _lockCardForOperation()) {
        return _startAsyncCardOperation([=]() {
            _setResponse<CardOperation::CKTapCard_Wait>(card->Wait());
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::beginSatscard_CertificateCheck() {
    if (auto card = _satscard.lock()) {
        return _startAsyncCardOperation([=]() {
            card->CertificateCheck();
            _setResponse<CardOperation::Satscard_CertificateCheck>(card->IsCertsChecked());
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::beginSatscard_GetSlot(int32_t slot, const char* cvc) {
    if (auto card = _satscard.lock()) {
        return _startAsyncCardOperation([=, cvc = makeCvc(cvc)]() {
            _setResponse<CardOperation::Satscard_GetSlot>(std::move(card->GetSlot(slot, cvc)));
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::beginSatscard_ListSlots(const char* cvc, int32_t limit) {
    if (auto card = _satscard.lock()) {
        return _startAsyncCardOperation([=, cvc = makeCvc(cvc)]() {
            auto result = card->ListSlots(cvc, static_cast<size_t>(limit));
            _setResponse<CardOperation::Satscard_ListSlots>(std::move(result));
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::beginSatscard_New(const char* chainCode, const char* cvc) {
    if (auto card = _satscard.lock()) {
        return _startAsyncCardOperation([=, chain = makeChainCode(chainCode), cvc = makeCvc(cvc)]() {
            _setResponse<CardOperation::Satscard_New>(std::move(card->New(chain, cvc)));
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::beginSatscard_Unseal(const char* cvc) {
    if (auto card = _satscard.lock()) {
        return _startAsyncCardOperation([=, cvc = makeCvc(cvc)]() {
            _setResponse<CardOperation::Satscard_Unseal>(std::move(card->Unseal(cvc)));
            return CKTapInterfaceErrorCode::success;
        });
    }
    return false;
}

bool TapProtocolThread::finalizeOperation() noexcept {
    if ((hasFinished() || hasFailed()) && _future.valid()) {
        try {
            _recentError = _future.get();
            return true;
        }
        catch (...) {
            _recentError = CKTapInterfaceErrorCode::failedToRetrieveValueFromFuture;
            return false;
        }
    }

    return false;
}

bool TapProtocolThread::hasStarted() const noexcept {
    return _state != CKTapThreadState::notStarted;
}

bool TapProtocolThread::hasFailed() const noexcept {
    return _state > CKTapThreadState::finished;
}

bool TapProtocolThread::hasFinished() const noexcept {
    return _state == CKTapThreadState::finished;
}

bool TapProtocolThread::isThreadActive() const noexcept {
    return hasStarted() && !hasFinished() && !hasFailed();
}

CKTapThreadState TapProtocolThread::getState() const noexcept {
    return _state;
}

CKTapInterfaceErrorCode TapProtocolThread::getRecentErrorCode() const noexcept {
    return _recentError;
}

bool TapProtocolThread::getTapProtocolException(CKTapProtoException& outException) const noexcept {
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

std::shared_ptr<tap_protocol::CKTapCard> TapProtocolThread::_lockCardForOperation() const noexcept {
    if (auto satscard = _satscard.lock()) {
        return satscard;
    } else {
        return _tapsigner.lock();
    }
}

std::unique_ptr<tap_protocol::CKTapCard> TapProtocolThread::_performHandshake(const int32_t cardType) {
    _state = CKTapThreadState::awaitingTransportRequest;
    auto transport = tap_protocol::MakeDefaultTransport([this](const tap_protocol::Bytes& bytes) {
        if (_state == CKTapThreadState::asyncActionStarting ||
            _state == CKTapThreadState::processingTransportResponse) {

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
    if (card.IsTapsigner()) {
        return tap_protocol::ToTapsigner(std::move(card));
    } else {
        return tap_protocol::ToSatscard(std::move(card));
    }
}

void TapProtocolThread::_signalTransportRequestReady(const tap_protocol::Bytes& bytes) {
    if (_state != CKTapThreadState::awaitingTransportRequest) {
        throw TransportException("Attempt to set transport request when state was \"" +
            std::to_string(_state) + '\"'
        );
    }
    _pendingTransportRequest = bytes;
    _state = CKTapThreadState::transportRequestReady;
}

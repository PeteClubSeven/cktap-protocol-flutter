#ifndef __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__
#define __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__

// STL
#include <stdexcept>

/// Thrown when the protocol thread cancels its current operation
class CancelationException final : public std::runtime_error {
public:
    explicit CancelationException(const std::string& message)
        : std::runtime_error(message) {
    }
    explicit CancelationException(const char* message)
        : std::runtime_error(message) {
    }
};

/// Thrown when the protocol thread times out
class TimeoutException final : public std::runtime_error {
public:
    explicit TimeoutException(const std::string& message)
        : std::runtime_error(message) {
    }
    explicit TimeoutException(const char* message)
        : std::runtime_error(message) {
    }
};

/// Thrown when the async thread enters an invalid state whilst processing transport requests
class TransportException final : public std::runtime_error {
public:
    explicit TransportException(const std::string& message)
        : std::runtime_error(message) {
    }
    explicit TransportException(const char* message)
        : std::runtime_error(message) {
    }
};

#endif // __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__
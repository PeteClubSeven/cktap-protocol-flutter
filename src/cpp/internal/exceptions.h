#ifndef __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__
#define __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__

// STL
#include <stdexcept>

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

#endif // __CKTAP_PROTOCOL__INTERNAL_EXCEPTIONS_H__
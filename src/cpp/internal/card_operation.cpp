#include <internal/card_operation.h>

// Project
#include <internal/utils.h>

// Used to test that indices values match to the expected response types
static constexpr CardResponseVariant response{ };

// Ensure indices of responses map to the corresponding operation
static_assert(std::is_same_v<tap_protocol::CKTapCard::WaitResponse,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::CKTapCard_Wait>(response))>>);
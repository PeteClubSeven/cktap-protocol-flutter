#ifndef __CKTAP_PROTOCOL__INTERNAL_CARD_OPERATIONS_H__
#define __CKTAP_PROTOCOL__INTERNAL_CARD_OPERATIONS_H__

// Project
#include <internal/utils.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <variant>

/// Op codes used to indicate a specific tap_protocol operation that we have to perform asynchronously. These
/// should map directly to the [CardResponseVariant]
enum class CardOperation : size_t {
    CKTapCard_Wait              = 0,
    Satscard_CertificateCheck   = 1,
    Satscard_GetSlot            = 2,
    Satscard_ListSlots          = 3,
    Satscard_New                = 4,
    Satscard_Unseal             = 5,
};

/// A type-safe collection of responses to [CardOperation] indices values
using CardResponseVariant = std::variant<
    tap_protocol::CKTapCard::WaitResponse,      // CKTapCard_Wait
    bool,                                       // Satscard_CertificateCheck
    tap_protocol::Satscard::Slot,               // Satscard_GetSlot
    std::vector<tap_protocol::Satscard::Slot>,  // Satscard_ListSlots
    tap_protocol::Satscard::Slot,               // Satscard_New
    tap_protocol::Satscard::Slot                // Satscard_Unseal
>;

/// Returns the expected type for the given op code
template <CardOperation op>
using CardResponseType = remove_cvref_t<decltype(std::get<(size_t)op>(CardResponseVariant{ }))>;

#endif //__CKTAP_PROTOCOL__INTERNAL_CARD_OPERATIONS_H__

#include <internal/card_operation.h>

// Project
#include <internal/utils.h>

using namespace tap_protocol;

// Used to test that indices values match to the expected response types
static CardResponseVariant response{ };

// Ensure indices of responses map to the corresponding operation
static_assert(std::is_same_v<CKTapCard::WaitResponse,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::CKTapCard_Wait>(response))>>);
static_assert(std::is_same_v<bool,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::Satscard_CertificateCheck>(response))>>);
static_assert(std::is_same_v<Satscard::Slot,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::Satscard_GetSlot>(response))>>);
static_assert(std::is_same_v<std::vector<Satscard::Slot>,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::Satscard_ListSlots>(response))>>);
static_assert(std::is_same_v<Satscard::Slot,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::Satscard_New>(response))>>);
static_assert(std::is_same_v<Satscard::Slot,
    remove_cvref_t<decltype(std::get<(size_t) CardOperation::Satscard_Unseal>(response))>>);
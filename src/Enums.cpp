#include <Enums.h>

// Third Party
#include <tap_protocol/cktapcard.h>

static_assert(CKTapSatscardSlotStatus::UNUSED == static_cast<int>(tap_protocol::Satscard::SlotStatus::UNUSED));
static_assert(CKTapSatscardSlotStatus::SEALED == static_cast<int>(tap_protocol::Satscard::SlotStatus::SEALED));
static_assert(CKTapSatscardSlotStatus::UNSEALED == static_cast<int>(tap_protocol::Satscard::SlotStatus::UNSEALED));
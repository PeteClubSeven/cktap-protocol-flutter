#include <internal/globals.h>

// Project
#include <internal/tap_protocol_thread.h>

std::unique_ptr<TapProtocolThread> g_protocolThread{ };
std::vector<SatscardWrapper> g_satscards{ };
std::vector<TapsignerWrapper> g_tapsigners{ };

void storeSatscardSlot(int32_t satscardHandle, tap_protocol::Satscard::Slot slot) noexcept {
    try {
        if (satscardHandle < 0) {
            return;
        }
        if (satscardHandle >= g_satscards.size()) {
            return;
        }
        const auto slotIndex = slot.index;
        if (slotIndex < 0) {
            return;
        }
        auto& slots = g_satscards[satscardHandle].slots;
        if (slotIndex >= slots.size()) {
            slots.resize(slotIndex + 1);
        }
        slots[slotIndex] = std::make_unique<tap_protocol::Satscard::Slot>(std::move(slot));
    } catch (...) { }
}
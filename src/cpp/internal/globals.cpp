#include <internal/globals.h>

// Project
#include <internal/tap_protocol_thread.h>

std::unique_ptr<TapProtocolThread> g_protocolThread{ };
std::vector<SatscardWrapper> g_satscards{ };
std::vector<TapsignerWrapper> g_tapsigners{ };

void storeSatscardSlot(size_t satscardIndex, tap_protocol::Satscard::Slot slot) noexcept {
    try {
        if (satscardIndex >= g_satscards.size()) {
            return;
        }
        const auto slotIndex = slot.index;
        if (slotIndex < 0) {
            return;
        }
        auto& slots = g_satscards[satscardIndex].slots;
        if (slots.size() < slotIndex) {
            slots.resize(slotIndex + 1);
        }
        slots[slotIndex] = std::make_unique<tap_protocol::Satscard::Slot>(std::move(slot));
    } catch (...) { }
}
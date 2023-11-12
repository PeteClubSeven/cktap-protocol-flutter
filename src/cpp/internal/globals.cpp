#include <internal/globals.h>

// Project
#include <internal/tap_protocol_thread.h>

std::unique_ptr<TapProtocolThread> g_protocolThread{ };
std::vector<SatscardWrapper> g_satscards{ };
std::vector<TapsignerWrapper> g_tapsigners{ };
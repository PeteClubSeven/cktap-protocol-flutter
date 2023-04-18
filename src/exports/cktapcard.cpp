#include "cktapcard.h"

#include <tap_protocol/cktapcard.h>
#include <tap_protocol/transport.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

std::unique_ptr<tap_protocol::Satscard> g_satscard { };
std::unique_ptr<tap_protocol::Tapsigner> g_tapsigner { };
std::atomic<bool> g_hasResponse { false };
tap_protocol::Bytes g_responseData { };

extern "C" 
{

int cktapcard_constructor(TransmitDataFunction transmitFunc)
{
    if ( transmitFunc == nullptr )
    {
        return -1;
    }

    auto transport = tap_protocol::MakeDefaultTransport([transmitFunc](const tap_protocol::Bytes& command)
    {
        // Something really bad happened
        if (g_hasResponse)
        {
            bool here = true;
        }

        // Attempt to communicate with the NFC card
        g_hasResponse = false;
        transmitFunc(command.data(), static_cast<int32_t>(command.size()));

        while (!g_hasResponse)
        {
            std::this_thread::sleep_for(20ms);
        }

        // Ensure nothing terrible happened
        if (g_responseData.empty())
        {
            return tap_protocol::Bytes { };
        }

        return g_responseData;
    });

    auto cktapcard = tap_protocol::CKTapCard { std::move(transport), true };
    if (cktapcard.IsTapsigner())
    {
        g_tapsigner = tap_protocol::ToTapsigner(std::move(cktapcard));
        if (g_tapsigner)
        {
            return 2;
        }
    }
    else
    {
        g_satscard = tap_protocol::ToSatscard(std::move(cktapcard));
        if (g_satscard)
        {
            return 1;
        }
    }

    return -1;
}

uint8_t* cktapcard_allocateResponse(const int32_t sizeInBytes)
{
    if (sizeInBytes < 0)
    {
        return nullptr;
    }

    g_responseData.resize(sizeInBytes);
    return g_responseData.data();
}

int cktapcard_finalizeResponse()
{
    if (g_hasResponse)
    {
        return -1;
    }

    g_hasResponse = true;
    return 0;
}

}
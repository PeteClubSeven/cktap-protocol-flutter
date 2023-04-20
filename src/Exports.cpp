#include <Exports.h>

// Project
#include <Internal/TapProtocolThread.h>

// Third party
#include <tap_protocol/cktapcard.h>

std::unique_ptr<TapProtocolThread> g_protocolThread { };

FFI_PLUGIN_EXPORT CKTapThreadState CKTapCard_GetThreadState()
{
    if (g_protocolThread == nullptr)
    {
        return CKTapThreadState::NotStarted;
    }

    return g_protocolThread->GetState();
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode CKTapCard_BeginInitialization()
{
    // If the thread already exists we shouldn't proceed unless it's finished
    if (g_protocolThread != nullptr && !g_protocolThread->HasFinished())
    {
        return CKTapInterfaceErrorCode::ThreadAlreadyInUse;
    }

    g_protocolThread.reset(TapProtocolThread::CreateNew());

    if (g_protocolThread == nullptr)
    {
        return CKTapInterfaceErrorCode::ThreadAllocationFailed;
    }
    if (!g_protocolThread->HasStarted())
    {
        //return CKTapInterfaceErrorCode::ThreadFailedtoStart;
    }
    if (g_protocolThread->HasFinished())
    {
        return CKTapInterfaceErrorCode::ThreadFinishedBeforeInitialTransportRequest;
    }

    return CKTapInterfaceErrorCode::Success;
}

FFI_PLUGIN_EXPORT const uint8_t* CKTapCard_GetTransportRequestPointer()
{
    if (g_protocolThread == nullptr)
    {
        return nullptr;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    
    return optionalBytes.has_value() ? optionalBytes.value()->data() : nullptr;
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_GetTransportRequestLength()
{
    if (g_protocolThread == nullptr)
    {
        return 0;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    
    return optionalBytes.has_value() ? static_cast<int32_t>(optionalBytes.value()->size()) : 0;
}

FFI_PLUGIN_EXPORT uint8_t* CKTapCard_AllocateTransportResponseBuffer(const int32_t sizeInBytes)
{
    if (g_protocolThread == nullptr)
    {
        return nullptr;
    }
    if (sizeInBytes <= 0)
    {
        return nullptr;
    }

    auto optionalBuffer = g_protocolThread->AllocateTransportResponseBuffer(static_cast<size_t>(sizeInBytes));
    return optionalBuffer.value_or(nullptr);
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode CKTapCard_FinalizeTransportResponse()
{
    if (g_protocolThread == nullptr)
    {
        return CKTapInterfaceErrorCode::ThreadNotYetStarted;
    }
    if (g_protocolThread->GetState() != CKTapThreadState::TransportRequestReady)
    {
        return CKTapInterfaceErrorCode::ThreadNotReadyForResponse;
    }

    return g_protocolThread->FinalizeTransportResponse() ?
        CKTapInterfaceErrorCode::Success :
        CKTapInterfaceErrorCode::ThreadResponseFinalizationFailed;
}

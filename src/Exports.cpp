#include <Exports.h>

// Project
#include <Internal/TapProtocolThread.h>
#include <Internal/Utils.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstdlib>

// ----------------------------------------------
// Core Bindings: 

FFI_PLUGIN_EXPORT CKTapThreadState Core_GetThreadState()
{
    if (g_protocolThread == nullptr)
    {
        return CKTapThreadState::NotStarted;
    }

    return g_protocolThread->GetState();
}

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_BeginInitialization()
{
    // If the thread already exists we shouldn't proceed unless it's finished
    if (g_protocolThread != nullptr && !g_protocolThread->HasFinished() && !g_protocolThread->HasFailed())
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

FFI_PLUGIN_EXPORT CKTapCardFinalizeOperationResponse Core_FinalizeRecentOperation()
{
    CKTapCardFinalizeOperationResponse response = 
    {
        .handle.index = -1,
        .handle.type = CKTapCardType::UnknownCard,
        .errorCode = CKTapInterfaceErrorCode::Success,
    };

    if (g_protocolThread == nullptr)
    {
        response.errorCode = CKTapInterfaceErrorCode::ThreadNotYetStarted;
        return response;
    }
    if (!g_protocolThread->HasStarted())
    {
        response.errorCode = CKTapInterfaceErrorCode::ThreadNotYetStarted;
    }
    if (!g_protocolThread->FinalizeRecentOperation())
    {
        response.errorCode = CKTapInterfaceErrorCode::ThreadOperationFinalizationFailed;
        return response;
    }
    
    response.errorCode = g_protocolThread->GetRecentErrorCode();
    if (!g_protocolThread->HasFinished() || !g_protocolThread->IsTapsigner().has_value())
    {
        return response;
    }
    
    size_t index = invalidIndex;
    if (g_protocolThread->IsTapsigner().value())
    {
        auto tapsigner = g_protocolThread->ReleaseTapsigner();
        if (!tapsigner)
        {
            response.errorCode = CKTapInterfaceErrorCode::ExpectedTapsignerButReceivedNothing;
            return response;
        }
        
        index = UpdateVectorWithTapCard(g_tapsigners, tapsigner);
        response.handle.type = CKTapCardType::Tapsigner;
    }
    else
    {
        auto satscard = g_protocolThread->ReleaseSatscard();
        if (!satscard)
        {
            response.errorCode = CKTapInterfaceErrorCode::ExpectedTapsignerButReceivedNothing;
            return response;
        }
        
        index = UpdateVectorWithTapCard(g_satscards, satscard);
        response.handle.type = CKTapCardType::Satscard;
    }

    if (index == invalidIndex)
    {
        response.errorCode = CKTapInterfaceErrorCode::InvalidHandlingOfTapCardDuringOperationFinalization;
        return response;
    }
    else
    {
        response.handle.index = static_cast<int32_t>(index);
    }
    return response;
}

FFI_PLUGIN_EXPORT const uint8_t* Core_GetTransportRequestPointer()
{
    if (g_protocolThread == nullptr)
    {
        return nullptr;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    
    return optionalBytes.has_value() ? optionalBytes.value()->data() : nullptr;
}

FFI_PLUGIN_EXPORT int32_t Core_GetTransportRequestLength()
{
    if (g_protocolThread == nullptr)
    {
        return 0;
    }

    const auto optionalBytes = g_protocolThread->GetTransportRequest();
    
    return optionalBytes.has_value() ? static_cast<int32_t>(optionalBytes.value()->size()) : 0;
}

FFI_PLUGIN_EXPORT uint8_t* Core_AllocateTransportResponseBuffer(const int32_t sizeInBytes)
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

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeTransportResponse()
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

// ----------------------------------------------
// CKTapCard:

FFI_PLUGIN_EXPORT char* CKTapCard_GetIdentCString(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, nullptr, [](const auto& card) 
    {
        return AllocateCStringFromCpp(card.GetIdent());
    });
}

FFI_PLUGIN_EXPORT char* CKTapCard_GetAppletVersionCString(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, nullptr, [](const auto& card) 
    {
        return AllocateCStringFromCpp(card.GetAppletVersion());
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_GetBirthHeight(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.GetBirthHeight();
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTestnet(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.IsTestnet() ? 1 : 0;
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_GetAuthDelay(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.GetAuthDelay();
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTampered(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.IsTampered() ? 1 : 0;
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_IsCertsChecked(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.IsCertsChecked() ? 1 : 0;
    });
}

FFI_PLUGIN_EXPORT int32_t CKTapCard_NeedSetup(const int32_t handle, const int32_t type)
{
    return GetFromTapCard(handle, type, 0, [](const auto& card) 
    {
        return card.NeedSetup() ? 1 : 0;
    });
}

// ----------------------------------------------
// Utility:
FFI_PLUGIN_EXPORT void Utility_FreeString(char* cString)
{
    if (cString != nullptr)
    {
        std::free(cString);
    }
}
#include <Internal/Utils.h>

// Project
#include <Internal/TapProtocolThread.h>

// Third party
#include <tap_protocol/cktapcard.h>

// STL
#include <cstring>

char* AllocateCStringFromCpp(const std::string& cppString)
{
    char* cString = strdup(cppString.c_str());
    return cString;
}

CKTapCardHandle ConstructTapCardHandle(const int32_t index, const int32_t type)
{
    CKTapCardHandle handle = 
    {
        .index = index,
        .type = IntToTapCardType(type),
    };

    return handle;
}

CKTapCardType IntToTapCardType(const int32_t type)
{
    switch (type)
    {
        case CKTapCardType::Satscard:
            return CKTapCardType::Satscard;
        case CKTapCardType::Tapsigner:
            return CKTapCardType::Tapsigner;
        default:
            return CKTapCardType::UnknownCard;
    }
}
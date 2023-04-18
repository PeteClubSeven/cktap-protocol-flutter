#ifndef __CKTAP_PROTOCOL__EXPORTS_CKTAPCARD_H__
#define __CKTAP_PROTOCOL__EXPORTS_CKTAPCARD_H__

#if __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <uchar.h>

/// A function pointer to handle transmitting data between the tap-protocol library and the actual card
/// The function will be called using the prepared data to transmit. The response should be written into
/// the given buffer.
/// The function should return an error code, 0 for successful.
typedef int (*TransmitDataFunction)(const uint8_t* rawData, const int32_t length);

/// @brief 
/// @param transmitFunc 
/// @return The handle of the constructed object, 1 or higher are valid values, negative values are errors.
int cktapcard_constructor(TransmitDataFunction transmitFunc);


uint8_t* cktapcard_allocateResponse(const int32_t sizeInBytes);

int cktapcard_finalizeResponse();


#if __cplusplus
}
#endif // extern "C"

#endif // __CKTAP_PROTOCOL__EXPORTS_CKTAPCARD_H__
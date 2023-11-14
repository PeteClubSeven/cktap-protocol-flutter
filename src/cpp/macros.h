#ifndef __CKTAP_PROTOCOL__MACROS_H__
#define __CKTAP_PROTOCOL__MACROS_H__

#ifdef __cplusplus
    #define FFI_FUNC_EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))
    #define FFI_TYPE_EXPORT extern "C"
#else
    #define FFI_FUNC_EXPORT __attribute__((visibility("default"))) __attribute__((used))
    #define FFI_TYPE_EXPORT
#endif

#endif // __CKTAP_PROTOCOL__MACROS_H__
#ifndef __CKTAP_PROTOCOL__INTERNAL_MACROS_H__
#define __CKTAP_PROTOCOL__INTERNAL_MACROS_H__

#include <stdexcept>
#include <variant>

#if defined(CKTAP_PLATFORM_ANDROID)
    #define CKTAP_PLATFORM_ANDROID 1
    #define CKTAP_PLATFORM_IOS 0
#elif defined(CKTAP_PLATFORM_IOS)
    #define CKTAP_PLATFORM_ANDROID 0
    #define CKTAP_PLATFORM_IOS 1
#else
    #define CKTAP_PLATFORM_ANDROID 0
    #define CKTAP_PLATFORM_IOS 0
#endif

// TODO: Figure out how to get rid of this horrible macro
// There seems to be a bug either with FFI or with the compiler where trying to catch a
// tap_protocol::TapProtoException doesn't work. This causes a full app crash and is replicable
// with the following code:
/* void triggerException() {
    tap_protocol::CKTapCard(tap_protocol::MakeDefaultTransport([](const auto& bytes) {
        return tap_protocol::Bytes{ }; // <- This triggers a TapProtoException as expected
    }));
}
extern "C" void myCrashingFunction() {
    try {
        triggerException();
    } catch (const tap_protocol::TapProtoException& e) {
        // This never catches the exception. It's completely ignored and the app will
        // hard crash with no warning.
    }
}
extern "C" void myWorkingFunction() {
    try {
        triggerException();
    } catch (const std::exception& e) {
        // This will always catch the exception! Success!
    }
}
*/
// Should catch every STL exception before static casting an std::exception (dynamic returns
// null). The list can be found at: https://en.cppreference.com/w/cpp/error/exception
#define CATCH_TAP_PROTO_EXCEPTION(name, code) \
    catch (const std::logic_error&) {} \
    catch (const std::runtime_error&) {} \
    catch (const std::bad_typeid&) {} \
    catch (const std::bad_cast&) {} \
    catch (const std::bad_optional_access&) {} \
    catch (const std::bad_weak_ptr&) {} \
    catch (const std::bad_function_call&) {} \
    catch (const std::bad_alloc&) {} \
    catch (const std::bad_exception&) {} \
    catch (const std::bad_variant_access&) {} \
    catch (const std::exception& ____exception) { \
        const auto& name = static_cast<const tap_protocol::TapProtoException&>(____exception); \
        code \
    }

#endif //__CKTAP_PROTOCOL__INTERNAL_MACROS_H__

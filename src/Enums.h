#ifndef __CKTAP_PROTOCOL__ENUMS_H__
#define __CKTAP_PROTOCOL__ENUMS_H__

#include "Macros.h"

/// @brief Represents errors that may occur when the library is used incorrectly
FFI_PLUGIN_EXPORT typedef enum CKTapInterfaceErrorCode
{
    Success,
    ThreadAlreadyInUse,
    ThreadAllocationFailed,
    ThreadFailedtoStart,
    ThreadFinishedBeforeInitialTransportRequest,
    ThreadNotYetStarted,
    ThreadNotReadyForResponse,
    ThreadResponseFinalizationFailed,
    ThreadTimeoutDuringTransport,
    ThreadEncounterTapProtocolError,
    UnknownErrorDuringInitialization,
} CKTapInterfaceErrorCode;

/// @brief Mirrors tap_protocol::TapProtoException
FFI_PLUGIN_EXPORT typedef enum CKTapInternalErrorCode
{
    INVALID_DEVICE = 100,
    UNLUCKY_NUMBER = 205,
    BAD_ARGUMENTS = 400,
    BAD_AUTH = 401,
    NEED_AUTH = 403,
    UNKNOW_COMMAND = 404,
    INVALID_COMMAND = 405,
    INVALID_STATE = 406,
    WEAK_NONCE = 417,
    BAD_CBOR = 422,
    BACKUP_FIRST = 425,
    RATE_LIMIT = 429,
    DEFAULT_ERROR = 500,
    MESSAGE_TOO_LONG = 601,
    MISSING_KEY = 602,
    ISO_SELECT_FAIL = 603,
    SW_FAIL = 604,
    INVALID_CVC_LENGTH = 605,
    PICK_KEY_PAIR_FAIL = 606,
    ECDH_FAIL = 607,
    XCVC_FAIL = 608,
    UNKNOW_PROTO_VERSION = 609,
    INVALID_PUBKEY_LENGTH = 610,
    NO_PRIVATE_KEY_PICKED = 611,
    MALFORMED_BIP32_PATH = 612,
    INVALID_HASH_LENGTH = 613,
    SIG_VERIFY_ERROR = 614,
    INVALID_DIGEST_LENGTH = 615,
    INVALID_PATH_LENGTH = 616,
    SERIALIZE_ERROR = 617,
    EXCEEDED_RETRY = 618,
    INVALID_CARD = 619,
    SIGN_ERROR = 620,
    SIG_TO_PUBKEY_FAIL = 621,
    PSBT_PARSE_ERROR = 622,
    PSBT_INVALID = 623,
    INVALID_ADDRESS_TYPE = 624,
    INVALID_BACKUP_KEY = 625,
    INVALID_PUBKEY = 626,
    INVALID_PRIVKEY = 627,
    INVALID_SLOT = 628,
} CKTapInternalErrorCode;

/// @brief The current state of the background thread which handles tap-protocol commands
FFI_PLUGIN_EXPORT typedef enum CKTapThreadState
{
    NotStarted,
    AwaitingTransportRequest,
    TransportRequestReady,
    TransportResponseReady,
    ProcessingTransportResponse,
    Finished,
    Timeout,
    TapProtocolError,
} CKTapThreadState;

#endif // __CKTAP_PROTOCOL__ENUMS_H__
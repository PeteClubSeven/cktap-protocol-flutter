#include <Enums.h>

// Third Party
#include <tap_protocol/cktapcard.h>
#include <tap_protocol/tap_protocol.h>

/// Ensure parity of Satscard slot enums
static_assert(CKTapSatscardSlotStatus::UNUSED == static_cast<int>(tap_protocol::Satscard::SlotStatus::UNUSED));
static_assert(CKTapSatscardSlotStatus::SEALED == static_cast<int>(tap_protocol::Satscard::SlotStatus::SEALED));
static_assert(CKTapSatscardSlotStatus::UNSEALED == static_cast<int>(tap_protocol::Satscard::SlotStatus::UNSEALED));

/// Ensure parity of exception error codes
static_assert(CKTapInternalErrorCode::INVALID_DEVICE == tap_protocol::TapProtoException::INVALID_DEVICE);
static_assert(CKTapInternalErrorCode::UNLUCKY_NUMBER == tap_protocol::TapProtoException::UNLUCKY_NUMBER);
static_assert(CKTapInternalErrorCode::BAD_ARGUMENTS == tap_protocol::TapProtoException::BAD_ARGUMENTS);
static_assert(CKTapInternalErrorCode::BAD_AUTH == tap_protocol::TapProtoException::BAD_AUTH);
static_assert(CKTapInternalErrorCode::NEED_AUTH == tap_protocol::TapProtoException::NEED_AUTH);
static_assert(CKTapInternalErrorCode::UNKNOW_COMMAND == tap_protocol::TapProtoException::UNKNOW_COMMAND);
static_assert(CKTapInternalErrorCode::INVALID_COMMAND == tap_protocol::TapProtoException::INVALID_COMMAND);
static_assert(CKTapInternalErrorCode::INVALID_STATE == tap_protocol::TapProtoException::INVALID_STATE);
static_assert(CKTapInternalErrorCode::WEAK_NONCE == tap_protocol::TapProtoException::WEAK_NONCE);
static_assert(CKTapInternalErrorCode::BAD_CBOR == tap_protocol::TapProtoException::BAD_CBOR);
static_assert(CKTapInternalErrorCode::BACKUP_FIRST == tap_protocol::TapProtoException::BACKUP_FIRST);
static_assert(CKTapInternalErrorCode::RATE_LIMIT == tap_protocol::TapProtoException::RATE_LIMIT);
static_assert(CKTapInternalErrorCode::DEFAULT_ERROR == tap_protocol::TapProtoException::DEFAULT_ERROR);
static_assert(CKTapInternalErrorCode::MESSAGE_TOO_LONG == tap_protocol::TapProtoException::MESSAGE_TOO_LONG);
static_assert(CKTapInternalErrorCode::MISSING_KEY == tap_protocol::TapProtoException::MISSING_KEY);
static_assert(CKTapInternalErrorCode::ISO_SELECT_FAIL == tap_protocol::TapProtoException::ISO_SELECT_FAIL);
static_assert(CKTapInternalErrorCode::SW_FAIL == tap_protocol::TapProtoException::SW_FAIL);
static_assert(CKTapInternalErrorCode::INVALID_CVC_LENGTH == tap_protocol::TapProtoException::INVALID_CVC_LENGTH);
static_assert(CKTapInternalErrorCode::PICK_KEY_PAIR_FAIL == tap_protocol::TapProtoException::PICK_KEY_PAIR_FAIL);
static_assert(CKTapInternalErrorCode::ECDH_FAIL == tap_protocol::TapProtoException::ECDH_FAIL);
static_assert(CKTapInternalErrorCode::XCVC_FAIL == tap_protocol::TapProtoException::XCVC_FAIL);
static_assert(CKTapInternalErrorCode::UNKNOW_PROTO_VERSION == tap_protocol::TapProtoException::UNKNOW_PROTO_VERSION);
static_assert(CKTapInternalErrorCode::INVALID_PUBKEY_LENGTH == tap_protocol::TapProtoException::INVALID_PUBKEY_LENGTH);
static_assert(CKTapInternalErrorCode::NO_PRIVATE_KEY_PICKED == tap_protocol::TapProtoException::NO_PRIVATE_KEY_PICKED);
static_assert(CKTapInternalErrorCode::MALFORMED_BIP32_PATH == tap_protocol::TapProtoException::MALFORMED_BIP32_PATH);
static_assert(CKTapInternalErrorCode::INVALID_HASH_LENGTH == tap_protocol::TapProtoException::INVALID_HASH_LENGTH);
static_assert(CKTapInternalErrorCode::SIG_VERIFY_ERROR == tap_protocol::TapProtoException::SIG_VERIFY_ERROR);
static_assert(CKTapInternalErrorCode::INVALID_DIGEST_LENGTH == tap_protocol::TapProtoException::INVALID_DIGEST_LENGTH);
static_assert(CKTapInternalErrorCode::INVALID_PATH_LENGTH == tap_protocol::TapProtoException::INVALID_PATH_LENGTH);
static_assert(CKTapInternalErrorCode::SERIALIZE_ERROR == tap_protocol::TapProtoException::SERIALIZE_ERROR);
static_assert(CKTapInternalErrorCode::EXCEEDED_RETRY == tap_protocol::TapProtoException::EXCEEDED_RETRY);
static_assert(CKTapInternalErrorCode::INVALID_CARD == tap_protocol::TapProtoException::INVALID_CARD);
static_assert(CKTapInternalErrorCode::SIGN_ERROR == tap_protocol::TapProtoException::SIGN_ERROR);
static_assert(CKTapInternalErrorCode::SIG_TO_PUBKEY_FAIL == tap_protocol::TapProtoException::SIG_TO_PUBKEY_FAIL);
static_assert(CKTapInternalErrorCode::PSBT_PARSE_ERROR == tap_protocol::TapProtoException::PSBT_PARSE_ERROR);
static_assert(CKTapInternalErrorCode::PSBT_INVALID == tap_protocol::TapProtoException::PSBT_INVALID);
static_assert(CKTapInternalErrorCode::INVALID_ADDRESS_TYPE == tap_protocol::TapProtoException::INVALID_ADDRESS_TYPE);
static_assert(CKTapInternalErrorCode::INVALID_BACKUP_KEY == tap_protocol::TapProtoException::INVALID_BACKUP_KEY);
static_assert(CKTapInternalErrorCode::INVALID_PUBKEY == tap_protocol::TapProtoException::INVALID_PUBKEY);
static_assert(CKTapInternalErrorCode::INVALID_PRIVKEY == tap_protocol::TapProtoException::INVALID_PRIVKEY);
static_assert(CKTapInternalErrorCode::INVALID_SLOT == tap_protocol::TapProtoException::INVALID_SLOT);

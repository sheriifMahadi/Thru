/* v1: one instruction, exactly 32 raw bytes representing a SHA-256 digest.
   The receipt event has the identical 32-byte layout. No account state is changed. */
#include <thru-sdk/c/tn_sdk.h>
#include <thru-sdk/c/tn_sdk_syscall.h>

#define RECEIPT_DIGEST_BYTES       (32UL)
#define RECEIPT_ERR_INPUT_LENGTH   (1001UL)
#define RECEIPT_ERR_EVENT_EMISSION (1002UL)

TSDK_ENTRYPOINT_FN void
start( void const * instruction_data, ulong instruction_data_sz ) {
  if( instruction_data_sz != RECEIPT_DIGEST_BYTES )
    tsdk_revert( RECEIPT_ERR_INPUT_LENGTH );
  if( tsys_emit_event( instruction_data, RECEIPT_DIGEST_BYTES ) != 0UL )
    tsdk_revert( RECEIPT_ERR_EVENT_EMISSION );
  tsdk_return( TSDK_SUCCESS );
}

#include <stddef.h>
#include <string.h>
#include <thru-sdk/c/tn_sdk.h>
#include <thru-sdk/c/tn_sdk_syscall.h>

/* Little-endian packed wire layout, shared by both operations. */
typedef struct __attribute__((packed)) {
  uchar operation;             /* 0=create, 1=increment */
  ushort account_index;
  uchar seed[32];              /* create only; zero for increment */
  uint proof_size;             /* proof bytes follow; zero for increment */
} instruction_t;
typedef struct { ulong tag; ulong value; } counter_t;
typedef struct __attribute__((packed)) {
  uchar operation;
  ulong old_value;
  ulong new_value;
} event_t;
#define COUNTER_TAG (0x315245544e554f43UL) /* COUNTER1 */
#define REQUIRE(c,e) do { if(!(c)) tsdk_revert(e); } while(0)
_Static_assert(sizeof(instruction_t)==39, "instruction layout");
_Static_assert(sizeof(counter_t)==16, "state layout");
_Static_assert(sizeof(event_t)==17, "event layout");

TSDK_ENTRYPOINT_FN void
start(void const * data, ulong size) {
  REQUIRE(size >= sizeof(instruction_t), 2001UL);
  instruction_t args;
  memcpy(&args, data, sizeof(args));
  REQUIRE(size - sizeof(args) == args.proof_size, 2001UL);
  REQUIRE(args.operation <= 1U, 2002UL);
  REQUIRE(args.account_index >= 2U && tsdk_is_account_idx_valid(args.account_index), 2003UL);
  counter_t state = {COUNTER_TAG, 0UL};
  event_t event = {args.operation, 0UL, 0UL};
  if(args.operation == 0U) {
    REQUIRE(args.proof_size > 0U, 2001UL);
    REQUIRE(!tsdk_account_exists(args.account_index), 2004UL);
    REQUIRE(tsys_account_create(args.account_index, args.seed,
      (uchar const *)data + sizeof(args), args.proof_size) == 0UL, 2005UL);
    REQUIRE(tsys_set_account_data_writable(args.account_index) == 0UL, 2006UL);
    REQUIRE(tsys_account_resize(args.account_index, sizeof(state)) == 0UL, 2007UL);
  } else {
    uchar const empty_seed[32] = {0};
    REQUIRE(args.proof_size == 0U && memcmp(args.seed, empty_seed, 32) == 0, 2001UL);
    REQUIRE(tsdk_account_exists(args.account_index) &&
      tsdk_is_account_owned_by_current_program(args.account_index), 2003UL);
    tsdk_account_meta_t const * meta = tsdk_get_account_meta(args.account_index);
    REQUIRE(meta && meta->data_sz == sizeof(state) &&
      !(meta->flags & (TSDK_ACCOUNT_FLAG_PROGRAM | TSDK_ACCOUNT_FLAG_DELETED | TSDK_ACCOUNT_FLAG_COMPRESSED)), 2003UL);
    void const * old = tsdk_get_account_data_ptr(args.account_index);
    REQUIRE(old, 2003UL);
    memcpy(&state, old, sizeof(state));
    REQUIRE(state.tag == COUNTER_TAG, 2003UL);
    REQUIRE(state.value != ~0UL, 2008UL);
    event.old_value = state.value;
    state.value++;
    event.new_value = state.value;
    REQUIRE(tsys_set_account_data_writable(args.account_index) == 0UL, 2006UL);
  }
  void * output = tsdk_get_account_data_ptr(args.account_index);
  REQUIRE(output, 2003UL);
  memcpy(output, &state, sizeof(state));
  REQUIRE(tsys_emit_event(&event, sizeof(event)) == 0UL, 2009UL);
  tsdk_return(0UL);
}

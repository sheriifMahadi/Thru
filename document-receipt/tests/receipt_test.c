/* Exercise the actual entrypoint using host substitutes for VM exit/event calls.
   This verifies C behavior, not the chain's VM or transaction runtime. */
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "../examples/document_receipt.c"

static jmp_buf exit_point;
static ulong result_code, emission_result;
static int reverted, emissions;
static unsigned char captured[32];

void tsdk_revert( ulong code ) { result_code = code; reverted = 1; longjmp(exit_point, 1); }
void tsdk_return( ulong code ) { result_code = code; reverted = 0; longjmp(exit_point, 1); }
ulong tsys_emit_event( void const * data, ulong size ) {
  assert(size == 32UL);
  emissions++;
  memcpy(captured, data, 32);
  return emission_result;
}
static void invoke( void const * data, ulong size, ulong syscall_result ) {
  emissions = 0;
  emission_result = syscall_result;
  if( setjmp(exit_point) == 0 ) start(data, size);
}
int main( void ) {
  unsigned char digest[33];
  for( unsigned int i = 0; i < sizeof(digest); i++ ) digest[i] = (unsigned char)i;
  const ulong lengths[] = {0UL, 1UL, 31UL, 33UL, 4096UL};
  for( unsigned int i = 0; i < sizeof(lengths)/sizeof(lengths[0]); i++ ) {
    /* NULL proves invalid-size branches do not dereference the payload. */
    invoke(NULL, lengths[i], 0UL);
    assert(reverted && result_code == 1001UL && emissions == 0);
  }
  invoke(digest + 1, 32UL, 0UL); /* unaligned binary input, not ASCII hex */
  assert(!reverted && result_code == 0UL && emissions == 1);
  assert(memcmp(captured, digest + 1, 32) == 0);
  FILE * event = fopen("build/receipt-event.bin", "wb");
  assert(event && fwrite(captured, 1, 32, event) == 32);
  assert(fclose(event) == 0);
  invoke(digest, 32UL, 7UL);
  assert(reverted && result_code == 1002UL && emissions == 1);
  memset(digest, 0, sizeof(digest));
  invoke(digest, 32UL, 0UL);
  assert(!reverted && emissions == 1); /* Any 32-byte digest is accepted. */
  invoke(digest, 32UL, 0UL);
  assert(!reverted && emissions == 1); /* No deduplication or hidden state. */
  puts("C entrypoint tests passed");
  return 0;
}

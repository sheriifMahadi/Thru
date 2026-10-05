#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include "../examples/simple_counter.c"
static jmp_buf exit_point;
static ulong code, syscall_failure;
static int reverted, exists, owned=1, valid=1, emissions;
static counter_t stored;
static event_t captured;
static tsdk_account_meta_t meta;
void tsdk_revert(ulong n){code=n;reverted=1;longjmp(exit_point,1);}
void tsdk_return(ulong n){code=n;reverted=0;longjmp(exit_point,1);}
int tsdk_is_account_idx_valid(ushort i){return valid && i==2;}
int tsdk_account_exists(ushort i){(void)i;return exists;}
int tsdk_is_account_owned_by_current_program(ushort i){(void)i;return owned;}
tsdk_account_meta_t const * tsdk_get_account_meta(ushort i){(void)i;return &meta;}
void * tsdk_get_account_data_ptr(ushort i){(void)i;return &stored;}
ulong tsys_account_create(ulong i,uchar const seed[32],void const *proof,ulong n){
 (void)i;(void)seed;assert(proof && n==1);if(syscall_failure==2005)return 1;exists=1;return 0;
}
ulong tsys_set_account_data_writable(ulong i){(void)i;return syscall_failure==2006;}
ulong tsys_account_resize(ulong i,ulong n){(void)i;meta.data_sz=(uint)n;return syscall_failure==2007;}
ulong tsys_emit_event(void const *data,ulong n){assert(n==17);memcpy(&captured,data,17);emissions++;return syscall_failure==2009;}
static void invoke(void const *p,ulong n){
 counter_t previous=stored;int old_exists=exists;tsdk_account_meta_t old_meta=meta;
 emissions=0;
 if(setjmp(exit_point)==0)start(p,n);
 /* Host harness models rollback; only live tests establish VM rollback. */
 if(reverted){stored=previous;exists=old_exists;meta=old_meta;}
}
int main(void){
 uchar bytes[40]={0};instruction_t args={0,2,{0},1};memcpy(bytes,&args,39);bytes[39]=7;
 invoke(bytes,40);assert(!reverted && exists && stored.value==0 && emissions==1);
 invoke(bytes,40);assert(reverted && code==2004 && stored.value==0);
 args.operation=1;args.proof_size=0;memcpy(bytes,&args,39);
 invoke(bytes,39);assert(!reverted && stored.value==1 && captured.old_value==0 && captured.new_value==1);
 invoke(bytes,39);assert(!reverted && stored.value==2 && captured.old_value==1);
 FILE *f=fopen("build/event.bin","wb");assert(f);assert(fwrite(&captured,1,17,f)==17);fclose(f);
 f=fopen("build/increment.bin","wb");assert(f);assert(fwrite(bytes,1,39,f)==39);fclose(f);
 invoke(NULL,0);assert(reverted && code==2001);
 invoke(bytes,38);assert(reverted && code==2001);
 invoke(bytes,40);assert(reverted && code==2001);
 bytes[0]=9;invoke(bytes,39);assert(reverted && code==2002);bytes[0]=1;
 valid=0;invoke(bytes,39);assert(reverted && code==2003);valid=1;
 owned=0;invoke(bytes,39);assert(reverted && code==2003);owned=1;
 meta.flags=TSDK_ACCOUNT_FLAG_PROGRAM;invoke(bytes,39);assert(reverted && code==2003);meta.flags=0;
 meta.data_sz=8;invoke(bytes,39);assert(reverted && code==2003);meta.data_sz=16;
 stored.tag=0;invoke(bytes,39);assert(reverted && code==2003);stored.tag=COUNTER_TAG;
 stored.value=~0UL;invoke(bytes,39);assert(reverted && code==2008 && stored.value==~0UL);stored.value=2;
 syscall_failure=2006;invoke(bytes,39);assert(reverted && code==2006 && stored.value==2);
 syscall_failure=2009;invoke(bytes,39);assert(reverted && code==2009 && stored.value==2);
 syscall_failure=0;bytes[3]=1;invoke(bytes,39);assert(reverted && code==2001);
 puts("Counter entrypoint tests passed");return 0;
}

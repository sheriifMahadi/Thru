# Simple counter — THRU C program

A public counter that remembers its value on-chain. Create a counter account at
zero, then increment it one at a time. Each successful operation emits the old
and new values. Anyone may increment a counter; this counts calls, not unique
people. No reset or decrement operation is provided.

This is a sibling of `document-receipt` under `/home/vanity/thru-projects`, with its
own Git repository. Document receipt demonstrates events; this program also
shows persistent state. Both can be inspected in ThruLens.

## Build and test

Requires the THRU C SDK and RISC-V toolchain already installed under `~/.thru/sdk`,
plus `cc`, Python 3, and the THRU CLI for local tests.

```bash
cd /home/vanity/thru-projects/simple-counter
make
make test
```

Deployable output: `build/thruvm/bin/simple_counter_c.bin`.
The `.elf` build is for debugging; upload the `.bin` using normal CLI validation.
The initial build uses THRU CLI 0.4.0 and GCC 15.2.0.

## How it works

- The executable program and each counter's state are separate accounts.
- State holds a `COUNTER1` type marker and an unsigned 64-bit value (16 bytes).
- Creation requires a fresh program-derived address and its chain state proof.
- Increment requires an existing, correctly sized account owned by this program,
  the expected type marker, and a value below the unsigned 64-bit maximum.
- The VM must allow writes to the state account. Pass it as a read/write account.
- Successful calls emit `CounterChanged { operation, old_value, new_value }`.
- The VM reverts state changes if a call fails, including event emission failure.

The compact C source is in `examples/simple_counter.c`. The ABI has no external
imports and describes instructions, events, and the stored account layout.

## Instruction format

Both operations share one packed, little-endian layout so clients use one encoder:

| Field | Size | Meaning |
| --- | --- | --- |
| operation | 1 byte | 0 = create; 1 = increment |
| account_index | 2 bytes | Position of the counter account in the transaction |
| seed | 32 bytes | Zero-padded UTF-8 seed for creation; all zero for increment |
| proof_size | 4 bytes | Number of proof bytes; zero for increment |
| proof | proof_size bytes | Creation proof; absent for increment |

With only one extra read/write account, its index is 2 (payer is 0, program is 1).
The helper prints hex data; it does not sign or submit anything:

```bash
python3 scripts/instruction.py increment
python3 scripts/instruction.py create --seed counter_demo_v1 --proof-hex-file proof.hex
```

The create proof must be obtained for the derived state account immediately before
submission. A captured old proof is not a reusable instruction for a fresh account.
Never create an existing counter again; this program rejects that instead of
resetting its value.

## Error codes

| Code | Meaning |
| --- | --- |
| 2001 | Invalid input length, proof length, or increment-only fields |
| 2002 | Unknown operation |
| 2003 | Invalid account index, owner, state layout, flags, or type marker |
| 2004 | Counter already exists |
| 2005 | Account creation failed |
| 2006 | Could not make account data writable |
| 2007 | Account resize failed |
| 2008 | Counter would overflow |
| 2009 | Event emission failed |

## Validation

`make test` runs the actual C entrypoint with host substitutes for VM functions.
It checks creation, increments, duplicate creation, malformed data, invalid accounts,
wrong owner/layout/tag, overflow, and syscall failures. The host harness models
rollback and does not by itself prove VM rollback. Separate live checks verify
persistent state and failed-call behavior on BetaNet.

The tests also check the Python instruction encoder and use the official CLI to
round-trip increment, dynamic creation proofs, and events through the ABI.

## GitHub

Build files and transient logs are ignored. Commit source, tests, ABI, README, and
public deployment records; keep wallet keys in your local THRU CLI configuration.
To push, create an empty GitHub repository and use its URL:

```bash
git remote add origin https://github.com/YOUR_USERNAME/simple-counter.git
git push -u origin main
```

Document receipt is a separate repository in `../document-receipt`; push it to its
own GitHub repository. Neither repository needs ThruLens running to build or call
the deployed programs.

## Deployed BetaNet demo

Deployed and verified on October 5, 2026, using the existing local `default` wallet.
The program and ABI remain upgradeable. Full public transaction references and
artifact hashes are in `deployment.betanet.json`.

- Program: `taKurxpTqC8_VCJRmkTaFalCF7eHfjb2lUYQirDyg2Nshn`
- Counter state: `tabNrmmdR-mz3cGCf4_kTWmjKhB1Iwnf62Ke_l1ueEdBYp`
- ABI: `taoV_FP4Qgcplby56IWzT8eGjY8enNbtekXk6lp4_gMiaR`
- Verified sequence: create at 0 → increment to 1 → increment to 2.
- Malformed input reverted with 2001; state remained 2 and no event was emitted.
- All three successful calls' exact event bytes and historical instruction/event
  decoding were verified using ThruLens's SDK modules, without running its servers.
- On-chain executable bytes were compared to the locally tested binary.

Read the current value without sending a transaction:

```bash
python3 scripts/read-counter.py
```

Increment the deployed demo (signs a BetaNet transaction with your local wallet):

```bash
counter_input=$(python3 scripts/instruction.py increment)
thru --url https://rpc.betanet.thru.org txn execute \
  taKurxpTqC8_VCJRmkTaFalCF7eHfjb2lUYQirDyg2Nshn \
  "$counter_input" --fee-payer default \
  --readwrite-accounts tabNrmmdR-mz3cGCf4_kTWmjKhB1Iwnf62Ke_l1ueEdBYp
python3 scripts/read-counter.py
```

For ThruLens: save the **program address**, not the state address. Use start slot
`401550` to include the initial verification calls if upstream history is still
available. This program has not been registered in the local dashboard yet;
ThruLens and its database were left stopped as requested.

# Document receipt — THRU C program

Submit exactly 32 bytes representing a SHA-256 document digest. The program emits
one `ReceiptRecorded` event containing those same bytes, then returns success.
The document stays on your machine. Anyone can submit a digest, and repeat
submissions are allowed. There is no persistent receipt account or deduplication.

A receipt records submission of a fingerprint. It does not prove authorship,
ownership, document truthfulness, or that the submitter possessed the document.
The program cannot determine whether supplied bytes were actually produced by
SHA-256. Hashing is done by the local helper, not by the on-chain program.

## Build and test

Uses the installed THRU C SDK and RISC-V toolchain. Local checks additionally need
a host C compiler (`cc`), Python 3.11+, and the THRU CLI for ABI reflection.

```bash
cd /home/vanity/thru-projects/document-receipt
make
make test
```

Outputs:

- `build/thruvm/bin/document_receipt_c.elf`: ELF build with debugging information.
- `build/thruvm/bin/document_receipt_c.bin`: raw program image (184 bytes at initial build).
- `build/thruvm/bin/document_receipt_c.s`: disassembly.

Built with the installed GCC 15.2.0 THRU toolchain and SDK build dated 2026-09-29;
ABI checks use THRU CLI 0.4.0+46a11b0f. Deployment compatibility must be verified
against the current BetaNet before uploading; never bypass image validation to
force an incompatible image through.

## Make instruction data

```bash
python3 scripts/hash-document.py --text 'My first receipt'
# Or hash the exact bytes of a local file:
python3 scripts/hash-document.py --file /path/to/document.pdf
```

The output is a 64-character hex string representing 32 bytes. The program expects
the decoded bytes, not 64 ASCII characters. The THRU transaction CLI accepts hex
instruction data and performs that conversion. No opcode or length prefix is used
in this single-operation v1 format. Text input uses UTF-8 with no added newline.

## Contract and error codes

| Input / condition | Runtime outcome | Program code | Event |
| --- | --- | --- | --- |
| Exactly 32 bytes; event accepted | Success | 0 | Exact submitted bytes |
| Any other length | Revert | 1001 | No emission attempted |
| Event syscall returns nonzero | Revert | 1002 | No successful receipt promised |

All-zero digests are accepted, because this program validates layout, not a
particular document. Failure 1002 is simulated locally; no claim is made that
we have reproduced it on BetaNet.

`receipt.abi.yaml` describes the `SubmitReceipt` instruction and `ReceiptRecorded`
event as a packed array named `document_sha256`. It has no external imports.
This ABI must be published for the deployed program before sending dashboard demo
transactions. The dashboard requires the schema's historical slot to precede the
execution slot; publishing it afterward will not retroactively decode old calls.

## Tests and current status

`make test` compiles the actual C entrypoint for the host with substitutes for VM
exit and event syscalls. It verifies invalid lengths (including empty, short, and
oversized inputs), exact binary event data, unaligned input, error propagation,
zero bytes, and repeated calls. It also checks the SHA-256 helper against a known
vector and round-trips the emitted fixture through both ABI roots with the CLI.
These are local tests, not full VM or network integration tests.

Implemented and locally tested; **not deployed**, no signing or chain writes have
been performed. No wallet or document contents are stored in this repository.

## Next: BetaNet deployment and dashboard test

Needed: the alias of an existing locally configured BetaNet signing wallet and
enough BetaNet balance for program/ABI creation and test transactions. Private
keys stay in the local CLI configuration and must not be pasted into chat.

Deployment sequence:

1. Check current BetaNet/toolchain compatibility, public wallet address and balance.
2. Deploy this program with a new managed-program seed (suggested `document_receipt_v1`).
3. Publish its matching program ABI and wait for a later finalized slot.
4. Save the deployed program in the dashboard, starting before the test calls.
5. Submit a valid digest and an invalid-length payload to exercise both outcomes.
6. Verify the receipt event, instruction decoding, stored history, and error 1001.

The dashboard and original hello-world program remain separate sibling projects.

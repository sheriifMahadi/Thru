# THRU programs

Two small C programs deployed on THRU BetaNet, with source, ABIs, local tests,
and public deployment records in one repository.

| Program | Purpose | Instructions |
| --- | --- | --- |
| [Document receipt](document-receipt/) | Emit a receipt for a submitted document hash | [README](document-receipt/README.md) |
| [Simple counter](simple-counter/) | Persist a public counter and emit value changes | [README](simple-counter/README.md) |

## Build and test

Requires the THRU C SDK and RISC-V toolchain under `~/.thru/sdk`, the THRU CLI,
a host C compiler, and Python 3.11 or newer.

```bash
cd /home/vanity/thru-projects/thru-programs
make -C document-receipt
make -C document-receipt test
make -C simple-counter
make -C simple-counter test
```

Tests are local and do not change the deployed programs. Each program's
`deployment.betanet.json` records its existing BetaNet addresses and verification
transactions. Moving the source into this repository does not redeploy anything.

Read the deployed counter without signing a transaction:

```bash
python3 simple-counter/scripts/read-counter.py
```

## Publish to GitHub

Create one empty GitHub repository, then run these commands from this folder,
replacing `YOUR_USERNAME` with your account name:

```bash
git remote add origin https://github.com/YOUR_USERNAME/thru-programs.git
git push -u origin main
```

Both programs are ordinary subdirectories, not nested repositories or submodules.
Their original Git histories are retained through subtree imports. Use this root
for future commits and pushes. The previous standalone project folders are left
in place as backups; changes made there will not automatically appear here.

Build output, temporary logs, and local environment files are ignored. Wallet
keys stay in the local THRU CLI configuration and must not be committed.

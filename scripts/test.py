#!/usr/bin/env python3
"""Local checks only. No keys, signing, deployment, or chain writes."""
import json
from pathlib import Path
import subprocess
import sys

sdk = Path(sys.argv[1])
Path("build").mkdir(exist_ok=True)
subprocess.run(["cc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-I", str(sdk / "include"), "tests/receipt_test.c", "-o", "build/receipt-test"], check=True)
subprocess.run(["build/receipt-test"], check=True)
result = subprocess.check_output([sys.executable, "scripts/hash-document.py", "--text", "abc"], text=True).strip()
assert result == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
Path("build/sample.txt").write_bytes(b"abc")
assert subprocess.check_output([sys.executable, "scripts/hash-document.py", "--file", "build/sample.txt"], text=True).strip() == result
subprocess.run(["thru", "--quiet", "abi", "analyze", "-f", "receipt.abi.yaml"], check=True)
for root in ("SubmitReceipt", "ReceiptRecorded"):
    output = subprocess.check_output(["thru", "--quiet", "abi", "reflect", "-f", "receipt.abi.yaml", "-t", root, "-d", "build/receipt-event.bin", "--values-only"], text=True)
    decoded = json.loads(output)
    assert decoded["document_sha256"] == [{"type": "u8", "value": n} for n in range(1, 33)], decoded
print("Hash helper and instruction/event ABI round-trip checks passed")

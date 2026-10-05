#!/usr/bin/env python3
"""Print hex instruction data; no signing or network access."""
import argparse
import struct
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('operation', choices=['create','increment'])
p.add_argument('--account-index',type=int,default=2)
p.add_argument('--seed',default='counter_demo_v1')
p.add_argument('--proof-hex-file', help='File containing only a hex-encoded creation proof')
a=p.parse_args()
if not 2 <= a.account_index <= 65535: p.error('Account index must be between 2 and 65535')
seed=bytes(32)
proof=b''
if a.operation=='create':
    seed=a.seed.encode('utf-8')
    if not 1 <= len(seed) <= 32: p.error('Seed must be 1–32 UTF-8 bytes')
    seed=seed.ljust(32,b'\0')
    if not a.proof_hex_file: p.error('Create requires --proof-hex-file')
    try: proof=bytes.fromhex(Path(a.proof_hex_file).read_text().strip())
    except (OSError,ValueError) as e: p.error(str(e))
    if not proof or len(proof)>65535-39: p.error('Proof is empty or too large')
elif a.proof_hex_file: p.error('Increment does not use a proof')
print((struct.pack('<BH32sI',int(a.operation=='increment'),a.account_index,seed,len(proof))+proof).hex())

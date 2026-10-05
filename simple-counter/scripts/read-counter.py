#!/usr/bin/env python3
"""Read the demo counter's current on-chain value. Does not sign or write."""
import base64
import json
from pathlib import Path
import struct
import subprocess

root=Path(__file__).resolve().parent.parent
deployment=json.loads((root/'deployment.betanet.json').read_text())
response=json.loads(subprocess.check_output([
    'thru','--quiet','--json','--url',deployment['rpcUrl'],
    'getaccountinfo',deployment['counterAccount']],text=True))['account_info']
if response['owner']!=deployment['program'] or response.get('isDeleted'):
    raise SystemExit('Unexpected counter account owner or deleted account')
data=base64.b64decode(response['data'],validate=True)
if len(data)!=16 or data[:8]!=b'COUNTER1':
    raise SystemExit('Unexpected counter state layout')
print(json.dumps({'account':deployment['counterAccount'],
    'value':str(struct.unpack('<Q',data[8:])[0]),'observedSlot':response['slot']},indent=2))

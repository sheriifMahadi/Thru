import json,subprocess,sys
from pathlib import Path
Path('build').mkdir(exist_ok=True)
subprocess.run(['cc','-std=c17','-Wall','-Wextra','-Werror','-I',str(Path(sys.argv[1])/'include'),'tests/counter_test.c','-o','build/counter-test'],check=True)
subprocess.run(['build/counter-test'],check=True)
subprocess.run(['thru','--quiet','abi','analyze','-f','counter.abi.yaml'],check=True)
def unwrap(value):
 if isinstance(value,dict):
  if set(value)=={'type','value'}: return unwrap(value['value'])
  return {k:unwrap(v) for k,v in value.items()}
 if isinstance(value,list): return [unwrap(v) for v in value]
 return value
for name,file in [('CounterInstruction','increment.bin'),('CounterChanged','event.bin')]:
 result=json.loads(subprocess.check_output(['thru','--quiet','abi','reflect','-f','counter.abi.yaml','-t',name,'-d','build/'+file,'--values-only'],text=True))
 result=unwrap(result)
 assert result['operation']==1,result
 if name=='CounterChanged': assert result['old_value']==1 and result['new_value']==2,result
 else: assert result['account_index']==2 and result['proof_size']==0,result
out=subprocess.check_output([sys.executable,'scripts/instruction.py','increment'],text=True).strip()
assert bytes.fromhex(out)==Path('build/increment.bin').read_bytes()
print('Instruction helper and ABI round-trip tests passed')

Path('build/proof.hex').write_text('010203')
create_hex=subprocess.check_output([sys.executable,'scripts/instruction.py','create','--proof-hex-file','build/proof.hex'],text=True).strip()
Path('build/create.bin').write_bytes(bytes.fromhex(create_hex))
r=unwrap(json.loads(subprocess.check_output(['thru','--quiet','abi','reflect','-f','counter.abi.yaml','-t','CounterInstruction','-d','build/create.bin','--values-only'],text=True)))
assert r['operation']==0 and r['proof_size']==3 and r['proof']==[1,2,3],r
print('Dynamic creation proof ABI check passed')

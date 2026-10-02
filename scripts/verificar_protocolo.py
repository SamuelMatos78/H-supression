#!/usr/bin/env python3
"""Regenerate fixtures and full schedule without replaying the full tournament."""
import hashlib,json,random,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'codigo/tools'))
from futures import fixture,policies

def main():
 run=ROOT/'dados/ultimo_experimento';binary=ROOT/'build/agents/hs_arena'
 protocol=json.loads((ROOT/'codigo/scenarios/futures_protocol.json').read_text())
 assert protocol==json.loads((run/'protocol.json').read_text())
 env=random.Random(810271);planner=random.Random(971420);jobs=[];blocks=[];inputs=0
 for family in protocol['families']:
  for block in range(protocol['paired_seed_blocks']):
   name=f'inputs/{family}_{block}.ag';content=fixture(binary,family,8,10,block).encode()
   assert content==(run/name).read_bytes(),name;inputs+=1
   base={'family':family,'block':block,'world_seed':env.getrandbits(64),'policy_seed':planner.getrandbits(64),'input':name,'input_sha256':hashlib.sha256(content).hexdigest()};blocks.append(base)
   for regime in protocol['regimes']:
    for opponent in policies(protocol,family)[1:]:
     for role in [0,1]:jobs.append(base|{'regime':regime,'pair_a':'hs','pair_b':opponent,'role':role,'section':'futures'})
 random.Random(82147).shuffle(jobs)
 assert blocks==json.loads((run/'blocks.json').read_text())
 assert jobs==json.loads((run/'schedule.json').read_text())
 for family in protocol['families']:
  for n in protocol['scaling']['items']:
   for rounds in protocol['scaling']['rounds']:
    assert fixture(binary,family,n,rounds,1).encode()==(run/'inputs'/f'scale_{family}_{n}_{rounds}.ag').read_bytes();inputs+=1
 for family in ['future_renewal','future_deterministic']:
  assert fixture(binary,family,2,2,1).encode()==(run/'inputs'/f'oracle_{family}.ag').read_bytes();inputs+=1
 # Same measured C/H/tests and same statistical routines; final futures adds one textual caveat only.
 provenance=json.loads((ROOT/'dados/proveniencia_copia.json').read_text())
 for r in provenance['files']:
  assert hashlib.sha256((ROOT/r['entrega']).read_bytes()).hexdigest()==r['sha256'],r['entrega']
 print(json.dumps({'fixtures_identical':inputs,'blocks_identical':len(blocks),'scheduled_matches_identical':len(jobs),'copied_file_hashes_valid':len(provenance['files'])},indent=2))
if __name__=='__main__':main()

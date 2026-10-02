#!/usr/bin/env python3
"""Six preselected matches: block zero, work1024, HS vs MCTS, both roles."""
import argparse,gzip,json,subprocess,time,platform,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 run=ROOT/'dados/ultimo_experimento';binary=ROOT/'build/agents/hs_arena'
 schedule=json.loads((run/'schedule.json').read_text())
 selected=[(i,j) for i,j in enumerate(schedule) if j['block']==0 and j['regime']=='work1024' and j['pair_b']=='mcts']
 assert len(selected)==6
 historical={}
 with gzip.open(run/'decisions.jsonl.gz','rt') as f:
  for line in f:
   r=json.loads(line)
   if r['game_id'] in {i for i,j in selected}:historical[r['game_id'],r['ply']]=r
 t=time.monotonic();count=0
 with (out/'games.jsonl').open('w') as games,(out/'decisions.jsonl').open('w') as decisions:
  for ident,j in selected:
   left,right=('hs',j['pair_b']) if j['role']==0 else (j['pair_b'],'hs')
   cmd=[str(binary),'match',str(run/j['input']),left,right,str(j['world_seed']),'1024','0','20','4096',str(j['policy_seed'])]
   rows=[json.loads(s) for s in subprocess.check_output(cmd,text=True,timeout=120).splitlines()]
   assert rows[-1]['invalid_actions']==[0,0]
   games.write(json.dumps(j|rows[-1])+'\n')
   for r in rows[:-1]:
    assert r['action']==historical[ident,r['ply']]['action'], 'Action differs; inspect compiler/platform before interpreting'
    assert r['metrics']['fully_evaluated']<=1024
    assert r['policy']!='hs' or r['metrics']['future_states']==0
    decisions.write(json.dumps({'historical_game_id':ident}|r)+'\n');count+=1
   print(j['family'], 'role',j['role'],'OK',flush=True)
 result={'matches':6,'decisions':count,'actions_match_historical':True,'elapsed_seconds':time.monotonic()-t,'platform':platform.platform(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'selection':'block=0, regime=work1024, opponent=mcts, both roles; verification subset, not new inference'}
 (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if __name__=='__main__':main()

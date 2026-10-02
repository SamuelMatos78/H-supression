#!/usr/bin/env python3
"""Check the numeric claims used in the narrative against the audited tables."""
import csv,json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def rows(name):
 with (ROOT/'dados/derivados'/f'{name}.csv').open() as f:return list(csv.DictReader(f))
def pt(x,d=0):return f'{x:,.{d}f}'.replace(',','@').replace('.',',').replace('@','.')
def main():
 report=(ROOT/'RELATORIO.md').read_text();method=(ROOT/'METODOLOGIA.md').read_text();facts=[]
 def check(label,value,d=0,text=report):
  token=pt(value,d);assert token in text,(label,token);facts.append({'claim':label,'value':value,'text':token})
 pairs=rows('matchups')
 for fam,expected in [('future_deterministic',[32,32,32]),('future_renewal',[30,29,32]),('future_shared',[31,31,32])]:
  for regime,wins in zip(['work1024','work4096','time5ms'],expected):
   r=next(r for r in pairs if r['family']==fam and r['regime']==regime and r['opponent']=='mcts');assert int(r['wins'])==wins and int(r['draws'])==0
 for r in pairs:
  if r['opponent']!='mcts':assert (int(r['wins']),int(r['draws']),int(r['losses']))==((0,32,0) if r['family']=='future_deterministic' else (16,0,16));assert float(r['mean_margin'])==0
 scale=[r for r in rows('scaling') if r['items']=='8' and r['rounds']=='10']
 for fam,policies in [('future_renewal',['hs','expectimax','mcts','renewal_exact']),('future_shared',['hs','greedy','expectimax','mcts'])]:
  for policy in policies:
   r=next(r for r in scale if r['family']==fam and r['policy']==policy);check(f'{fam}/{policy}/fixed median ms',float(r['median_us'])/1000,6)
 for r in rows('uncapped'):
  if (r['family']=='future_shared' and r['policy'] in ['hs','hs_no_filter','hs_no_aggregation']) or (r['family']=='future_renewal' and r['policy'] in ['hs','hs_no_aggregation']):check(r['family']+'/'+r['policy']+'/uncapped median ms',float(r['median_us'])/1000,6)
 r=next(r for r in rows('uncapped') if r['family']=='future_shared' and r['policy']=='hs')
 for field in ['mean_generated','mean_invalid_candidates','mean_eliminated_before','mean_fully_evaluated']:check('shared HS '+field,float(r[field]))
 r=next(r for r in rows('uncapped') if r['family']=='future_shared' and r['policy']=='lex_simple');check('shared lex heap',float(r['max_owned_heap_bytes']))
 a=json.loads((ROOT/'dados/derivados/auditoria.json').read_text())
 for key in ['matches','decisions','scaling_decisions','uncapped_decisions']:check(key,a[key])
 for key in ['time_overruns','time_overruns_without_flag','process_rss_min_kib','process_rss_max_kib']:check(key,a[key],text=method)
 check('time max',a['max_time_ms'],6,text=method);check('time excess',a['max_time_ms']-5,6,text=method)
 check('C measured subtotal',a['sum_measured_policy_seconds'],2,text=(ROOT/'README.md').read_text())
 # Every point actually plotted in the horizon panels must match its CSV row.
 for panel in json.loads((ROOT/'graficos/06_pontos.json').read_text()):
  i=panel['panel'];fam=['future_deterministic','future_renewal','future_shared'][i//3]
  if i%3==1:
   for line in panel['series']:
    for h,y in zip(line['x'],line['median_ms']):
     r=next(r for r in rows('scaling') if r['family']==fam and r['policy']==line['policy'] and r['items']=='8' and int(r['rounds'])*2==h)
     assert math.isclose(y,float(r['median_us'])/1000,rel_tol=1e-12)
 result={'narrative_numeric_claims_checked':len(facts),'all_competitive_cells_checked':len(pairs),'horizon_plot_points_match_csv':True,'claims':facts}
 (ROOT/'dados/validacao_entrega/conferencia_relatorio.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n')
 print('Verified narrative claims:',len(facts),'; competitive cells:',len(pairs),'; plotted horizon points: OK')
if __name__=='__main__':main()

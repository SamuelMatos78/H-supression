#!/usr/bin/env python3
"""Recompute all published numeric tables; audit limits without inventing causes."""
import csv,json,sys,statistics,hashlib
from collections import defaultdict
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'codigo/tools'))
from report import read,summarize_games,summarize_decisions,write_csv,quantile,table
RUN=ROOT/'dados/ultimo_experimento';OUT=ROOT/'dados/derivados'
def main():
 OUT.mkdir(exist_ok=True)
 # Read every row of all four metric streams and all games.
 games=list(read(RUN/'games.jsonl'));decisions=list(read(RUN/'decisions.jsonl.gz'))
 scaling=list(read(RUN/'scaling.jsonl'));uncapped=list(read(RUN/'uncapped.jsonl'));oracles=list(read(RUN/'oracle_checks.jsonl'))
 derived={'matchups':summarize_games(games),'decisions':summarize_decisions(decisions,['family','regime','policy']),'scaling':summarize_decisions(scaling,['family','items','rounds','regime','policy']),'uncapped':summarize_decisions(uncapped,['family','policy'])}
 for name,rows in derived.items():
  path=OUT/f'{name}.csv';write_csv(path,rows)
  assert path.read_bytes()==(RUN/'analysis'/f'{name}.csv').read_bytes(),name+' differs from archive'
 protocol=json.loads((RUN/'protocol.json').read_text());groups=defaultdict(list)
 for r in decisions:groups[r['family'],r['regime'],r['policy']].append(r)
 audits=[]
 for (family,regime,policy),rows in sorted(groups.items()):
  cap,ms,depth,it=protocol['regimes'][regime];m=[r['metrics'] for r in rows];t=[x['elapsed_ns']/1e6 for x in m]
  truncated=[x for x in m if x['truncated']]
  # Evidence-compatible classifications, NOT retrospectively logged stop reasons.
  eval_consistent=sum(x['fully_evaluated']==cap for x in truncated) if cap else 0
  time_consistent=sum(x['elapsed_ns']>=ms*1e6 for x in truncated) if ms else 0
  tree=policy in ['expectimax','alphabeta','oracle']
  audits.append({'family':family,'regime':regime,'policy':policy,'decisions':len(rows),'truncated':len(truncated),'fraction_truncated':len(truncated)/len(rows),'eval_cap':cap,'time_cap_ms':ms,'configured_depth':depth,'iteration_cap':it,'max_evaluations':max(x['fully_evaluated'] for x in m),'evaluation_overruns':sum(x['fully_evaluated']>cap for x in m) if cap else 0,'truncated_consistent_eval_cap':eval_consistent,'truncated_consistent_time_cap':time_consistent,'unresolved_truncation':len(truncated)-eval_consistent-time_consistent,'median_ms':statistics.median(t),'p95_ms':quantile(t,.95),'max_ms':max(t),'over_time_count':sum(v>ms for v in t) if ms else 0,'over_time_without_truncated':sum(x['elapsed_ns']>ms*1e6 and not x['truncated'] for x in m) if ms else 0,'max_time_overrun_ms':max(0,max(t)-ms) if ms else 0,'max_time_ratio':max(t)/ms if ms else 0,'min_completed_depth':min(x['completed_depth'] for x in m),'max_completed_depth':max(x['completed_depth'] for x in m),'max_visited_depth':max(x['temporal_depth'] for x in m),'truncated_without_complete_layer':sum(x['truncated'] and x['completed_depth']==0 for x in m) if tree else 0,'full_horizon_certified':sum(r['search_complete'] for r in rows) if tree else 0,'normal_mcts_iteration_cap':sum(not x['truncated'] and x['rollouts']==it for x in m) if policy=='mcts' else 0,'not_truncated':sum(not x['truncated'] for x in m)})
 write_csv(OUT/'limites.csv',audits)
 summary={'matches':len(games),'decisions':len(decisions),'scaling_decisions':len(scaling),'uncapped_decisions':len(uncapped),'oracle_decisions':len(oracles),'invalid_actions':sum(sum(g['invalid_actions']) for g in games),'hs_future_states':sum(r['metrics']['future_states'] for r in decisions if r['policy']=='hs'),'evaluation_overruns':sum(r['evaluation_overruns'] for r in audits),'unresolved_truncations':sum(r['unresolved_truncation'] for r in audits),'time_overruns':sum(r['over_time_count'] for r in audits),'time_overruns_without_flag':sum(r['over_time_without_truncated'] for r in audits),'max_time_ms':max(r['max_ms'] for r in audits if r['regime']=='time5ms'),'sum_measured_policy_seconds':sum(r['metrics']['elapsed_ns'] for rows in [decisions,scaling,uncapped,oracles] for r in rows)/1e9,'process_rss_min_kib':min(g['process_peak_rss_kib'] for g in games),'process_rss_max_kib':max(g['process_peak_rss_kib'] for g in games),'tables_byte_equal_to_archive':True}
 assert summary['matches']==2016 and summary['decisions']==40320
 assert summary['invalid_actions']==summary['hs_future_states']==summary['evaluation_overruns']==0
 (OUT/'auditoria.json').write_text(json.dumps(summary,indent=2)+'\n')
 lines=['# Resultados recalculados dos registros brutos','Gerado por `scripts/auditar.py`. Sem edição manual. Todas as quatro tabelas CSV foram comparadas byte a byte com as tabelas arquivadas.',f"{len(games)} partidas; {len(decisions)} decisões; {len(scaling)} medições de escala; {len(uncapped)} medições locais sem limites de tempo/avaliações; {len(oracles)} verificações por oráculo."]
 for regime in protocol['regimes']:
  lines+=['## '+regime,table([r for r in derived['matchups'] if r['regime']==regime],[('Cenário',lambda r:r['family']),('Rival',lambda r:r['opponent']),('V/E/D HS',lambda r:f"{r['wins']}/{r['draws']}/{r['losses']}"),('Pontos [IC exploratório 95%]',lambda r:f"{r['points']:.3f} [{r['points_ci_low']:.3f}, {r['points_ci_high']:.3f}]"),('Margem média',lambda r:f"{r['mean_margin']:.2f}")])]
 lines+=['## Observação inicial: 8 itens, 10 rodadas/jogador, work4096',table([r for r in derived['scaling'] if r['items']==8 and r['rounds']==10],[('Cenário',lambda r:r['family']),('Política',lambda r:r['policy']),('Mediana ms',lambda r:f"{r['median_us']/1000:.6f}"),('Avaliações médias',lambda r:f"{r['mean_fully_evaluated']:.1f}"),('Sucessores médios',lambda r:f"{r['mean_future_states']:.1f}"),('Heap máximo B',lambda r:r['max_owned_heap_bytes']),('Nível concluído',lambda r:r['max_completed_depth'])])]
 lines+=['## Prazo cooperativo de 5 ms: consumo e extrapolação',table([r for r in audits if r['regime']=='time5ms'],[('Cenário',lambda r:r['family']),('Política',lambda r:r['policy']),('N',lambda r:r['decisions']),('Truncadas',lambda r:r['truncated']),('Acima de 5 ms',lambda r:r['over_time_count']),('Mediana ms',lambda r:f"{r['median_ms']:.4f}"),('Máximo ms',lambda r:f"{r['max_ms']:.4f}"),('Excesso máximo ms',lambda r:f"{r['max_time_overrun_ms']:.4f}"),('Razão máx./limite',lambda r:f"{r['max_time_ratio']:.3f}")])]
 lines+=['## Interrupção de aprofundamento e cap de avaliações',table([r for r in audits if r['policy'] in ['expectimax','alphabeta','mcts']],[('Cenário',lambda r:r['family']),('Regime',lambda r:r['regime']),('Política',lambda r:r['policy']),('Truncadas/N',lambda r:f"{r['truncated']}/{r['decisions']}"),('Compatíveis com cap eval/tempo',lambda r:f"{r['truncated_consistent_eval_cap']}/{r['truncated_consistent_time_cap']}"),('Prof. concluída min–max',lambda r:f"{r['min_completed_depth']}–{r['max_completed_depth']}"),('Prof. visitada máx.',lambda r:r['max_visited_depth']),('Truncadas sem camada completa (árvores)',lambda r:r['truncated_without_complete_layer']),('MCTS terminou iterações',lambda r:r['normal_mcts_iteration_cap'])])]
 lines+=['## Leitura dos diagnósticos','Compatibilidade com o único cap ativo é uma inferência apoiada no código e contadores; a causa individual exata não foi registrada. Zero casos não resolvidos por esses critérios não transforma inferências em eventos instrumentados. MCTS não certifica profundidade completa. Memória esgotada, estouro de pilha e falha de processo não foram observados nestes registros completos. Ver METODOLOGIA.md.', '```json\n'+json.dumps(summary,indent=2)+'\n```']
 (OUT/'RESULTADOS.md').write_text('\n\n'.join(lines)+'\n')
 print(json.dumps(summary,indent=2))
if __name__=='__main__':main()

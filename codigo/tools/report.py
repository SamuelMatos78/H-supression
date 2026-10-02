#!/usr/bin/env python3
"""Paired-block analysis. Retains every comparison, including losses and ties."""
import argparse, csv, gzip, hashlib, json, random, statistics
from collections import defaultdict
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def read(path):
    opener=gzip.open if path.suffix=='.gz' else open
    with opener(path,'rt') as f:
        for line in f:yield json.loads(line)
def mean(x):return statistics.fmean(x)
def quantile(x,p):
    x=sorted(x);at=(len(x)-1)*p;i=int(at)
    return x[i]+(x[min(i+1,len(x)-1)]-x[i])*(at-i)
def intervals(blocks,key):
    """Percentile bootstrap of entire role pairs; never resample individual games."""
    values=list(blocks.values());n=len(values)
    rng=random.Random(int.from_bytes(hashlib.sha256(key.encode()).digest()[:8],'big'))
    means=[[mean(v[j] for v in pair) for j in range(3)] for pair in values]
    dist=[[],[],[]]
    for _ in range(4000):
        total=[0.,0.,0.]
        for _ in range(n):
            v=means[rng.randrange(n)]
            for j in range(3):total[j]+=v[j]
        for j in range(3):dist[j].append(total[j]/n)
    return [(quantile(d,.025),quantile(d,.975)) for d in dist]
def write_csv(path,rows):
    with path.open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
def summarize_games(games):
    groups=defaultdict(list)
    for g in games:groups[tuple(g[k] for k in ['section','family','regime','pair_a','pair_b'])].append(g)
    result=[]
    for key,rows in sorted(groups.items()):
        blocks=defaultdict(list);wins=draws=losses=0;utility=[];opponent=[];reward=[];cash=[];energy=[]
        for g in rows:
            # Self-play diagnostics are oriented to seat 0, never counted twice.
            seat=0 if g['policy0']==key[3] else 1
            w=g['winner'];win=int(w==seat);draw=int(w==-1)
            wins+=win;draws+=draw;losses+=int(w!=-1 and w!=seat)
            u=g['utility'][seat];v=g['utility'][1-seat]
            blocks[g['block']].append((win,win+.5*draw,u-v))
            utility.append(u);opponent.append(v);reward.append(g['reward'][seat]);cash.append(g['cash'][seat]);energy.append(g['energy'][seat])
        if any(len(v)!=2 for v in blocks.values()):raise ValueError('Incomplete role pair')
        ci=intervals(blocks,'|'.join(key));n=len(rows)
        result.append(dict(zip(['section','family','regime','policy','opponent'],key))|{
            'games':n,'blocks':len(blocks),'wins':wins,'draws':draws,'losses':losses,'win_fraction':wins/n,
            'win_ci_low':ci[0][0],'win_ci_high':ci[0][1],'points':(wins+.5*draws)/n,
            'points_ci_low':ci[1][0],'points_ci_high':ci[1][1],'mean_utility':mean(utility),'mean_opponent_utility':mean(opponent),
            'mean_margin':mean(a-b for a,b in zip(utility,opponent)),'margin_ci_low':ci[2][0],'margin_ci_high':ci[2][1],
            'mean_reward':mean(reward),'mean_cash':mean(cash),'mean_energy':mean(energy)})
    return result
def summarize_decisions(rows,keys):
    groups=defaultdict(list)
    for row in rows:groups[tuple(row[k] for k in keys)].append(row)
    result=[]
    for key,values in sorted(groups.items()):
        metrics=[v['metrics'] for v in values];times=[m['elapsed_ns']/1000 for m in metrics]
        med=statistics.median(times)
        summary=dict(zip(keys,key))|{'decisions':len(values),'median_us':med,'mad_us':statistics.median(abs(x-med) for x in times),
            'p95_us':quantile(times,.95),'max_us':max(times),'fraction_truncated':mean(m['truncated'] for m in metrics),
            'max_owned_heap_bytes':max(m['peak_owned_heap_bytes'] for m in metrics),
            'distinct_actions':len({json.dumps(v['action'],sort_keys=True) for v in values}),
            'max_temporal_depth':max(m['temporal_depth'] for m in metrics),'max_completed_depth':max(m['completed_depth'] for m in metrics)}
        for field in ['generated','invalid_candidates','fully_evaluated','eliminated_before','defeated_after','future_states','probability_terms','outcomes_enumerated','rollouts','cutoffs']:
            summary['mean_'+field]=mean(m[field] for m in metrics)
        result.append(summary)
    return result
def table(rows,columns):
    text=['| '+' | '.join(label for label,fn in columns)+' |','| '+' | '.join('---' for _ in columns)+' |']
    for row in rows:text.append('| '+' | '.join(str(fn(row)) for label,fn in columns)+' |')
    return '\n'.join(text)+'\n'
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--run',type=Path,required=True);a=ap.parse_args();run=a.run.resolve()
    out=run/'analysis';out.mkdir(exist_ok=True)
    games=list(read(run/'games.jsonl'));summary=summarize_games(games)
    block_count=json.loads((run/'environment.json').read_text())['seed_blocks']
    write_csv(out/'matchups.csv',summary)
    computational=summarize_decisions(read(run/'decisions.jsonl.gz'),['section','family','regime','policy'])
    fixed=summarize_decisions(read(run/'decisions_fixed.jsonl'),['family','regime','policy'])
    boundary=summarize_decisions(read(run/'boundary.jsonl'),['family','size','policy'])
    for name,rows in [('decisions',computational),('fixed_observations',fixed),('boundary',boundary)]:write_csv(out/f'{name}.csv',rows)
    hs=[r for r in summary if r['policy']=='hs' and r['opponent']!='hs']
    index={'definition':'Exploratory 95% paired-block interval for HS tie-adjusted points; unadjusted for multiple comparisons. Not a confirmatory superiority claim.',
        'wins':[r for r in hs if r['points_ci_low']>.5],
        'losses':[r for r in hs if r['points_ci_high']<.5],
        'inconclusive_or_balanced':[r for r in hs if r['points_ci_low']<=.5<=r['points_ci_high']]}
    (out/'all_outcomes.json').write_text(json.dumps(index,indent=2)+'\n')
    check={'games':len(games),'decisions':sum(r['decisions'] for r in computational),
        'invalid_actions':sum(sum(g['invalid_actions']) for g in games),
        'hs_future_states':sum(r['mean_future_states']*r['decisions'] for r in computational if r['policy'].startswith('hs')),
        'process_rss_kib_min':min(g['process_peak_rss_kib'] for g in games),'process_rss_kib_max':max(g['process_peak_rss_kib'] for g in games),
        'report_source_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
    (out/'checks.json').write_text(json.dumps(check,indent=2)+'\n')
    for lang in ['en','pt-BR']:
        pt=lang=='pt-BR';lines=[]
        lines.append('# '+('Resultados de políticas autônomas' if pt else 'Autonomous policy results'))
        lines.append((f"Execução preservada: **{len(games):,} partidas, {check['decisions']:,} decisões**, {check['invalid_actions']} ações inválidas; HS e suas ablações: zero estados futuros. " if pt else f"Preserved run: **{len(games):,} matches, {check['decisions']:,} decisions**, {check['invalid_actions']} invalid actions; HS and its ablations: zero future states. ")+('Dados sintéticos; evidência de reutilização da política, sem validação operacional em Balatro.' if pt else 'Synthetic data; evidence of policy reuse, without operational validation in Balatro.'))
        lines.append('32 blocos por família; cada confronto usa 64 partidas com posições alternadas. Os intervalos de 95% reamostram blocos completos (4.000 reamostragens). P = (vitórias + 0,5 empates)/partidas; não é a taxa de vitórias estritas. Resultados exploratórios, sem correção de comparações múltiplas. Intervalos degenerados em jogos determinísticos descrevem estes casos finitos, não certeza universal.' if pt else '32 blocks per family; each matchup uses 64 games with swapped seats. The 95% intervals resample complete blocks (4,000 resamples). P = (wins + 0.5 draws)/games; this is not the strict win fraction. Results are exploratory, without multiple-comparison correction. Degenerate intervals in deterministic games describe these finite cases, not universal certainty.')
        for regime in ['work512','work4096','time2ms']:
            selected=[r for r in hs if r['section']=='tournament' and r['regime']==regime]
            if not selected:continue
            lines.append('## '+regime)
            lines.append(table(selected,[('Family',lambda r:r['family']),('Opponent',lambda r:r['opponent']),('W/D/L',lambda r:f"{r['wins']}/{r['draws']}/{r['losses']}"),('P [95% CI]',lambda r:f"{r['points']:.3f} [{r['points_ci_low']:.3f}, {r['points_ci_high']:.3f}]"),('Mean utility margin',lambda r:f"{r['mean_margin']:.2f}")]))
        lines.append('## '+('Ablações sem limite artificial' if pt else 'Uncapped ablations'))
        lines.append('A ausência de filtro e o controle lexicográfico preservam o objetivo; a enumeração probabilística explícita preserva a distribuição. Sem dependências/simplificada altera o modelo de avaliação; prioridade por recompensa altera o objetivo local. Igualdade de P=0,5 pode incluir vitórias alternadas por posição e resultados aleatórios.' if pt else 'Removing the filter and the lexicographic control preserve the objective; explicit probability enumeration preserves the distribution. No-dependencies/simplified changes the evaluation model; reward-first changes the local objective. P=0.5 can include seat-dependent wins and stochastic outcomes.')
        lines.append(table([r for r in hs if r['section']=='ablation'],[('Family',lambda r:r['family']),('Ablation/control',lambda r:r['opponent']),('W/D/L',lambda r:f"{r['wins']}/{r['draws']}/{r['losses']}"),('P [95% CI]',lambda r:f"{r['points']:.3f} [{r['points_ci_low']:.3f}, {r['points_ci_high']:.3f}]")]))
        lines.append('## '+('Custo na mesma observação inicial' if pt else 'Cost at the same initial observation'))
        lines.append('Uma execução de aquecimento excluída e sete medições C independentes por caso. Tempo mediano em µs. Heap é apenas alocação dinâmica pertencente à política: zero não significa memória total zero. RSS é de todo o processo com ambos os agentes, incluindo o efeito do processo lançador; não pode ser atribuído a uma política. Tabelas completas incluem dispersão, truncamento, memória e todos os contadores.' if pt else 'One excluded warmup and seven independent C measurements per case. Median time in µs. Heap is policy-owned dynamic allocation only: zero does not mean zero total memory. RSS covers the process with both agents, including launcher effects; it cannot be attributed to a policy. Complete tables include dispersion, truncation, memory and every counter.')
        lines.append(table([r for r in fixed if r['regime']=='work512' and r['policy'] in ['hs','lex_simple','hs_no_filter','hs_no_aggregation','greedy','expectimax','alphabeta','mcts']], [('Family',lambda r:r['family']),('Policy',lambda r:r['policy']),('µs',lambda r:f"{r['median_us']:.1f}"),('Evaluations',lambda r:f"{r['mean_fully_evaluated']:.0f}"),('Before eval',lambda r:f"{r['mean_eliminated_before']:.0f}"),('Successors',lambda r:f"{r['mean_future_states']:.0f}"),('Heap B',lambda r:r['max_owned_heap_bytes'])]))
        lines.append('## '+('Escalabilidade e limite temporal' if pt else 'Scaling and temporal boundary'))
        lines.append(table([r for r in boundary if r['family']=='branching'],[('Items',lambda r:r['size']),('Policy',lambda r:r['policy']),('Generated',lambda r:f"{r['mean_generated']:.0f}"),('Evaluated',lambda r:f"{r['mean_fully_evaluated']:.0f}"),('µs',lambda r:f"{r['median_us']:.1f}"),('Heap B',lambda r:r['max_owned_heap_bytes'])]))
        lines.append('No contraexemplo `tiny`, HS obtém utilidade 23 e o oráculo exato 130,2: arrependimento 107,2. O oráculo investe antes de pontuar. A profundidade zero da HS impede esse planejamento. Crescer o inventário também aumenta a enumeração de ações atuais; o filtro não elimina esse custo. `tiny_chance` fornece um segundo oráculo estocástico pequeno. As execuções sem limite de avaliações/tempo ainda usam profundidade 3 para expectimax/alfa-beta e 4.096 iterações para MCTS: só o oráculo completo certifica ótimo.' if pt else 'In the `tiny` counterexample, HS obtains utility 23 and the exact oracle 130.2: regret 107.2. The oracle invests before scoring. HS at depth zero cannot perform that planning. Increasing inventory also increases current-action enumeration; the filter does not eliminate that cost. `tiny_chance` supplies a second small stochastic oracle. Runs without time/evaluation caps still use depth 3 for expectimax/alpha-beta and 4,096 iterations for MCTS: only the complete oracle certifies optimality.')
        lines.append('## '+('Interpretação e arquivos' if pt else 'Interpretation and files'))
        lines.append('Os limites de 512/4.096 avaliações não igualam operações: geração, checagem de legalidade, alocação e o tamanho das distribuições variam. O limite de 2 ms é cooperativo e pode ser excedido por uma operação indivisível; tempos observados são preservados. Probabilidades desconhecidas usam o mesmo ponto médio para todos, sem aprendizado. Expectimax modela o oponente como uniforme; alfa-beta é adversarial e determinístico. MCTS usa UCT aberto com rollouts uniformes. Estas implementações de referência não representam o melhor desempenho possível de cada família.' if pt else 'The 512/4,096 evaluation caps do not equalize operations: generation, legality checks, allocation and distribution sizes differ. The 2 ms limit is cooperative and may overrun during an indivisible operation; observed times are retained. Unknown probabilities use the same midpoint for all policies, without learning. Expectimax models a uniform opponent; alpha-beta is adversarial and deterministic. MCTS uses open-loop UCT with uniform rollouts. These reference implementations do not represent the best possible performance of each algorithm family.')
        lines.append('Não se observou um mecanismo de otimização novo além dos componentes conhecidos: o controle lexicográfico completo é construído para selecionar a mesma ação. A diferença de implementação é filtrar trocas imediatamente equivalentes e reter só o vencedor atual. Avaliação rica pode mudar a qualidade; isso é separado de ganhos de tempo. Vitórias, derrotas e empates permanecem disponíveis.' if pt else 'No new optimization mechanism beyond the known components is established: the complete lexicographic control is constructed to select the same action. Implementation differences are filtering immediately equivalent swaps and retaining only the current winner. Rich evaluation can change quality; this is separated from timing gains. Wins, losses and ties remain available.')
        lines.append('[All matchups](matchups.csv), [online decision costs](decisions.csv), [fixed observations](fixed_observations.csv), [boundary/scaling](boundary.csv), [all outcome classifications](all_outcomes.json), [checks](checks.json), [raw games](../games.jsonl), [raw decisions](../decisions.jsonl.gz), [inputs](../inputs), [examples](../examples), [environment](../environment.json).')
        report='\n\n'.join(lines)+'\n'
        report=report.replace('32 blocos por família',f'{block_count} blocos por família').replace('32 blocks per family',f'{block_count} blocks per family').replace('64 partidas com posições',f'{2*block_count} partidas com posições').replace('64 games with swapped',f'{2*block_count} games with swapped')
        (out/f'results.{lang}.md').write_text(report)
    print(json.dumps(check,indent=2))
if __name__=='__main__':main()

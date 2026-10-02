#!/usr/bin/env python3
"""Large-history battles; retain raw histories separately from executed work."""
import argparse,gzip,hashlib,json,os,platform,random,subprocess,sys,time
from pathlib import Path
from report import read,summarize_games,summarize_decisions,write_csv,table
ROOT=Path(__file__).resolve().parents[1];HS=ROOT.parent
PROTOCOL=ROOT/'scenarios/futures_protocol.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def call(command):return subprocess.check_output(command,text=True,stderr=subprocess.STDOUT,timeout=120)
def fixture(binary,family,n,rounds,block):
    lines=call([str(binary),'dump',family,str(n)]).splitlines();rules=lines[1].split();rules[3]=str(rounds);lines[1]=' '.join(rules)
    for i in range(n):
        row=lines[2+i].split()
        if family!='future_deterministic':row[11]=row[12]=str([.25,.5,.75][block%3])
        lines[2+i]=' '.join(row)
    item_count=int(rules[0]);obs=lines[2+item_count].split();obs[2]=str(rounds*2);lines[2+item_count]=' '.join(obs)
    order=list(range(n));random.Random(90271+block).shuffle(order)
    for index in [3+item_count,4+item_count]:
        player=lines[index].split();lines[index]=' '.join(player[:6]+list(map(str,order)))
    return '\n'.join(lines)+'\n'
def policies(protocol,family):return ['hs',*protocol['opponents'],*protocol['extra_controls'].get(family,[])]
def histories(family,n,plies):
    common={'family':family,'items':n,'plies':plies,'count_type':'histories, not distinct states or executed operations'}
    if family=='future_deterministic':return common|{'action_histories':str(2**(n*plies)),'fixed_full_action_outcome_histories':'1','unmerged_action_outcome_leaves':str(2**(n*plies)),'bound':'exact full unmerged tree'}
    if family=='future_renewal':return common|{'action_histories':str(2**(n*plies)),'fixed_full_action_outcome_histories':str(2**(n*plies)),'unmerged_action_outcome_leaves':str(3**(n*plies)),'bound':'exact full unmerged tree'}
    return common|{'action_histories':None,'fixed_full_action_outcome_histories':None,'unmerged_action_outcome_leaves':str(3**(min(n,4)*plies)),'bound':'lower bound: no preparations and only subsets of the first four sources; cost <= refresh on every own turn'}
def analyze(out):
    protocol=json.loads((out/'protocol.json').read_text());games=list(read(out/'games.jsonl'))
    pairs=summarize_games(games);cost=summarize_decisions(read(out/'decisions.jsonl.gz'),['family','regime','policy'])
    fixed=summarize_decisions(read(out/'scaling.jsonl'),['family','items','rounds','regime','policy'])
    full=summarize_decisions(read(out/'uncapped.jsonl'),['family','policy'])
    folder=out/'analysis';folder.mkdir(exist_ok=True)
    for name,rows in [('matchups',pairs),('decisions',cost),('scaling',fixed),('uncapped',full)]:write_csv(folder/f'{name}.csv',rows)
    history=[histories(f,n,h*2) for f in protocol['families'] for n in protocol['scaling']['items'] for h in protocol['scaling']['rounds']]
    (folder/'history_counts.json').write_text(json.dumps(history,indent=2)+'\n')
    checks={'matches':len(games),'decisions':sum(r['decisions'] for r in cost),'invalid_actions':sum(sum(g['invalid_actions']) for g in games),
            'hs_future_states':sum(r['mean_future_states']*r['decisions'] for r in cost if r['policy'].startswith('hs')),
            'analysis_source_sha256':sha(Path(__file__))}
    (folder/'checks.json').write_text(json.dumps(checks,indent=2)+'\n')
    (folder/'all_outcomes.json').write_text(json.dumps({'exploratory_wins':[r for r in pairs if r['points_ci_low']>.5], 'exploratory_losses':[r for r in pairs if r['points_ci_high']<.5],'balanced_or_inconclusive':[r for r in pairs if r['points_ci_low']<=.5<=r['points_ci_high']],'caution':'Unadjusted exploratory paired-block intervals; synthetic scenarios, not a universal superiority claim.'},indent=2)+'\n')
    for lang in ['en','pt-BR']:
        pt=lang=='pt-BR';lines=['# '+('Batalhas com futuros combinatórios' if pt else 'Battles with combinatorial futures')]
        lines.append((f"**{len(games):,} partidas; {checks['decisions']:,} decisões; zero ações inválidas; HS com zero sucessores.** " if pt else f"**{len(games):,} matches; {checks['decisions']:,} decisions; zero invalid actions; HS has zero successors.** ")+('As sementes alternam posições; intervalos de 95% reamostram pares completos 4.000 vezes. P=(vitórias+0,5 empates)/partidas. Comparações exploratórias, sem correção múltipla.' if pt else 'Seeds swap seats; 95% intervals resample whole pairs 4,000 times. P=(wins+0.5 draws)/games. Exploratory comparisons, without multiple-testing correction.'))
        lines.append('## '+('Tamanho do espaço hipotético' if pt else 'Hypothetical history space'))
        lines.append(table([r for r in history if r['items']==8 and r['plies']==20],[('Family',lambda r:r['family']),('Raw leaves / lower bound',lambda r:f"{int(r['unmerged_action_outcome_leaves']):.3e}"),('Fixed-policy outcomes',lambda r:f"{int(r['fixed_full_action_outcome_histories']):.3e}" if r['fixed_full_action_outcome_histories'] else 'not stationary'),('Type',lambda r:r['bound'])]))
        lines.append('Esses números contam histórias da árvore sem reunir equivalências. Não são estados distintos, nós realmente visitados ou operações que se possa creditar como eliminadas pela HS. Em renewal, a soma por jogada é Σ C(n,k)2^k=3^n, portanto 3^(nH) folhas. Uma política que ativa todos os oito itens por 20 jogadas tem 2^160 resultados elementares. Em shared só se afirma o limite inferior de uma subárvore sempre legal, não uma potência de ramificação constante do jogo inteiro.' if pt else 'These numbers count histories without merging equivalences. They are not distinct states, visited nodes, or operations credited as suppressed by HS. In renewal, per-ply branching sums to Σ C(n,k)2^k=3^n, giving 3^(nH) leaves. A fixed policy activating all eight items for 20 plies has 2^160 elementary outcomes. For shared we claim only a lower bound from an always-legal subtree, not stationary branching for the entire game.')
        for regime in protocol['regimes']:
            lines.append('## '+regime)
            lines.append(table([r for r in pairs if r['regime']==regime],[('Family',lambda r:r['family']),('Opponent',lambda r:r['opponent']),('HS W/D/L',lambda r:f"{r['wins']}/{r['draws']}/{r['losses']}"),('P [95% CI]',lambda r:f"{r['points']:.3f} [{r['points_ci_low']:.3f}, {r['points_ci_high']:.3f}]"),('Mean utility margin',lambda r:f"{r['mean_margin']:.2f}")]))
        lines.append('## '+('Custo observado por decisão no torneio' if pt else 'Observed per-decision tournament cost'))
        lines.append(table([r for r in cost if r['regime']=='work1024'],[('Family',lambda r:r['family']),('Policy',lambda r:r['policy']),('Median µs',lambda r:f"{r['median_us']:.1f}"),('Mean evals',lambda r:f"{r['mean_fully_evaluated']:.1f}"),('Mean successors',lambda r:f"{r['mean_future_states']:.1f}"),('Max heap B',lambda r:r['max_owned_heap_bytes']),('Truncated fraction',lambda r:f"{r['fraction_truncated']:.3f}")]))
        lines.append('## '+('Escala na mesma observação: oito itens' if pt else 'Same-observation scaling: eight items'))
        lines.append(table([r for r in fixed if r['items']==8 and r['policy'] in ['hs','expectimax','alphabeta','mcts','renewal_exact']], [('Family',lambda r:r['family']),('Rounds/player',lambda r:r['rounds']),('Policy',lambda r:r['policy']),('Median µs',lambda r:f"{r['median_us']:.1f}"),('Completed depth',lambda r:r['max_completed_depth']),('Visited depth',lambda r:r['max_temporal_depth']),('Successors',lambda r:f"{r['mean_future_states']:.0f}")]))
        lines.append('## '+('Atribuição da vantagem e limites' if pt else 'Attribution and limitations'))
        lines.append('HS preserva avaliação completa, prioridade lexicográfica e seleção incremental. O filtro omite trocas aditivas equivalentes apenas em shared; nos casos sem preparação não há supressão antecipada. Greedy e lex_simple usam as mesmas distribuições analíticas das buscas e da HS. renewal_exact verifica condições de renovação sem custo e escolhe todos os itens não negativos: a dominância ponto a ponto torna essa decisão ótima para qualquer horizonte. Esse controle não é HS e é rejeitado em shared. Portanto, vantagem contra busca ingênua não significa dificuldade intrínseca ou vantagem exclusiva da supressão.' if pt else 'HS retains full evaluation, lexicographic priorities and streaming selection. Its filter skips equivalent additive swaps only in shared; preparation-free cases have no early suppression. Greedy and lex_simple use the same analytical distributions as search and HS. renewal_exact verifies free renewal conditions and activates every nonnegative item: pointwise dominance proves optimality at any horizon. This control is not HS and is rejected for shared. Advantage over naive search therefore does not establish intrinsic difficulty or a unique suppression advantage.')
        lines.append('Expectimax recebe horizonte 20 e mantém o último nível completo. Alfa-beta usa poda no cenário determinístico; não é aplicado a probabilidades. MCTS amostra trajetórias e nunca precisa enumerar todas as folhas: perdas podem decorrer de amostras insuficientes, geração de ações cara ou ruído dos rollouts uniformes. Todas essas implementações são referências simples; transposições, programação dinâmica ou reconhecimento de separabilidade podem alterar o resultado.' if pt else 'Expectimax requests horizon 20 and retains its last completed layer. Alpha-beta prunes in the deterministic family; it is not applied to chance games. MCTS samples trajectories and never needs all leaves: losses may reflect limited samples, expensive action generation or noisy uniform rollouts. These are simple reference implementations; transpositions, dynamic programming or separability recognition can change the result.')
        lines.append('Igual número de avaliações não iguala instruções. O limite de 5 ms é cooperativo; excesso observado é registrado. Heap próprio exclui pilha; RSS é do processo inteiro e pode herdar o máximo do lançador. O caso renewal é deliberadamente favorável à decisão local; shared acrescenta estoque disputado, energia limitada e efeitos permanentes. A função terminal é a mesma dos torneios anteriores. Resultados completos, inclusive perdas, permanecem abaixo.' if pt else 'Equal evaluation counts do not equal instructions. The 5 ms limit is cooperative; observed overruns are retained. Owned heap excludes stack; process RSS can inherit the launcher high-water mark. Renewal deliberately favors local decisions; shared adds contested stock, limited energy and permanent effects. Terminal utility matches the previous tournaments. Complete outcomes, including losses, remain available below.')
        lines.append('[Matchups](matchups.csv) · [decision metrics](decisions.csv) · [all scaling data](scaling.csv) · [uncapped local controls](uncapped.csv) · [history counts](history_counts.json) · [all outcomes](all_outcomes.json) · [protocol](../protocol.json) · [metadata](../environment.json) · [raw games](../games.jsonl) · [raw decisions](../decisions.jsonl.gz) · [exact small oracle checks](../oracle_checks.jsonl).')
        (folder/f'results.{lang}.md').write_text('\n\n'.join(lines)+'\n')
    print(json.dumps(checks),flush=True)
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--output',type=Path,default=ROOT/'results/raw/futures-2026-10-02');ap.add_argument('--binary',type=Path,default=HS/'build/agents/hs_arena');ap.add_argument('--smoke',action='store_true');ap.add_argument('--analyze',type=Path);a=ap.parse_args()
    if a.analyze:analyze(a.analyze.resolve());return
    out=a.output.resolve();binary=a.binary.resolve();out.mkdir(parents=True,exist_ok=False);(out/'inputs').mkdir()
    protocol=json.loads(PROTOCOL.read_text())
    if a.smoke:protocol['paired_seed_blocks']=2;protocol['regimes']={'work1024':protocol['regimes']['work1024']};protocol['scaling']={'items':[2,8],'rounds':[1,10],'regime':'work1024','warmups':1,'measured_repeats':2}
    (out/'protocol.json').write_text(json.dumps(protocol,indent=2)+'\n')
    sources={}
    for base in ['src','include','tests','tools']:
        for src in (ROOT/base).glob('*'):
            if src.suffix in ['.c','.h','.py']:
                rel=src.relative_to(HS);sources[str(rel)]=sha(src);dst=out/'source'/rel;dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(src.read_bytes())
    meta={'utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'command':sys.argv,'platform':platform.platform(),'cpu':call(['lscpu']),'compiler':call(['cc','--version']),'binary_sha256':sha(binary),'sources':sources,'compile_commands':(HS/'build/compile_commands.json').read_text(),'sizes':json.loads(call([str(binary),'sizes'])),'affinity':sorted(os.sched_getaffinity(0)),'process_timeout_seconds':120,'seed_blocks':protocol['paired_seed_blocks']}
    (out/'environment.json').write_text(json.dumps(meta,indent=2)+'\n')
    env=random.Random(810271);planner=random.Random(971420);jobs=[];blocks=[]
    for family in protocol['families']:
        for block in range(protocol['paired_seed_blocks']):
            path=out/'inputs'/f'{family}_{block}.ag';path.write_text(fixture(binary,family,8,10,block))
            base={'family':family,'block':block,'world_seed':env.getrandbits(64),'policy_seed':planner.getrandbits(64),'input':str(path.relative_to(out)),'input_sha256':sha(path)};blocks.append(base)
            for regime in protocol['regimes']:
                for opponent in policies(protocol,family)[1:]:
                    for role in [0,1]:jobs.append(base|{'regime':regime,'pair_a':'hs','pair_b':opponent,'role':role,'section':'futures'})
    random.Random(82147).shuffle(jobs);(out/'schedule.json').write_text(json.dumps(jobs,indent=2)+'\n');(out/'blocks.json').write_text(json.dumps(blocks,indent=2)+'\n')
    with (out/'games.jsonl').open('w',buffering=1) as games,gzip.open(out/'decisions.jsonl.gz','wt') as decisions:
        for index,job in enumerate(jobs):
            left,right=('hs',job['pair_b']) if not job['role'] else (job['pair_b'],'hs');budget=protocol['regimes'][job['regime']]
            cmd=[str(binary),'match',str(out/job['input']),left,right,str(job['world_seed']),*map(str,budget),str(job['policy_seed'])]
            try:
                parsed=[json.loads(s) for s in call(cmd).splitlines()];game=parsed[-1]
                assert game['type']=='game' and game['invalid_actions']==[0,0]
                ident={k:v for k,v in job.items() if k not in ['input','input_sha256']};ident['game_id']=index
                games.write(json.dumps(ident|game)+'\n')
                for decision in parsed[:-1]:
                    m=decision['metrics'];assert not budget[0] or m['fully_evaluated']<=budget[0]
                    assert not decision['policy'].startswith('hs') or m['future_states']==0
                    decisions.write(json.dumps(ident|decision)+'\n')
            except Exception as error:
                (out/'failure.json').write_text(json.dumps({'job':job,'command':cmd,'error':str(error)},indent=2)+'\n');raise
            if index%100==0:print(f'{index+1}/{len(jobs)} games: {job["family"]} {job["regime"]}',flush=True)
    print('Tournament complete; measuring fixed-observation scaling.',flush=True)
    with (out/'scaling.jsonl').open('w') as f:
        for family in protocol['families']:
            for n in protocol['scaling']['items']:
                for rounds in protocol['scaling']['rounds']:
                    p=out/'inputs'/f'scale_{family}_{n}_{rounds}.ag';p.write_text(fixture(binary,family,n,rounds,1))
                    for policy in policies(protocol,family):
                        regime=protocol['scaling']['regime'];budget=[4096,0,20,4096] if regime=='work4096' else [1024,0,20,4096]
                        cmd=[str(binary),'decide',str(p),policy,*map(str,budget),'17381'];call(cmd)
                        for rep in range(protocol['scaling']['measured_repeats']):f.write(json.dumps({'family':family,'items':n,'rounds':rounds,'regime':regime,'repeat':rep}|json.loads(call(cmd)))+'\n')
                    print(f'scaling {family} n={n} rounds={rounds}',flush=True)
    with (out/'uncapped.jsonl').open('w') as f:
        for family in protocol['families']:
            for policy in [p for p in policies(protocol,family) if p not in ['expectimax','alphabeta','mcts']]:
                path=out/'inputs'/f'{family}_0.ag';cmd=[str(binary),'decide',str(path),policy,'0','0','20','4096','17381'];call(cmd)
                for rep in range(7):f.write(json.dumps({'family':family,'repeat':rep}|json.loads(call(cmd)))+'\n')
    with (out/'oracle_checks.jsonl').open('w') as f:
        for family in ['future_renewal','future_deterministic']:
            p=out/'inputs'/f'oracle_{family}.ag';p.write_text(fixture(binary,family,2,2,1));answers=[]
            for policy in ['hs','greedy','renewal_exact','oracle']+(['alphabeta'] if family=='future_deterministic' else []):
                r=json.loads(call([str(binary),'decide',str(p),policy,'0','0','20','4096','17381']));answers.append(r['action']);f.write(json.dumps({'family':family}|r)+'\n')
                assert r['search_complete']
            assert all(x==answers[0] for x in answers)
    analyze(out)
if __name__=='__main__':main()

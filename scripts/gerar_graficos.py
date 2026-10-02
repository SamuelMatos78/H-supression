#!/usr/bin/env python3
"""Plot verified raw-derived data; never edits archived results."""
import csv,json,os,sys
from pathlib import Path
os.environ.setdefault('MPLCONFIGDIR','/tmp/aramuny-matplotlib')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
ROOT=Path(__file__).resolve().parents[1];DATA=ROOT/'dados/derivados';OUT=ROOT/'graficos'
sys.path.insert(0,str(ROOT/'scripts'))
import auditar
FAMILIES=['future_deterministic','future_renewal','future_shared'];REGIMES=['work1024','work4096','time5ms']
LABEL={'future_deterministic':'Determinístico','future_renewal':'Renovável probabilístico','future_shared':'Recursos e mercado compartilhados'}
POLICIES=['hs','greedy','lex_simple','hs_no_filter','hs_no_aggregation','expectimax','alphabeta','mcts','renewal_exact']
COLORS=dict(zip(POLICIES,plt.get_cmap('tab10').colors))
def read(name):
 with (DATA/f'{name}.csv').open() as f:return list(csv.DictReader(f))
def save(fig,name,caption):
 if name=='06_escalabilidade':
  payload=[{'panel':i,'series':[{'policy':line.get_label(),'x':list(map(float,line.get_xdata())),'median_ms':list(map(float,line.get_ydata()))} for line in ax.lines]} for i,ax in enumerate(fig.axes)]
  (OUT/'06_pontos.json').write_text(json.dumps(payload,indent=2)+'\n')
 fig.text(.5,.012,caption,ha='center',va='bottom',fontsize=9)
 fig.tight_layout(rect=(0,.07,1,.96))
 for ext in ['png','svg']:fig.savefig(OUT/f'{name}.{ext}',dpi=170,metadata={'Date':None} if ext=='svg' else None)
 plt.close(fig)
def grids(rows,field,title,unit,name,scale='linear',factor=1):
 fig,axes=plt.subplots(3,3,figsize=(17,13));fig.suptitle(title,fontsize=17)
 for i,fam in enumerate(FAMILIES):
  for j,reg in enumerate(REGIMES):
   ax=axes[i,j];rs=[next(r for r in rows if r['family']==fam and r['regime']==reg and r['policy']==p) for p in POLICIES if any(r['family']==fam and r['regime']==reg and r['policy']==p for r in rows)]
   values=[float(r[field])*factor for r in rs];ax.barh([r['policy'] for r in rs],values,color=[COLORS[r['policy']] for r in rs]);ax.invert_yaxis();ax.set_title(f'{LABEL[fam]}\n{reg}',fontsize=10);ax.set_xlabel(unit);ax.grid(axis='x',alpha=.25)
   if scale=='log':ax.set_xscale('log')
   elif scale=='symlog':ax.set_xscale('symlog',linthresh=1)
   if field=='fraction_truncated':ax.set_xlim(0,1)
   for k,v in enumerate(values):
    if v==0:ax.text(0,k,' 0',va='center',fontsize=8)
 save(fig,name,'Torneio: 8 fontes, 20 jogadas, 16 blocos × 2 posições por rival; HS aparece em todos os confrontos.\nAgregações por política usam observações visitadas diferentes. Eixo symlog, quando usado, mantém zero visível.')
def main():
 auditar.main();OUT.mkdir(exist_ok=True)
 plt.rcParams.update({'font.size':10,'svg.hashsalt':'aramuny-hs','axes.spines.top':False,'axes.spines.right':False})
 matches=read('matchups');cost=read('decisions');scale=read('scaling');uncapped=read('uncapped')
 fig,axes=plt.subplots(3,3,figsize=(17,13));fig.suptitle('Qualidade competitiva da HS: vitórias, empates e derrotas',fontsize=17)
 for i,fam in enumerate(FAMILIES):
  for j,reg in enumerate(REGIMES):
   ax=axes[i,j];rows=[r for r in matches if r['family']==fam and r['regime']==reg];y=np.arange(len(rows));left=np.zeros(len(rows))
   for field,label,color in [('wins','Vitórias','#16806a'),('draws','Empates','#b5bec9'),('losses','Derrotas','#b94b4b')]:
    vals=np.array([int(r[field]) for r in rows]);ax.barh(y,vals,left=left,label=label,color=color)
    for k,v in enumerate(vals):
     if v:ax.text(left[k]+v/2,k,str(v),ha='center',va='center',fontsize=8)
    left+=vals
   ax.set_yticks(y,[r['opponent'] for r in rows]);ax.invert_yaxis();ax.set_xlim(0,32);ax.set_xlabel('Partidas (32 por confronto)');ax.set_title(f'{LABEL[fam]}\n{reg}',fontsize=10)
 handles,labels=axes[0,0].get_legend_handles_labels();fig.legend(handles,labels,loc='upper center',bbox_to_anchor=(.5,.965),ncol=3)
 save(fig,'01_confrontos','Todas as 2.016 partidas preservadas. Contagens descritivas; não demonstram superioridade geral nem significância ajustada.\nSementes pareadas com inversão de posição. Intervalos exploratórios estão nas tabelas, sem inferência populacional de 100%.')
 grids(cost,'median_us','Tempo mediano por decisão','Milissegundos (log)','02_tempo','log',.001)
 grids(cost,'mean_fully_evaluated','Avaliações completas por decisão','Média de avaliações','03_avaliacoes','log')
 grids(cost,'mean_future_states','Aplicações hipotéticas de transição por decisão','Média de sucessores, incluindo repetições (symlog)','04_sucessores','symlog')
 grids(cost,'fraction_truncated','Fração com o indicador truncated=true','Proporção de decisões','05_truncamentos')
 grids(cost,'max_owned_heap_bytes','Memória: máximo de heap próprio observado','Bytes alocados pela política (symlog)','08_memoria','symlog')
 chosen=['hs','greedy','expectimax','alphabeta','mcts','renewal_exact'];fig,axes=plt.subplots(3,3,figsize=(17,12));fig.suptitle('Escalabilidade na mesma observação inicial — work4096',fontsize=17)
 for i,fam in enumerate(FAMILIES):
  for policy in chosen:
   rows=[r for r in scale if r['family']==fam and r['policy']==policy]
   if not rows:continue
   by_n=sorted([r for r in rows if int(r['rounds'])==10],key=lambda r:int(r['items']))
   by_h=sorted([r for r in rows if int(r['items'])==8],key=lambda r:int(r['rounds']))
   for ax,rs,x in [(axes[i,0],by_n,lambda r:int(r['items'])),(axes[i,1],by_h,lambda r:2*int(r['rounds']))]:ax.plot([x(r) for r in rs],[float(r['median_us'])/1000 for r in rs],'-o',label=policy,color=COLORS[policy],markersize=4)
   # Legal current candidates visited by HS, including subsequently filtered equivalents.
   xs=[]
   for r in by_n:
    controls=[v for v in scale if v['family']==fam and v['items']==r['items'] and v['rounds']=='10' and v['policy']=='hs']
    c=controls[0];xs.append(float(c['mean_generated'])-float(c['mean_invalid_candidates']))
   axes[i,2].plot(xs,[float(r['median_us'])/1000 for r in by_n],'-o',label=policy,color=COLORS[policy],markersize=4)
  for j,ax in enumerate(axes[i]):ax.set_yscale('log');ax.set_ylabel('Mediana ms (log)');ax.set_title(LABEL[fam]);ax.grid(alpha=.25)
  axes[i,0].set_xlabel('Fontes ativas (horizonte 20)');axes[i,1].set_xlabel('Jogadas restantes (8 fontes)');axes[i,2].set_xlabel('Ações atuais legais geradas pela HS');axes[i,2].set_xscale('log')
 handles,labels=axes[0,0].get_legend_handles_labels();fig.legend(handles,labels,loc='upper center',bbox_to_anchor=(.5,.96),ncol=6)
 save(fig,'06_escalabilidade','Cada ponto: mediana de 7 execuções após 1 aquecimento, mesma observação e semente 17381; cap 4.096 avaliações.\nOs pontos podem ser truncados. Ações atuais incluem equivalências omitidas antes de avaliar; não são histórias futuras.')
 ablations=['hs','hs_no_filter','hs_no_aggregation','lex_simple','greedy'];fig,axes=plt.subplots(3,3,figsize=(17,12));fig.suptitle('Ablações e controles locais — observação inicial sem cap de tempo/avaliações',fontsize=15)
 for i,fam in enumerate(FAMILIES):
  rows=[next(r for r in uncapped if r['family']==fam and r['policy']==p) for p in ablations]
  for j,(field,unit) in enumerate([('median_us','Mediana ms'),('mean_fully_evaluated','Avaliações médias'),('mean_eliminated_before','Omissões pré-avaliação médias')]):
   ax=axes[i,j];vals=[float(r[field])/(1000 if field=='median_us' else 1) for r in rows];ax.barh(ablations,vals,color=[COLORS[p] for p in ablations]);ax.invert_yaxis();ax.set_title(LABEL[fam]);ax.set_xlabel(unit);ax.grid(axis='x',alpha=.25)
   for k,v in enumerate(vals):ax.text(v,k,f' {v:.3g}',va='center',fontsize=8)
   ax.set_xlim(0,max(vals)*1.25 if max(vals)>0 else 1)
 save(fig,'07_ablacoes','7 repetições, 8 fontes, 20 jogadas. Qualidade competitiva das mesmas variantes: figura 01.\nSem dependências e reward_first não foram medidas neste protocolo; não se atribui efeito causal a variantes ausentes.')
 (OUT/'index.json').write_text(json.dumps({'source':'dados/ultimo_experimento','tables':'dados/derivados','matplotlib':matplotlib.__version__,'plots':sorted(p.name for p in OUT.glob('*.png'))},indent=2)+'\n')
 print('Generated',len(list(OUT.glob('*.png'))),'PNG + SVG figures')
if __name__=='__main__':main()

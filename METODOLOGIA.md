# Metodologia e auditoria dos limites

## Problema e matemática implementada

Dois agentes alternam decisões com inventários públicos ordenados, recursos, mercado e horizonte finito. Uma ação combina até uma preparação atual (compra, venda, substituição, troca de ordem ou nenhuma) e a ativação de um subconjunto. São componentes de uma decisão imediata, não turnos futuros simulados.

O objetivo terminal comum, com bônus de meta 20 neste protocolo, é

\[
U_i=\sum_t[S_{i,t}+20\mathbf1(S_{i,t}\ge T)]+0{,}1(C_i^{final}+E_i^{final}).
\]

Dinheiro e energia são `C` e `E`. A comparação competitiva usa `U_i-U_adversário`. As prioridades locais da HS podem conflitar com essa utilidade. `ag_decide()` recebe regras e observação imutável; não recebe semente privada do ambiente nem identidade da política adversária. Todos usam o mesmo motor e probabilidades públicas conhecidas. Este confronto não mede inferência de probabilidades ocultas.

Em [engine.c](codigo/src/engine.c), `ag_evaluate()` resolve uma visão algébrica dos IDs da preparação, custos, dependências, efeitos passivos e ordem dos modificadores. Custos são pagos antes dos ganhos da ativação. A avaliação HS não chama `ag_apply()`, que avança o estado somente no ambiente e nas buscas.

Para uma ação admissível, o motor obtém

\[
S=c+\sum_{j=1}^{k}w_jX_j,\quad X_j\sim Bernoulli(p_j),
\qquad D_j(s)=(1-p_j)D_{j-1}(s)+p_jD_{j-1}(s-w_j).
\]

Os coeficientes incorporam a ordem dos efeitos; os ensaios são independentes no modelo. A convolução parte de `D_0(c)=1` e reúne pontuações **exatamente iguais**. Calcula `E[S]=Σ_s sD_k(s)`, `P(S≥T)=Σ_{s≥T}D_k(s)` e `E[recompensa]=E[S]+20P(S≥T)`. Coeficientes distintos podem conservar os `2^k` resultados: não há compressão linear geral. O teste preservado verifica `S=2+20 Binomial(4;0,2)`, cinco pontuações contra 16 atribuições, `P(S≥42)=0,1808`, `E[S]=18`.

Em [policies.c](codigo/src/policies.c), as operações HS são distintas:

| Operação | Implementação | Efeito |
| --- | --- | --- |
| Avaliação | `ag_evaluate(..., AG_FULL, ...)` | Calcula probabilidades, crescimento, dinheiro e esperança |
| Prioridade lexicográfica | `key()` e `greater()` | Maximiza `(round(10^12 P)/10^12, crescimento, Δdinheiro, E[S])`, nessa ordem; primeiro empate |
| Supressão antes de avaliar | `ag_equivalent_swap()` em `immediate_visit()` | Omite trocas legais aditivas equivalentes, sem multiplicadores diferentes de 1 ou pontuação dependente de dinheiro no intervalo |
| Seleção após avaliar | Vencedor corrente de `immediate_visit()` | Descarta um candidato por comparação, mantendo somente o melhor atual |

O filtro preserva avaliação imediata, não todas as oportunidades futuras. Ações omitidas ainda são geradas e validadas. A prioridade opcional por menos cartas do Derrisor não foi incluída neste módulo. Não há soma ponderada das quatro prioridades. Nas famílias renovável/determinística, sem preparações, não ocorre supressão antecipada; em shared o filtro atua sobre trocas. Todas mantêm avaliação, comparação lexicográfica e seleção incremental.

## Cenários, sementes e orçamentos

| Família | Configuração e interpretação |
| --- | --- |
| `future_renewal` | 8 fontes renováveis gratuitas, base 3 e bônus `2^i X_i`; sem preparação/mercado/custo. Deliberadamente favorável à decisão local |
| `future_deterministic` | Substitui bônus por base adicional `0,5×2^i`; admite minimax com poda |
| `future_shared` | Custo de energia 1 por fonte, energia inicial 8/reposição 4, dinheiro 12; mercado único: passivo ×1,5 por 10 e treinamento base 6/crescimento 2 por 5; preparações |

Alvo com oito fontes: `24+0,5×255=151,5`. Ativar tudo no renovável produz `S=24+Σ2^iX_i`; a meta exige sucesso do maior bit, logo `P=p` e `E[recompensa]=24+275p`. A dominância daí decorrente não se transfere ao cenário compartilhado.

São 16 blocos por família, probabilidades públicas ciclando `0,25; 0,5; 0,75`, mesma permutação de inventário nos dois assentos, e posições invertidas para cada rival/orçamento. As famílias renovável, determinística e compartilhada têm 7, 8 e 6 rivais: 32 partidas por célula, **2.016 partidas e 40.320 decisões**. HS participa de todos os pares; métricas agregadas por política usam observações diferentes.

| Regime | Avaliações | Prazo da política | Profundidade das árvores | Iterações MCTS |
| --- | ---: | ---: | ---: | ---: |
| work1024 | 1.024 | sem prazo | 20 | 4.096 |
| work4096 | 4.096 | sem prazo | 20 | 4.096 |
| time5ms | sem cap explícito | 5 ms cooperativos | 20 | 4.096 |

Não há cap de memória. O wrapper impõe 120 segundos por subprocesso. Igual número de avaliações não iguala instruções: geração, legalidade, distribuição, alocações e seleção têm custos diferentes. Work4096 deliberadamente permite tempos acima de 5 ms; os regimes não acumulam limites.

Sementes mestras: 810271 (ambiente), 971420 (planejamento), 90271+bloco (ordem pública), 82147 (agenda). Valores concretos de 64 bits estão em `blocks.json`/`schedule.json`; não houve seleção por resultado. O ambiente usa sorteios indexados por semente, turno, item e repetição, comuns aos pares; o planejador tem RNG separado. Sondagens usam 17381. O script de verificação regenera as 98 entradas e agenda completa.

Escala: 2/4/6/8 fontes × 2/4/10/20 jogadas, mesma observação, work4096, um aquecimento excluído e sete repetições: **2.688 decisões**. Controles locais sem caps de tempo/avaliações: **119 decisões**. Oráculos em dois itens e quatro jogadas: **nove decisões**. Não há busca exaustiva ilimitada concluída no caso grande.

## Rivais e ablações

| Política | Implementação medida |
| --- | --- |
| `greedy` | Maximiza `E[recompensa]+0,1(Δdinheiro+Δenergia)` localmente, com avaliador completo |
| `lex_simple` | Mesmos avaliador/prioridades HS, sem filtro, armazena chaves e seleciona ao final; reconstrói testemunha |
| `hs_no_filter` | Seleção incremental, sem omissão por equivalência |
| `hs_no_aggregation` | Enumera atribuições elementares; mantém filtro/prioridades; zero sucessores temporais |
| `expectimax` | Aprofundamento iterativo, esperança dos eventos, maximização própria e adversário uniforme sobre ações legais |
| `alphabeta` | Minimax determinístico com poda e aprofundamento iterativo; somente família determinística |
| `mcts` | UCT aberto sobre históricos de ações, amostras novas, rollout uniforme legal; escolhe filho raiz mais visitado, desempate pela média |
| `renewal_exact` | Verifica condições de renovação gratuita não negativa e ativa tudo: dominância ponto a ponto, uma avaliação; rejeitado em shared |
| `oracle` | Expectiminimax exaustivo, adversário minimizador, horizonte completo; só casos minúsculos |

Expectimax não conhece a política real do rival e não é minimax estocástico. MCTS usa `sign×média/100 + sqrt(2 log(N+1)/(n+10^-9))`; sinal conforme jogador, valor parcial propagado quando o rollout é interrompido. Não herda automaticamente todas as garantias teóricas de UCT. Não há transposições, ordenação avançada ou redução geral por separabilidade. [Referências verificáveis](referencias/README.md).

## Causas de interrupção: registro e inferência

O histórico registra `metrics.truncated`, mas **não registra código de causa**. No código, `exhausted()` detecta avaliações/prazo; adicionalmente, `ag_decide()` marca truncamento ao aplicar passagem de fallback por ausência de ação encontrada. Não é possível obter retrospectivamente um evento de causa que não foi instrumentado.

[limites.csv](dados/derivados/limites.csv) separa cada política/cenário/regime. `truncated_consistent_eval_cap` significa flag e contagem final igual ao único cap ativo de avaliações. `truncated_consistent_time_cap` significa flag e tempo observado pelo menos igual ao único prazo ativo, sem cap de avaliações. Todos os truncamentos históricos são compatíveis com um desses critérios. Isso sustenta a interpretação, mas **não transforma inferências em causas individuais registradas**. Não se adicionou instrumentação ao motor.

| Condição | Evidência e interpretação |
| --- | --- |
| Cap de avaliações | Checado nos visitantes/iterações. Árvores, MCTS e lex_simple reservam uma avaliação para a testemunha final |
| Prazo | Checado cooperativamente; avaliação, geração, seleção, liberação ou testemunha podem ultrapassar o instante limite |
| Profundidade configurada | Árvores iteram até `min(depth, plies)`. Parada normal; se `depth<plies`, `search_complete=false` não implica truncamento |
| Camada incompleta | `completed_depth` retém última camada inteira; `temporal_depth` pode incluir trabalho parcial posterior |
| Iterações MCTS | Pode terminar normalmente ao atingir 4.096 iterações; `search_complete` sempre falso. Seu rollout segue turnos restantes, sem usar separadamente o campo `depth` |
| Memória esgotada | Falha de alocação abortaria o processo; não seria um simples flag. Não observada nos registros |
| Estouro de pilha | Não há instrumentação histórica específica; não observado nem inferido a partir de truncamento |
| Falha/timeout do processo | Runner escreveria `failure.json` e interromperia a execução. Os registros completos não contêm tal falha; isso não demonstra imunidade em outras escalas |
| Conclusão normal | Fluxo de candidatos completo para políticas locais; horizonte completo para árvores; limite de iterações para MCTS. São noções diferentes |

O aprofundamento iterativo mantém a ação da **última profundidade completa**. Se a primeira camada não termina, conserva o melhor candidato raiz já processado completamente, caso exista. Se nenhum terminou, retorna passagem legal e marca truncamento. Não substitui a última camada concluída por uma estimativa da camada parcial posterior. `completed_depth=0` sozinho não distingue candidato raiz parcialmente enumerado de fallback, pois esse evento não foi registrado.

Com work4096, as árvores renovável/determinística registram profundidade concluída 1; nas últimas jogadas ela já cobre o horizonte restante. Em shared, as 320 decisões Expectimax desse regime não completam sequer a primeira camada. Não concluíram profundidade 20. MCTS não certifica camadas: seu zero tem outro significado. **Duas decisões MCTS de shared/time5ms terminaram normalmente as 4.096 iterações**, apesar de `search_complete=false`.

## Limites ultrapassados e execução sem cap

Houve **zero extrapolações do cap de avaliações**. No regime de 5 ms, **2.457 decisões ultrapassaram o prazo**, incluindo **20 sem flag**: a execução pode atravessar o prazo depois do último checkpoint. Máximo: **5,118448 ms**; excesso **0,118448 ms**; razão **1,0236896**. As [tabelas geradas](dados/derivados/RESULTADOS.md) apresentam limite, consumo e excesso por política/cenário; o CSV também contém p95. Esses eventos medem comportamento cooperativo, não falha de processo.

Execução sem cap explícito de tempo/avaliações ainda tem restrições: catálogo, inventário, ensaios, horizonte, profundidade/iterações quando aplicáveis e timeout do processo. As medições uncapped aqui são locais; não são MCTS ou árvore grande ilimitada. Consumo elevado no regime de avaliações é variável experimental, não erro por ultrapassar um prazo que esse regime não impõe.

## Significado dos contadores e memória

| Métrica | Mede | Não representa |
| --- | --- | --- |
| Histórias hipotéticas | Folhas de árvore sem reunir equivalências | Estados únicos, trabalho executado ou supressões HS |
| `generated` | Entradas reais no visitante, em raiz e buscas | Todas as máscaras internas/instruções |
| `fully_evaluated` | Avaliações completas, inclusive testemunha adicional | Trabalho de CPU igual entre políticas |
| `future_states` | Transições hipotéticas `ag_apply`, com repetições/amostras | Estados únicos; transições reais do ambiente |
| `eliminated_before` | Ações legais omitidas antes da avaliação completa | Geração evitada ou histórias futuras eliminadas |
| `defeated_after` | Derrotas em comparações locais | Avaliações evitadas: já ocorreram |
| `elapsed_ns` | Tempo na fronteira C da política | Parsing, impressão, Python e transição real |
| `peak_owned_heap_bytes` | Pico de alocações próprias rastreadas | Pilha, overhead do alocador, runtime ou memória total |
| `process_peak_rss_kib` | Pico RSS do processo completo | Memória exclusiva de uma política |

RSS do torneio: **26.240–26.692 KiB**; pode refletir máximo herdado do lançador nesta plataforma. Heap zero não significa memória zero, especialmente em recursão. Pico de pilha não foi medido. Para HS, `generated = invalid_candidates + fully_evaluated + eliminated_before`; fluxo não vazio tem `defeated_after = fully_evaluated - 1`. Coleta MCTS e testemunhas adicionais não obedecem à mesma identidade.

## Estatística e ambiente

Qualidade: V/E/D, pontos `(V+0,5E)/N` e margem de utilidade. Intervalos percentis exploratórios de 95% usam 4.000 reamostragens de **blocos completos dos dois assentos**, sem correção múltipla. Intervalos degenerados `[1,1]` não provam taxa populacional de 100%. Cenários sintéticos não constituem amostra aleatória de Balatro.

[environment.json](dados/ultimo_experimento/environment.json) preserva CPU Intel i5-1035G1, Linux, compilador, afinidade e comandos. Caminhos absolutos nesse arquivo são proveniência; não são dependências da reprodução. A soma dos tempos C é subtotal, não duração de parede. Veja [proveniência e exclusões](referencias/PROVENIENCIA.md).

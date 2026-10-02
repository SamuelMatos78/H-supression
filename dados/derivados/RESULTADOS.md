# Resultados recalculados dos registros brutos

Gerado por `scripts/auditar.py`. Sem edição manual. Todas as quatro tabelas CSV foram comparadas byte a byte com as tabelas arquivadas.

2016 partidas; 40320 decisões; 2688 medições de escala; 119 medições locais sem limites de tempo/avaliações; 9 verificações por oráculo.

## work1024

| Cenário | Rival | V/E/D HS | Pontos [IC exploratório 95%] | Margem média |
| --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | expectimax | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | greedy | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_aggregation | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_filter | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | lex_simple | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | mcts | 32/0/0 | 1.000 [1.000, 1.000] | 584.41 |
| future_deterministic | renewal_exact | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | mcts | 30/0/2 | 0.938 [0.844, 1.000] | 486.19 |
| future_renewal | renewal_exact | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | mcts | 31/0/1 | 0.969 [0.906, 1.000] | 930.73 |


## work4096

| Cenário | Rival | V/E/D HS | Pontos [IC exploratório 95%] | Margem média |
| --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | expectimax | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | greedy | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_aggregation | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_filter | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | lex_simple | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | mcts | 32/0/0 | 1.000 [1.000, 1.000] | 417.70 |
| future_deterministic | renewal_exact | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | mcts | 29/0/3 | 0.906 [0.812, 1.000] | 416.31 |
| future_renewal | renewal_exact | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | mcts | 31/0/1 | 0.969 [0.906, 1.000] | 916.99 |


## time5ms

| Cenário | Rival | V/E/D HS | Pontos [IC exploratório 95%] | Margem média |
| --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | expectimax | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | greedy | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_aggregation | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | hs_no_filter | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | lex_simple | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_deterministic | mcts | 32/0/0 | 1.000 [1.000, 1.000] | 635.02 |
| future_deterministic | renewal_exact | 0/32/0 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_renewal | mcts | 32/0/0 | 1.000 [1.000, 1.000] | 521.06 |
| future_renewal | renewal_exact | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | expectimax | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | greedy | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_aggregation | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | hs_no_filter | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | lex_simple | 16/0/16 | 0.500 [0.500, 0.500] | 0.00 |
| future_shared | mcts | 32/0/0 | 1.000 [1.000, 1.000] | 1163.52 |


## Observação inicial: 8 itens, 10 rodadas/jogador, work4096

| Cenário | Política | Mediana ms | Avaliações médias | Sucessores médios | Heap máximo B | Nível concluído |
| --- | --- | --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 1.227394 | 4096.0 | 4095.0 | 0 | 1 |
| future_deterministic | expectimax | 1.203999 | 4096.0 | 4095.0 | 0 | 1 |
| future_deterministic | greedy | 0.091847 | 256.0 | 0.0 | 0 | 0 |
| future_deterministic | hs | 0.093756 | 256.0 | 0.0 | 0 | 0 |
| future_deterministic | hs_no_aggregation | 0.095542 | 256.0 | 0.0 | 0 | 0 |
| future_deterministic | hs_no_filter | 0.090265 | 256.0 | 0.0 | 0 | 0 |
| future_deterministic | lex_simple | 0.097464 | 257.0 | 0.0 | 18432 | 0 |
| future_deterministic | mcts | 53.827596 | 4096.0 | 4095.0 | 34816 | 0 |
| future_deterministic | renewal_exact | 0.002078 | 1.0 | 0.0 | 0 | 0 |
| future_renewal | expectimax | 13.410602 | 4096.0 | 104159.0 | 0 | 1 |
| future_renewal | greedy | 0.420950 | 256.0 | 0.0 | 0 | 0 |
| future_renewal | hs | 0.419049 | 256.0 | 0.0 | 0 | 0 |
| future_renewal | hs_no_aggregation | 0.165498 | 256.0 | 0.0 | 0 | 0 |
| future_renewal | hs_no_filter | 0.412790 | 256.0 | 0.0 | 0 | 0 |
| future_renewal | lex_simple | 0.433473 | 257.0 | 0.0 | 18432 | 0 |
| future_renewal | mcts | 58.376167 | 4096.0 | 4095.0 | 34816 | 0 |
| future_renewal | renewal_exact | 0.022279 | 1.0 | 0.0 | 0 | 0 |
| future_shared | expectimax | 14.332019 | 4096.0 | 104720.0 | 0 | 0 |
| future_shared | greedy | 6.911835 | 4096.0 | 0.0 | 0 | 0 |
| future_shared | hs | 2.370367 | 768.0 | 0.0 | 0 | 0 |
| future_shared | hs_no_aggregation | 1.552044 | 768.0 | 0.0 | 0 | 0 |
| future_shared | hs_no_filter | 6.570349 | 4096.0 | 0.0 | 0 | 0 |
| future_shared | lex_simple | 6.845530 | 4096.0 | 0.0 | 294912 | 0 |
| future_shared | mcts | 4046.460677 | 4096.0 | 4095.0 | 544768 | 0 |


## Prazo cooperativo de 5 ms: consumo e extrapolação

| Cenário | Política | N | Truncadas | Acima de 5 ms | Mediana ms | Máximo ms | Excesso máximo ms | Razão máx./limite |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 320 | 304 | 304 | 5.0007 | 5.0365 | 0.0365 | 1.007 |
| future_deterministic | expectimax | 320 | 304 | 304 | 5.0007 | 5.0349 | 0.0349 | 1.007 |
| future_deterministic | greedy | 320 | 0 | 0 | 0.0784 | 0.1306 | 0.0000 | 0.026 |
| future_deterministic | hs | 2560 | 0 | 0 | 0.0811 | 0.3713 | 0.0000 | 0.074 |
| future_deterministic | hs_no_aggregation | 320 | 0 | 0 | 0.0754 | 0.1906 | 0.0000 | 0.038 |
| future_deterministic | hs_no_filter | 320 | 0 | 0 | 0.0755 | 0.1484 | 0.0000 | 0.030 |
| future_deterministic | lex_simple | 320 | 0 | 0 | 0.0790 | 0.1573 | 0.0000 | 0.031 |
| future_deterministic | mcts | 320 | 320 | 320 | 5.0013 | 5.0325 | 0.0325 | 1.007 |
| future_deterministic | renewal_exact | 320 | 0 | 0 | 0.0003 | 0.0064 | 0.0000 | 0.001 |
| future_renewal | expectimax | 320 | 304 | 304 | 5.0177 | 5.0944 | 0.0944 | 1.019 |
| future_renewal | greedy | 320 | 0 | 0 | 0.3829 | 0.6897 | 0.0000 | 0.138 |
| future_renewal | hs | 2240 | 0 | 0 | 0.3869 | 0.6316 | 0.0000 | 0.126 |
| future_renewal | hs_no_aggregation | 320 | 0 | 0 | 0.1430 | 0.2397 | 0.0000 | 0.048 |
| future_renewal | hs_no_filter | 320 | 0 | 0 | 0.3817 | 0.6361 | 0.0000 | 0.127 |
| future_renewal | lex_simple | 320 | 0 | 0 | 0.4038 | 0.5852 | 0.0000 | 0.117 |
| future_renewal | mcts | 320 | 320 | 320 | 5.0027 | 5.0354 | 0.0354 | 1.007 |
| future_renewal | renewal_exact | 320 | 0 | 0 | 0.0159 | 0.0504 | 0.0000 | 0.010 |
| future_shared | expectimax | 320 | 320 | 320 | 5.0021 | 5.0580 | 0.0580 | 1.012 |
| future_shared | greedy | 320 | 54 | 54 | 4.7139 | 5.0474 | 0.0474 | 1.009 |
| future_shared | hs | 1920 | 0 | 0 | 2.2943 | 3.9291 | 0.0000 | 0.786 |
| future_shared | hs_no_aggregation | 320 | 0 | 0 | 1.7472 | 2.2355 | 0.0000 | 0.447 |
| future_shared | hs_no_filter | 320 | 62 | 62 | 4.7304 | 5.0063 | 0.0063 | 1.001 |
| future_shared | lex_simple | 320 | 131 | 151 | 4.9808 | 5.1184 | 0.1184 | 1.024 |
| future_shared | mcts | 320 | 318 | 318 | 5.0225 | 5.1151 | 0.1151 | 1.023 |


## Interrupção de aprofundamento e cap de avaliações

| Cenário | Regime | Política | Truncadas/N | Compatíveis com cap eval/tempo | Prof. concluída min–max | Prof. visitada máx. | Truncadas sem camada completa (árvores) | MCTS terminou iterações |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| future_deterministic | time5ms | alphabeta | 304/320 | 0/304 | 1–2 | 3 | 0 | 0 |
| future_deterministic | time5ms | expectimax | 304/320 | 0/304 | 1–1 | 2 | 0 | 0 |
| future_deterministic | time5ms | mcts | 320/320 | 0/320 | 0–0 | 20 | 0 | 0 |
| future_deterministic | work1024 | alphabeta | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_deterministic | work1024 | expectimax | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_deterministic | work1024 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |
| future_deterministic | work4096 | alphabeta | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_deterministic | work4096 | expectimax | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_deterministic | work4096 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |
| future_renewal | time5ms | expectimax | 304/320 | 0/304 | 1–1 | 2 | 0 | 0 |
| future_renewal | time5ms | mcts | 320/320 | 0/320 | 0–0 | 20 | 0 | 0 |
| future_renewal | work1024 | expectimax | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_renewal | work1024 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |
| future_renewal | work4096 | expectimax | 304/320 | 304/0 | 1–1 | 2 | 0 | 0 |
| future_renewal | work4096 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |
| future_shared | time5ms | expectimax | 320/320 | 0/320 | 0–0 | 1 | 320 | 0 |
| future_shared | time5ms | mcts | 318/320 | 0/318 | 0–0 | 4 | 0 | 2 |
| future_shared | work1024 | expectimax | 320/320 | 320/0 | 0–0 | 1 | 320 | 0 |
| future_shared | work1024 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |
| future_shared | work4096 | expectimax | 320/320 | 320/0 | 0–0 | 1 | 320 | 0 |
| future_shared | work4096 | mcts | 320/320 | 320/0 | 0–0 | 20 | 0 | 0 |


## Leitura dos diagnósticos

Compatibilidade com o único cap ativo é uma inferência apoiada no código e contadores; a causa individual exata não foi registrada. Zero casos não resolvidos por esses critérios não transforma inferências em eventos instrumentados. MCTS não certifica profundidade completa. Memória esgotada, estouro de pilha e falha de processo não foram observados nestes registros completos. Ver METODOLOGIA.md.

```json
{
  "matches": 2016,
  "decisions": 40320,
  "scaling_decisions": 2688,
  "uncapped_decisions": 119,
  "oracle_decisions": 9,
  "invalid_actions": 0,
  "hs_future_states": 0,
  "evaluation_overruns": 0,
  "unresolved_truncations": 0,
  "time_overruns": 2457,
  "time_overruns_without_flag": 20,
  "max_time_ms": 5.118448,
  "sum_measured_policy_seconds": 1667.848901167,
  "process_rss_min_kib": 26240,
  "process_rss_max_kib": 26692,
  "tables_byte_equal_to_archive": true
}
```

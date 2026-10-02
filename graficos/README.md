# Figuras do experimento preservado

Geradas por `scripts/gerar_graficos.py`, após recalcular e comparar as tabelas com os registros brutos. Cada figura tem PNG e SVG. Não foram alteradas manualmente.

| Figura | Medida e leitura |
| --- | --- |
| [01 Confrontos](01_confrontos.png) | V/E/D HS por rival, cenário e orçamento; inclui todos os resultados |
| [02 Tempo](02_tempo.png) | Mediana por decisão no torneio, milissegundos, eixo logarítmico |
| [03 Avaliações](03_avaliacoes.png) | Média de avaliações completas, eixo logarítmico |
| [04 Sucessores](04_sucessores.png) | Média de transições hipotéticas com repetições; symlog preserva zeros |
| [05 Truncamentos](05_truncamentos.png) | Fração com truncated=true, sem atribuir causa individual não registrada |
| [06 Escalabilidade](06_escalabilidade.png) | Mediana em observação inicial, work4096; fontes, horizonte e ações legais geradas; sete repetições; pode haver truncamento |
| [07 Ablações](07_ablacoes.png) | Controles locais completos sem cap de tempo/avaliações: tempo, avaliações, omissões; qualidade na figura 01 |
| [08 Memória](08_memoria.png) | Máximo de heap próprio rastreado; exclui pilha/runtime e não equivale a RSS |

Cada figura identifica unidades, cenário e protocolo. Métricas online misturam observações visitadas distintas; a escala e as ablações isoladas usam observação comum. As variantes sem dependências/reward_first não têm dados neste último protocolo e não receberam valores fictícios. `06_pontos.json` registra cada ponto desenhado na escala. Nenhuma figura demonstra causalidade, significância ajustada ou superioridade geral.

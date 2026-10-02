# Validação da entrega

Preparação em 2 de outubro de 2026. As políticas C/H/testes são cópias do código medido; não houve otimização nem nova instrumentação.

| Verificação executada | Resultado |
| --- | --- |
| Compilação independente Release, C11, GCC 15.2.0 | Aprovada, avisos tratados como erros |
| CTest `autonomous_agent_properties` | 1/1 suíte aprovada; efeitos compostos, ordem, dependências, agregação, limites, identidade HS, episódios e contraexemplo temporal |
| Reprodução quick na pasta original da entrega | 6 partidas/120 decisões; ações iguais às históricas, sem ilegalidades nem extrapolação de avaliações, zero sucessores HS |
| Cópia para diretório temporário fora do projeto | Compilação + suíte + quick aprovadas novamente; 6 partidas/120 decisões; ações iguais; aproximadamente 22 segundos incluindo build/testes |
| Protocolo regenerado com a cópia | 98 entradas, 48 blocos de sementes e agenda de 2.016 partidas idênticos |
| Leitura completa dos registros de métricas | 40.320 decisões de partidas + 2.688 de escala + 119 locais sem caps + nove de oráculos |
| Reanálise de partidas/métricas | Quatro tabelas CSV recalculadas byte a byte iguais às históricas |
| Conferência da narrativa | 29 valores numéricos principais e todas as 63 células competitivas conferidos; pontos desenhados no eixo de horizonte comparados ao CSV |
| Gráficos | Oito figuras geradas em PNG e SVG a partir dos dados; confrontos, escala e ablações inspecionados visualmente |
| Preservação externa | 1.401 arquivos/links do Derrisor, 420 arquivos dos torneios anteriores e 130 do último experimento verificados contra manifestos, sem mudança |
| Integridade da entrega | Manifesto SHA-256 de todos os arquivos distribuídos, exceto o próprio manifesto |

Evidências desta preparação: [portabilidade.log](dados/validacao_entrega/portabilidade.log), [portabilidade.json](dados/validacao_entrega/portabilidade.json), [quick_original.json](dados/validacao_entrega/quick_original.json), [conferência do relatório](dados/validacao_entrega/conferencia_relatorio.json), [preservação](dados/validacao_entrega/preservacao.json) e [diferença textual de análise](dados/validacao_entrega/script_analysis.diff).

O ambiente de gráficos foi instalado isoladamente fora do pacote, com as versões de `scripts/requirements-graficos.txt`. Os comandos equivalentes do README foram executados; não há dependência desse ambiente temporário. Artefatos de compilação, caches e execuções quick foram retirados da pasta distribuída após a validação, conservando os registros acima. Os comandos de reprodução os recriam.

**Não executados nesta preparação:** torneio integral novo, novos torneios das ablações ausentes, testes reais em Balatro, outras máquinas/compiladores e nova execução de sanitizadores. O runner integral foi preservado e sua geração de entradas/sementes/agenda foi conferida; isso não equivale a ter repetido todas as partidas agora. Os arquivos históricos `build_verification.json` e `verification.json` são evidências anteriores, inclusive de sanitizadores, claramente separados dos testes desta entrega.

As verificações não prometem identidade binária entre plataformas. Igualdade de ações foi observada na mesma família de ambiente, em orçamento fixo de avaliações. Tempo de 5 ms é cooperativo e depende da máquina/carga. Não foram acrescentadas conclusões competitivas com base nas seis partidas de smoke test.

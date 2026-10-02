# H-Suppression — entrega intermediária ao professor Aramuni

Esta pasta independente apresenta o experimento `futures-2026-10-02`. O nome **Aramuny** segue a solicitação. As políticas e o Derrisor não foram modificados. Os resultados são sintéticos e não demonstram superioridade em Balatro.

Leia o [relatório](RELATORIO.md), a [metodologia e auditoria dos limites](METODOLOGIA.md), as [limitações](LIMITACOES.md) e as [tabelas recalculadas](dados/derivados/RESULTADOS.md). O [relatório histórico integral](dados/ultimo_experimento/analysis/results.pt-BR.md) e todas as partidas/decisões estão preservados.

## Organização

```text
Aramuny/
├── README.md, RELATORIO.md, METODOLOGIA.md, LIMITACOES.md, VALIDACAO.md
├── CMakeLists.txt, LICENSE, THIRD_PARTY.md, SHA256SUMS
├── codigo/
│   ├── src/                 engine.c, policies.c, cases.c, driver.c
│   ├── include/agent.h
│   ├── tests/test.c
│   ├── tools/               futures.py, report.py
│   └── scenarios/futures_protocol.json
├── dados/
│   ├── ultimo_experimento/
│   │   ├── inputs/          98 entradas AG1
│   │   ├── analysis/        relatórios históricos PT/EN e tabelas
│   │   └── *.json*, *.gz    registros brutos, agenda, sementes e metadados
│   ├── derivados/          tabelas e auditoria recalculadas
│   ├── validacao_entrega/   evidências dos testes desta entrega
│   └── proveniencia_copia.json
├── graficos/               8 figuras PNG e SVG
├── scripts/                reprodução, auditoria, gráficos, verificação do protocolo
└── referencias/            bibliografia, proveniência e script histórico de medição
```

`build/`, `execucoes/` e ambientes Python são criados pelos comandos e não integram a entrega preservada.

## Requisitos

Linux/POSIX, compilador C11 GCC ou Clang, `libm`, CMake ≥ 3.16, ferramenta de build e Python ≥ 3.10. O runner integral também usa `cc`, `lscpu` (util-linux) e `os.sched_getaffinity`. A referência validada é Linux, GCC 15.2.0 e Python 3.14, build Release `-O3 -DNDEBUG`, avisos `-Wall -Wextra -Wpedantic -Werror`. Não são necessários Balatro, restante do repositório, internet ou bibliotecas Python externas para compilar, testar, reproduzir partidas e recalcular tabelas.

Somente os gráficos exigem Matplotlib. `requirements-graficos.txt` registra as versões usadas, incluindo Matplotlib 3.11.2, validadas com Python 3.14. Em outro Python, pode ser necessário escolher versões compatíveis e registrar a mudança. Instalar essas dependências requer índice de pacotes ou wheels disponíveis; nenhum ambiente virtual está incluído.

## Comandos

Execute **dentro desta pasta**, que pode ser transferida para qualquer caminho:

```bash
sha256sum --check SHA256SUMS
bash scripts/reproduzir.sh quick
```

O modo rápido compila, testa e repete seis partidas históricas: bloco 0, HS contra MCTS, ambos os assentos, três famílias, work1024. Confere legalidade, limite de avaliações, zero sucessores HS e igualdade das ações com a evidência. É verificação, não nova amostra para inferência. As partidas levaram cerca de 20 segundos nesta máquina; reserve um minuto com compilação.

Compilação/testes separados e verificação da geração do protocolo:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python3 scripts/verificar_protocolo.py
```

Reprodução integral, com mesmas sementes, cenários, ordem e parâmetros:

```bash
bash scripts/reproduzir.sh full
```

Executa 2.016 partidas, escala, controles locais sem caps de tempo/avaliações e pequenos oráculos em `execucoes/full-<UTC>-<PID>/`. O runner recusa destino já existente. Nunca sobrescreve `dados/ultimo_experimento/`. Há timeout de 120 segundos por subprocesso; em máquina muito lenta, uma falha será preservada em `failure.json`.

A soma dos tempos C históricos é **1.667,85 segundos (27,8 minutos)**, incluindo medições de escala/oráculos e excluindo aquecimentos, inicialização e análise. Reserve **30–60 minutos** em hardware semelhante: estimativa, não duração de parede histórica instrumentada. O regime de 5 ms depende da máquina/carga. Não se promete identidade binária entre ambientes.

Recalcular tabelas e auditoria:

```bash
python3 scripts/auditar.py
python3 scripts/conferir_relatorio.py
```

Regenerar figuras, recalculando e verificando primeiro as tabelas:

```bash
python3 -m venv .venv-graficos
.venv-graficos/bin/python -m pip install -r scripts/requirements-graficos.txt
.venv-graficos/bin/python scripts/gerar_graficos.py
```

A análise escreve somente em `dados/derivados/` e `graficos/`. Para analisar uma nova execução use `python3 codigo/tools/futures.py --analyze execucoes/<nome>`. As figuras entregues continuam identificadas com a execução histórica. Renderização em outro ambiente pode alterar hashes das imagens sem alterar dados; preserve o manifesto original para comparar. Builds e novas execuções não fazem parte do manifesto.

Sementes concretas: [blocks.json](dados/ultimo_experimento/blocks.json) e [schedule.json](dados/ultimo_experimento/schedule.json). Orçamentos: [protocol.json](dados/ultimo_experimento/protocol.json). Veja [VALIDACAO.md](VALIDACAO.md) para distinguir os testes desta preparação, os testes históricos e a reprodução integral ainda não repetida.

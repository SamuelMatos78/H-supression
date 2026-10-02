# Battles with combinatorial futures

**2,016 matches; 40,320 decisions; zero invalid actions; HS has zero successors.** Seeds swap seats; 95% intervals resample whole pairs 4,000 times. P=(wins+0.5 draws)/games. Exploratory comparisons, without multiple-testing correction.

Degenerate intervals such as [1,1] when every block wins reflect resampling these cases; they do not establish a 100% population win rate.

## Hypothetical history space

| Family | Raw leaves / lower bound | Fixed-policy outcomes | Type |
| --- | --- | --- | --- |
| future_renewal | 2.185e+76 | 1.462e+48 | exact full unmerged tree |
| future_deterministic | 1.462e+48 | 1.000e+00 | exact full unmerged tree |
| future_shared | 1.478e+38 | not stationary | lower bound: no preparations and only subsets of the first four sources; cost <= refresh on every own turn |


These numbers count histories without merging equivalences. They are not distinct states, visited nodes, or operations credited as suppressed by HS. In renewal, per-ply branching sums to Σ C(n,k)2^k=3^n, giving 3^(nH) leaves. A fixed policy activating all eight items for 20 plies has 2^160 elementary outcomes. For shared we claim only a lower bound from an always-legal subtree, not stationary branching for the entire game.

## work1024

| Family | Opponent | HS W/D/L | P [95% CI] | Mean utility margin |
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

| Family | Opponent | HS W/D/L | P [95% CI] | Mean utility margin |
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

| Family | Opponent | HS W/D/L | P [95% CI] | Mean utility margin |
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


## Observed per-decision tournament cost

| Family | Policy | Median µs | Mean evals | Mean successors | Max heap B | Truncated fraction |
| --- | --- | --- | --- | --- | --- | --- |
| future_deterministic | alphabeta | 319.7 | 985.6 | 984.6 | 0 | 0.950 |
| future_deterministic | expectimax | 321.3 | 985.6 | 984.6 | 0 | 0.950 |
| future_deterministic | greedy | 74.5 | 256.0 | 0.0 | 0 | 0.000 |
| future_deterministic | hs | 76.8 | 256.0 | 0.0 | 0 | 0.000 |
| future_deterministic | hs_no_aggregation | 70.1 | 256.0 | 0.0 | 0 | 0.000 |
| future_deterministic | hs_no_filter | 68.8 | 256.0 | 0.0 | 0 | 0.000 |
| future_deterministic | lex_simple | 74.2 | 257.0 | 0.0 | 18432 | 0.000 |
| future_deterministic | mcts | 12302.8 | 1024.0 | 1023.0 | 931840 | 1.000 |
| future_deterministic | renewal_exact | 0.3 | 1.0 | 0.0 | 0 | 0.000 |
| future_renewal | expectimax | 3187.1 | 985.6 | 24776.3 | 0 | 0.950 |
| future_renewal | greedy | 384.7 | 256.0 | 0.0 | 0 | 0.000 |
| future_renewal | hs | 380.2 | 256.0 | 0.0 | 0 | 0.000 |
| future_renewal | hs_no_aggregation | 133.4 | 256.0 | 0.0 | 0 | 0.000 |
| future_renewal | hs_no_filter | 376.3 | 256.0 | 0.0 | 0 | 0.000 |
| future_renewal | lex_simple | 396.6 | 257.0 | 0.0 | 18432 | 0.000 |
| future_renewal | mcts | 13594.0 | 1024.0 | 1023.0 | 919552 | 1.000 |
| future_renewal | renewal_exact | 15.9 | 1.0 | 0.0 | 0 | 0.000 |
| future_shared | expectimax | 1713.8 | 1024.0 | 12055.1 | 0 | 1.000 |
| future_shared | greedy | 860.1 | 1024.0 | 0.0 | 0 | 1.000 |
| future_shared | hs | 1731.0 | 660.7 | 0.0 | 0 | 0.520 |
| future_shared | hs_no_aggregation | 1419.7 | 598.1 | 0.0 | 0 | 0.450 |
| future_shared | hs_no_filter | 850.2 | 1024.0 | 0.0 | 0 | 1.000 |
| future_shared | lex_simple | 862.6 | 1024.0 | 0.0 | 73728 | 1.000 |
| future_shared | mcts | 1008119.6 | 1024.0 | 1023.0 | 696320 | 1.000 |


## Same-observation scaling: eight items

| Family | Rounds/player | Policy | Median µs | Completed depth | Visited depth | Successors |
| --- | --- | --- | --- | --- | --- | --- |
| future_deterministic | 1 | alphabeta | 1209.5 | 1 | 2 | 4095 |
| future_deterministic | 1 | expectimax | 1185.4 | 1 | 2 | 4095 |
| future_deterministic | 1 | hs | 92.8 | 0 | 0 | 0 |
| future_deterministic | 1 | mcts | 13069.8 | 0 | 2 | 4095 |
| future_deterministic | 1 | renewal_exact | 2.0 | 0 | 0 | 0 |
| future_deterministic | 2 | alphabeta | 1213.1 | 1 | 2 | 4095 |
| future_deterministic | 2 | expectimax | 1241.2 | 1 | 2 | 4095 |
| future_deterministic | 2 | hs | 91.3 | 0 | 0 | 0 |
| future_deterministic | 2 | mcts | 38079.0 | 0 | 4 | 4095 |
| future_deterministic | 2 | renewal_exact | 2.3 | 0 | 0 | 0 |
| future_deterministic | 5 | alphabeta | 1173.0 | 1 | 2 | 4095 |
| future_deterministic | 5 | expectimax | 1205.2 | 1 | 2 | 4095 |
| future_deterministic | 5 | hs | 92.0 | 0 | 0 | 0 |
| future_deterministic | 5 | mcts | 51452.2 | 0 | 10 | 4095 |
| future_deterministic | 5 | renewal_exact | 2.3 | 0 | 0 | 0 |
| future_deterministic | 10 | alphabeta | 1227.4 | 1 | 2 | 4095 |
| future_deterministic | 10 | expectimax | 1204.0 | 1 | 2 | 4095 |
| future_deterministic | 10 | hs | 93.8 | 0 | 0 | 0 |
| future_deterministic | 10 | mcts | 53827.6 | 0 | 20 | 4095 |
| future_deterministic | 10 | renewal_exact | 2.1 | 0 | 0 | 0 |
| future_renewal | 1 | expectimax | 13346.2 | 1 | 2 | 104159 |
| future_renewal | 1 | hs | 448.1 | 0 | 0 | 0 |
| future_renewal | 1 | mcts | 19920.8 | 0 | 2 | 4095 |
| future_renewal | 1 | renewal_exact | 21.8 | 0 | 0 | 0 |
| future_renewal | 2 | expectimax | 13106.7 | 1 | 2 | 104159 |
| future_renewal | 2 | hs | 422.7 | 0 | 0 | 0 |
| future_renewal | 2 | mcts | 43428.7 | 0 | 4 | 4095 |
| future_renewal | 2 | renewal_exact | 21.9 | 0 | 0 | 0 |
| future_renewal | 5 | expectimax | 13179.4 | 1 | 2 | 104159 |
| future_renewal | 5 | hs | 420.8 | 0 | 0 | 0 |
| future_renewal | 5 | mcts | 56515.9 | 0 | 10 | 4095 |
| future_renewal | 5 | renewal_exact | 22.0 | 0 | 0 | 0 |
| future_renewal | 10 | expectimax | 13410.6 | 1 | 2 | 104159 |
| future_renewal | 10 | hs | 419.0 | 0 | 0 | 0 |
| future_renewal | 10 | mcts | 58376.2 | 0 | 20 | 4095 |
| future_renewal | 10 | renewal_exact | 22.3 | 0 | 0 | 0 |
| future_shared | 1 | expectimax | 14222.5 | 0 | 1 | 104720 |
| future_shared | 1 | hs | 2315.4 | 0 | 0 | 0 |
| future_shared | 1 | mcts | 2090818.9 | 0 | 2 | 4095 |
| future_shared | 2 | expectimax | 14344.2 | 0 | 1 | 104720 |
| future_shared | 2 | hs | 2329.1 | 0 | 0 | 0 |
| future_shared | 2 | mcts | 3082583.0 | 0 | 4 | 4095 |
| future_shared | 5 | expectimax | 14148.1 | 0 | 1 | 104720 |
| future_shared | 5 | hs | 2335.5 | 0 | 0 | 0 |
| future_shared | 5 | mcts | 3732791.9 | 0 | 10 | 4095 |
| future_shared | 10 | expectimax | 14332.0 | 0 | 1 | 104720 |
| future_shared | 10 | hs | 2370.4 | 0 | 0 | 0 |
| future_shared | 10 | mcts | 4046460.7 | 0 | 20 | 4095 |


## Attribution and limitations

HS retains full evaluation, lexicographic priorities and streaming selection. Its filter skips equivalent additive swaps only in shared; preparation-free cases have no early suppression. Greedy and lex_simple use the same analytical distributions as search and HS. renewal_exact verifies free renewal conditions and activates every nonnegative item: pointwise dominance proves optimality at any horizon. This control is not HS and is rejected for shared. Advantage over naive search therefore does not establish intrinsic difficulty or a unique suppression advantage.

Expectimax requests horizon 20 and retains its last completed layer. Alpha-beta prunes in the deterministic family; it is not applied to chance games. MCTS samples trajectories and never needs all leaves: losses may reflect limited samples, expensive action generation or noisy uniform rollouts. These are simple reference implementations; transpositions, dynamic programming or separability recognition can change the result.

Equal evaluation counts do not equal instructions. The 5 ms limit is cooperative; observed overruns are retained. Owned heap excludes stack; process RSS can inherit the launcher high-water mark. Renewal deliberately favors local decisions; shared adds contested stock, limited energy and permanent effects. Terminal utility matches the previous tournaments. Complete outcomes, including losses, remain available below.

[Matchups](matchups.csv) · [decision metrics](decisions.csv) · [all scaling data](scaling.csv) · [uncapped local controls](uncapped.csv) · [history counts](history_counts.json) · [all outcomes](all_outcomes.json) · [protocol](../protocol.json) · [metadata](../environment.json) · [raw games](../games.jsonl) · [raw decisions](../decisions.jsonl.gz) · [exact small oracle checks](../oracle_checks.jsonl).

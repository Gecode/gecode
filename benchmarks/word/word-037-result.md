# Word-037 comparison campaign

External result root: `/private/tmp/gecode-word-037-reproducible.MnFzfv`
Git revision: `a8311ee183f69dfc80ae1753ca8e0b09c5fd8974`
Image: `localhost/gecode-word037-runtime:a8311ee-final2` (`c04206132c027d58c0e5dab3b6f13e6de1c4f48fceb568670bcb54dcc17113be`)
Limits: `{"cpu_seconds": 21600, "cpus": 1, "memory": 4294967296, "memory_swap": 4294967296, "network": "none"}`
Options: `{"followup_timeout": 300, "repeats": 5, "screen_timeout": 30, "tiny_min_seconds": 0.25}`
Calibration selection: `{"bounded-alu": "split-min", "crc-xorshift": "msb", "dma": "split-min", "inverse": "split-min", "reduced-speck": "msb", "register": "split-min"}`

Screen accounting: 288/288 cells; deferred=0, measured=216, unsupported=72. CPU ledger: 228.17/21600 seconds.

Gecode search counters and SMT statuses are retained separately; they are not treated as the same work metric.

## Screen timing groups

These groups use only the single screen run for each measured matrix cell.

| Family | Status | Configuration | n | Median s | Range s |
|---|---|---|---:|---:|---:|
| bounded-alu | sat | gecode-baseline | 6 | 1.58691e-05 | 8.8501e-06–3.17383e-05 |
| bounded-alu | sat | gecode-candidate | 6 | 2.41089e-05 | 1.34277e-05–5.85937e-05 |
| bounded-alu | sat | z3 | 6 | 0.00664063 | 0.00296875–0.010625 |
| bounded-alu | unsat | gecode-baseline | 6 | 1.15967e-05 | 8.23975e-06–2.31934e-05 |
| bounded-alu | unsat | gecode-candidate | 6 | 1.7395e-05 | 1.0376e-05–5.00488e-05 |
| bounded-alu | unsat | z3 | 6 | 0.0059375 | 0.00320313–0.0121875 |
| crc-xorshift | sat | gecode-baseline | 6 | 2.63214e-06 | 1.14441e-06–1.78528e-05 |
| crc-xorshift | sat | gecode-candidate | 6 | 2.70844e-06 | 1.06812e-06–1.15967e-05 |
| crc-xorshift | sat | z3 | 6 | 0.00546875 | 0.001875–0.0115625 |
| crc-xorshift | unsat | gecode-baseline | 6 | 3.7384e-06 | 1.06812e-06–6.71387e-06 |
| crc-xorshift | unsat | gecode-candidate | 6 | 4.34875e-06 | 1.06812e-06–8.54492e-06 |
| crc-xorshift | unsat | z3 | 6 | 0.00453125 | 0.00171875–0.125312 |
| dma | sat | gecode-baseline | 6 | 0.000263672 | 4.15039e-05–0.00144531 |
| dma | sat | gecode-candidate | 6 | 0.000244141 | 4.02832e-05–0.00136719 |
| dma | sat | z3 | 6 | 0.0053125 | 0.0040625–0.0078125 |
| dma | unsat | gecode-baseline | 6 | 0 | 0–0 |
| dma | unsat | gecode-candidate | 6 | 0 | 0–0 |
| dma | unsat | z3 | 6 | 0.00296875 | 0.00257813–0.0053125 |
| inverse | sat | gecode-baseline | 6 | 8.8501e-06 | 6.10352e-07–0.00012085 |
| inverse | sat | gecode-candidate | 6 | 9.61304e-06 | 6.10352e-07–8.30078e-05 |
| inverse | sat | z3 | 6 | 0.00460938 | 0.00121094–0.0078125 |
| inverse | unsat | gecode-baseline | 6 | 3.05176e-07 | 3.05176e-07–3.05176e-07 |
| inverse | unsat | gecode-candidate | 6 | 3.05176e-07 | 3.05176e-07–3.05176e-07 |
| inverse | unsat | z3 | 6 | 0.00195312 | 0.00109375–0.0034375 |
| reduced-speck | sat | gecode-baseline | 6 | 8.69751e-06 | 4.11987e-06–6.5918e-05 |
| reduced-speck | sat | gecode-candidate | 6 | 2.48718e-05 | 3.96729e-06–0.000141602 |
| reduced-speck | sat | z3 | 6 | 0.00625 | 0.00242187–0.01875 |
| reduced-speck | unsat | gecode-baseline | 6 | 1.02234e-05 | 3.43323e-06–8.78906e-05 |
| reduced-speck | unsat | gecode-candidate | 6 | 1.04523e-05 | 3.50952e-06–0.000244141 |
| reduced-speck | unsat | z3 | 6 | 0.00664062 | 0.00234375–0.0625 |
| register | sat | gecode-baseline | 6 | 0.000107422 | 9.15527e-06–0.00128906 |
| register | sat | gecode-candidate | 6 | 0.000111084 | 9.15527e-06–0.00140625 |
| register | sat | z3 | 6 | 0.00226563 | 0.00183594–0.00390625 |
| register | unsat | gecode-baseline | 6 | 2.81525e-05 | 1.14441e-06–6.83594e-05 |
| register | unsat | gecode-candidate | 6 | 2.75421e-05 | 1.2207e-06–6.83594e-05 |
| register | unsat | z3 | 6 | 0.00433594 | 0.00183594–0.00671875 |

## Broad screen ranges

- bounded-alu: gecode-candidate paired median 0.675× (range 0.172–1.86); loss versus baseline.
- bounded-alu: z3 paired median 0.00239× (range 0.000948–0.00843); loss versus baseline.
- crc-xorshift: gecode-candidate paired median 0.988× (range 0.643–3.08); loss versus baseline.
- crc-xorshift: z3 paired median 0.000636× (range 2.19e-05–0.0022); loss versus baseline.
- dma: gecode-candidate paired median 1.06× (range 1.03–1.38); win versus baseline.
- dma: z3 paired median 0.0391× (range 0.0102–0.264); loss versus baseline.
- inverse: gecode-candidate paired median 1× (range 0.862–3.81); parity versus baseline.
- inverse: z3 paired median 0.000279× (range 8.88e-05–0.0998); loss versus baseline.
- reduced-speck: gecode-candidate paired median 0.91× (range 0.246–1.15); loss versus baseline.
- reduced-speck: z3 paired median 0.0017× (range 0.00102–0.00502); loss versus baseline.
- register: gecode-candidate paired median 1× (range 0.917–1.04); parity versus baseline.
- register: z3 paired median 0.00985× (range 0.000533–0.33); loss versus baseline.

## Resource and completion metrics

Peak RSS uses GNU time's per-process maximum resident set in KiB.

| Configuration | Maximum RSS KiB |
|---|---:|
| gecode-baseline | 64412 |
| gecode-candidate | 60448 |
| z3 | 45560 |
| bitwuzla | unavailable |

Timeout-aware counts use screen rows only.

| Configuration | Measured | Timeout | Memory limit | Unknown | Error | Unsupported | Deferred |
|---|---:|---:|---:|---:|---:|---:|---:|
| gecode-baseline | 72 | 0 | 0 | 0 | 0 | 0 | 0 |
| gecode-candidate | 72 | 0 | 0 | 0 | 0 | 0 | 0 |
| z3 | 72 | 0 | 0 | 0 | 0 | 0 | 0 |
| bitwuzla | 0 | 0 | 0 | 0 | 0 | 72 | 0 |

## Five-repeat selected cases

### alu-large-sat-a (sat)

- gecode-baseline: n=5, median 7.32422e-06s, range 7.32422e-06–1.09863e-05s.
- gecode-candidate: n=5, median 2.68555e-05s, range 2.56348e-05–3.05176e-05s.
- z3: n=5, median 0.0065625s, range 0.00625–0.009375s.
- baseline/gecode-candidate: n=5, median 0.286×, range 0.261–0.36; loss (stable).
- baseline/z3: n=5, median 0.00112×, range 0.00107–0.00117; loss (stable).

### crc-xorshift-large-xor-sat (sat)

- gecode-baseline: n=5, median 5.49316e-06s, range 5.0354e-06–8.8501e-06s.
- gecode-candidate: n=5, median 5.64575e-06s, range 5.0354e-06–9.46045e-06s.
- z3: n=5, median 0.0075s, range 0.0065625–0.0142188s.
- baseline/gecode-candidate: n=5, median 0.935×, range 0.644–1; loss.
- baseline/z3: n=5, median 0.000671×, range 0.000427–0.000818; loss (stable).

### dma-large-sat-b (sat)

- gecode-baseline: n=5, median 0.00152344s, range 0.00148438–0.0025s.
- gecode-candidate: n=5, median 0.00140625s, range 0.00136719–0.00261719s.
- z3: n=5, median 0.00578125s, range 0.00546875–0.0132813s.
- baseline/gecode-candidate: n=5, median 1.08×, range 0.955–1.29; win.
- baseline/z3: n=5, median 0.264×, range 0.188–0.321; loss (stable).

### inverse-small-sat-2 (sat)

- gecode-baseline: n=5, median 4.11987e-06s, range 3.96729e-06–9.91821e-06s.
- gecode-candidate: n=5, median 2.21252e-06s, range 2.13623e-06–4.65393e-06s.
- z3: n=5, median 0.009375s, range 0.008125–0.01875s.
- baseline/gecode-candidate: n=5, median 1.86×, range 1.79–2.13; win (stable).
- baseline/z3: n=5, median 0.000507×, range 0.000439–0.000553; loss (stable).

### register-large-allocation-sat (sat)

- gecode-baseline: n=5, median 0.00136719s, range 0.00132813–0.00269531s.
- gecode-candidate: n=5, median 0.00136719s, range 0.00132813–0.00238281s.
- z3: n=5, median 0.00296875s, range 0.00289062–0.00710938s.
- baseline/gecode-candidate: n=5, median 1.03×, range 0.557–1.19; win.
- baseline/z3: n=5, median 0.459×, range 0.379–0.474; loss (stable).

### speck-large-unsat-a (unsat)

- gecode-baseline: n=5, median 3.17383e-05s, range 3.17383e-05–4.39453e-05s.
- gecode-candidate: n=5, median 3.2959e-05s, range 3.05176e-05–5.24902e-05s.
- z3: n=5, median 0.009375s, range 0.009375–0.0175s.
- baseline/gecode-candidate: n=5, median 0.963×, range 0.837–1.04; loss.
- baseline/z3: n=5, median 0.00339×, range 0.00251–0.00339; loss (stable).

## Data-derived priorities

- alu-large-sat-a: candidate is a stable loss versus baseline (0.286× baseline/candidate, 0.261–0.36)
- inverse-small-sat-2: candidate is a stable win versus baseline (1.86× baseline/candidate, 1.79–2.13)

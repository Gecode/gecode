# Word-037 comparison campaign

External result root: `/private/tmp/gecode-word-037-local.Z23NJu`
Git revision: `f29ee69530f5230d507c2605823919afdafd7165`
Runner: `native subprocess` on `Darwin 25.6.0 arm64`
Limits: `{"campaign_budget_seconds": 21600, "cpu_affinity": "host default", "memory": "host default; RSS not sampled", "wall_seconds": {"followup": 300, "screen": 30}}`
Options: `{"repeats": 5, "tiny_min_seconds": 0.25}`
Calibration selection: `{"bounded-alu": "split-min", "crc-xorshift": "msb", "dma": "split-min", "inverse": "split-min", "reduced-speck": "msb", "register": "split-min"}`

Screen accounting: 288/288 cells; deferred=0, measured=216, unsupported=72. Wall-time ledger: 245.32/21600 seconds.

All solver processes ran directly on the host. CPU affinity and memory were left at the macOS defaults; wall caps bound individual runs. Per-process RSS is unavailable in this mode.

Gecode search counters and SMT statuses are retained separately; they are not treated as the same work metric.

## Screen timing groups

These groups use only the single screen run for each measured matrix cell.

| Family | Status | Configuration | n | Median s | Range s |
|---|---|---|---:|---:|---:|
| bounded-alu | sat | gecode-baseline | 6 | 8.00826e-06 | 7.16607e-06–1.15565e-05 |
| bounded-alu | sat | gecode-candidate | 6 | 1.80364e-05 | 9.6107e-06–2.48613e-05 |
| bounded-alu | sat | z3 | 6 | 0.00209009 | 0.00199632–0.00222367 |
| bounded-alu | unsat | gecode-baseline | 6 | 1.11553e-05 | 7.98936e-06–1.2978e-05 |
| bounded-alu | unsat | gecode-candidate | 6 | 1.756e-05 | 9.24377e-06–3.14277e-05 |
| bounded-alu | unsat | z3 | 6 | 0.00207229 | 0.00195465–0.00219687 |
| crc-xorshift | sat | gecode-baseline | 6 | 2.98998e-06 | 1.3703e-06–5.85234e-06 |
| crc-xorshift | sat | gecode-candidate | 6 | 2.98059e-06 | 1.27376e-06–5.85968e-06 |
| crc-xorshift | sat | z3 | 6 | 0.00529214 | 0.00163152–0.00638209 |
| crc-xorshift | unsat | gecode-baseline | 6 | 3.0176e-06 | 1.26197e-06–5.78409e-06 |
| crc-xorshift | unsat | gecode-candidate | 6 | 2.97462e-06 | 1.3434e-06–5.80681e-06 |
| crc-xorshift | unsat | z3 | 6 | 0.00542698 | 0.00156445–0.00711647 |
| dma | sat | gecode-baseline | 6 | 0.000230594 | 3.9387e-05–0.00127723 |
| dma | sat | gecode-candidate | 6 | 0.00021726 | 3.79907e-05–0.00124787 |
| dma | sat | z3 | 6 | 0.00404219 | 0.00344085–0.00526377 |
| dma | unsat | gecode-baseline | 6 | 6.55828e-08 | 5.92995e-08–7.51095e-08 |
| dma | unsat | gecode-candidate | 6 | 6.47389e-08 | 6.13556e-08–7.73462e-08 |
| dma | unsat | z3 | 6 | 0.00298116 | 0.00232797–0.00500873 |
| inverse | sat | gecode-baseline | 6 | 9.71296e-06 | 7.88758e-07–8.34103e-05 |
| inverse | sat | gecode-candidate | 6 | 9.39123e-06 | 8.11254e-07–9.76144e-05 |
| inverse | sat | z3 | 6 | 0.00254878 | 0.00092992–0.00341947 |
| inverse | unsat | gecode-baseline | 6 | 5.36699e-07 | 5.14109e-07–5.51665e-07 |
| inverse | unsat | gecode-candidate | 6 | 5.32165e-07 | 5.11466e-07–5.56115e-07 |
| inverse | unsat | z3 | 6 | 0.00165248 | 0.000918711–0.00263161 |
| reduced-speck | sat | gecode-baseline | 6 | 5.83927e-06 | 3.38872e-06–3.2075e-05 |
| reduced-speck | sat | gecode-candidate | 6 | 5.86877e-06 | 3.34903e-06–5.49798e-05 |
| reduced-speck | sat | z3 | 6 | 0.0032121 | 0.00190545–0.00631107 |
| reduced-speck | unsat | gecode-baseline | 6 | 4.92181e-06 | 2.94629e-06–3.13779e-05 |
| reduced-speck | unsat | gecode-candidate | 6 | 4.8704e-06 | 2.98037e-06–4.45734e-05 |
| reduced-speck | unsat | z3 | 6 | 0.00321433 | 0.00190099–0.00645629 |
| register | sat | gecode-baseline | 6 | 0.000395581 | 1.00332e-05–0.00791362 |
| register | sat | gecode-candidate | 6 | 0.000112293 | 9.8772e-06–0.00132816 |
| register | sat | z3 | 6 | 0.00207354 | 0.00166581–0.00292761 |
| register | unsat | gecode-baseline | 6 | 2.92291e-05 | 1.29655e-06–7.23967e-05 |
| register | unsat | gecode-candidate | 6 | 2.76104e-05 | 1.2802e-06–7.39058e-05 |
| register | unsat | z3 | 6 | 0.00407771 | 0.00174769–0.00736473 |

## Broad screen ranges

- bounded-alu: gecode-candidate paired median 0.635× (range 0.288–0.883); loss versus baseline.
- bounded-alu: z3 paired median 0.00434× (range 0.00322–0.00601); loss versus baseline.
- crc-xorshift: gecode-candidate paired median 0.992× (range 0.939–1.08); loss versus baseline.
- crc-xorshift: z3 paired median 0.000878× (range 0.000465–0.00107); loss versus baseline.
- dma: gecode-candidate paired median 1.04× (range 0.882–1.08); win versus baseline.
- dma: z3 paired median 0.00574× (range 1.38e-05–0.243); loss versus baseline.
- inverse: gecode-candidate paired median 0.992× (range 0.854–1.82); loss versus baseline.
- inverse: z3 paired median 0.000581× (range 0.000206–0.0406); loss versus baseline.
- reduced-speck: gecode-candidate paired median 0.996× (range 0.583–1.05); loss versus baseline.
- reduced-speck: z3 paired median 0.00174× (range 0.00151–0.00508); loss versus baseline.
- register: gecode-candidate paired median 1.01× (range 0.98–67.5); win versus baseline.
- register: z3 paired median 0.0101× (range 0.000638–4.75); loss versus baseline.

## Resource and completion metrics

Per-process peak RSS is unavailable in direct local mode.

| Configuration | Maximum RSS KiB |
|---|---:|
| gecode-baseline | unavailable |
| gecode-candidate | unavailable |
| z3 | unavailable |
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

- gecode-baseline: n=5, median 7.26038e-06s, range 7.16323e-06–7.55873e-06s.
- gecode-candidate: n=5, median 2.4802e-05s, range 2.47703e-05–2.4927e-05s.
- z3: n=5, median 0.00219911s, range 0.00217211–0.00225213s.
- baseline/gecode-candidate: n=5, median 0.292×, range 0.289–0.305; loss (stable).
- baseline/z3: n=5, median 0.00329×, range 0.00323–0.00344; loss (stable).

### crc-xorshift-large-xor-sat (sat)

- gecode-baseline: n=5, median 5.77763e-06s, range 5.74377e-06–5.95305e-06s.
- gecode-candidate: n=5, median 5.80674e-06s, range 5.75013e-06–5.85363e-06s.
- z3: n=5, median 0.0061171s, range 0.00599175–0.0062708s.
- baseline/gecode-candidate: n=5, median 0.997×, range 0.989–1.02; loss.
- baseline/z3: n=5, median 0.000957×, range 0.000925–0.000971; loss (stable).

### dma-large-sat-b (sat)

- gecode-baseline: n=5, median 0.00127212s, range 0.00126622–0.00127737s.
- gecode-candidate: n=5, median 0.00119646s, range 0.00118732–0.00120382s.
- z3: n=5, median 0.00484024s, range 0.00478117–0.00489118s.
- baseline/gecode-candidate: n=5, median 1.06×, range 1.06–1.07; win (stable).
- baseline/z3: n=5, median 0.263×, range 0.259–0.267; loss (stable).

### inverse-small-sat-2 (sat)

- gecode-baseline: n=5, median 4.05042e-06s, range 4.01499e-06–4.0889e-06s.
- gecode-candidate: n=5, median 2.3351e-06s, range 2.32168e-06–2.37333e-06s.
- z3: n=5, median 0.00323366s, range 0.00315935–0.00329581s.
- baseline/gecode-candidate: n=5, median 1.73×, range 1.71–1.76; win (stable).
- baseline/z3: n=5, median 0.00125×, range 0.00123–0.00128; loss (stable).

### register-large-allocation-sat (sat)

- gecode-baseline: n=5, median 0.00124382s, range 0.00124183–0.00126471s.
- gecode-candidate: n=5, median 0.00125329s, range 0.00124787–0.00125746s.
- z3: n=5, median 0.00253661s, range 0.00251897–0.00258054s.
- baseline/gecode-candidate: n=5, median 0.992×, range 0.988–1.01; loss.
- baseline/z3: n=5, median 0.491×, range 0.487–0.496; loss (stable).

### speck-large-unsat-a (unsat)

- gecode-baseline: n=5, median 2.63855e-05s, range 2.63509e-05–2.6509e-05s.
- gecode-candidate: n=5, median 2.49496e-05s, range 2.47082e-05–2.514e-05s.
- z3: n=5, median 0.00638435s, range 0.00626482–0.00647898s.
- baseline/gecode-candidate: n=5, median 1.06×, range 1.05–1.07; win (stable).
- baseline/z3: n=5, median 0.00414×, range 0.00407–0.00421; loss (stable).

## Data-derived priorities

- alu-large-sat-a: candidate is a stable loss versus baseline (0.292× baseline/candidate, 0.289–0.305)
- inverse-small-sat-2: candidate is a stable win versus baseline (1.73× baseline/candidate, 1.71–1.76)

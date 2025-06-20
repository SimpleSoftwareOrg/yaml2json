| Command | Mean [ms] | Min [ms] | Max [ms] | Relative |
|:---|---:|---:|---:|---:|
| `../build/yaml2json small_117kb.yaml` | 3.1 ± 0.7 | 2.4 | 5.0 | 1.00 |
| `yq -o json small_117kb.yaml` | 22.8 ± 0.5 | 21.7 | 23.5 | 7.34 ± 1.58 |
| `./lq . small_117kb.yaml` | 20.4 ± 0.4 | 19.8 | 21.3 | 6.59 ± 1.42 |

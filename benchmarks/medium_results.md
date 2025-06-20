| Command | Mean [ms] | Min [ms] | Max [ms] | Relative |
|:---|---:|---:|---:|---:|
| `../build/yaml2json medium_1mb.yaml` | 12.2 ± 1.0 | 11.4 | 16.1 | 1.00 |
| `yq -o json medium_1mb.yaml` | 168.9 ± 2.9 | 165.2 | 176.9 | 13.79 ± 1.17 |
| `./lq . medium_1mb.yaml` | 150.0 ± 20.8 | 143.1 | 237.6 | 12.25 ± 1.98 |

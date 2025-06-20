| Command | Mean [ms] | Min [ms] | Max [ms] | Relative |
|:---|---:|---:|---:|---:|
| `../build/yaml2json large_6_5mb.yaml` | 68.4 ± 0.4 | 67.8 | 69.2 | 1.00 |
| `yq -o json large_6_5mb.yaml` | 992.0 ± 45.4 | 966.1 | 1182.3 | 14.51 ± 0.67 |
| `./lq . large_6_5mb.yaml` | 898.4 ± 25.1 | 885.0 | 992.3 | 13.14 ± 0.37 |

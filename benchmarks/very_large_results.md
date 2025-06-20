| Command | Mean [ms] | Min [ms] | Max [ms] | Relative |
|:---|---:|---:|---:|---:|
| `../build/yaml2json very_large_13mb.yaml` | 140.4 ± 2.7 | 138.6 | 150.2 | 1.00 |
| `yq -o json very_large_13mb.yaml` | 2078.2 ± 63.0 | 2009.1 | 2226.0 | 14.80 ± 0.53 |
| `./lq . very_large_13mb.yaml` | 1855.6 ± 47.6 | 1824.1 | 2008.1 | 13.21 ± 0.42 |

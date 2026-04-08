Tables 1 to 4 have been generated with `appmc/gen_tables_md.py`.
The script depends on `pandas`, `numpy`, and `mpmath`.

Table 5 has been generated with `approxmc/accuracy_md.py`
, and Table 6 has been generated with `approxmc/runtime_md.py`.

The raw CSV files are available in the respective directories.

The `urs` directory contains the raw results of the uniformity tests
performed on `cppddnnf/build/sampler` (here called `ksampler`) and `cppddnnf/build/rsampler`.

# Datasets

## Table 1: Dataset Summary
|Dataset             | #F<sub>total</sub> | min(&#124;Var(F)&#124;) | max(&#124;Var(F)&#124;) | min(&#124;F&#124;) | max(&#124;F&#124;)|
|------------------- | ------------------ | ----------------------- | ----------------------- | ------------------ | ------------------|
|Global              | 4656               | 0                       | 8286433                 | 0                  | 7689680           |
|↪ Lagniez           | 1979               | 5                       | 229100                  | 10                 | 399794            |
|↪↪ Bayesian Network | 1116               | 32                      | 229100                  | 38                 | 399794            |
|↪↪ BMC              | 18                 | 762                     | 63624                   | 2469               | 368352            |
|↪↪ Circuit          | 68                 | 26                      | 8704                    | 66                 | 83902             |
|↪↪ Configuration    | 35                 | 1400                    | 2038                    | 1698               | 11342             |
|↪↪ Handmade         | 68                 | 61                      | 3176                    | 254                | 174160            |
|↪↪ Planning         | 557                | 5                       | 24816                   | 10                 | 148891            |
|↪↪ QIF              | 7                  | 251                     | 4473                    | 452                | 14011             |
|↪↪ Random           | 104                | 75                      | 150                     | 150                | 525               |
|↪↪ Scheduling       | 6                  | 19500                   | 22500                   | 103887             | 123329            |
|↪ Soos              | 1896               | 2                       | 8286433                 | 1                  | 7689680           |
|↪ Plazar            | 503                | 14                      | 486193                  | 31                 | 2598178           |
|↪↪ Blasted Real     | 210                | 14                      | 10329                   | 35                 | 33008             |
|↪↪ Feature Models   | 133                | 45                      | 62482                   | 104                | 343944            |
|↪↪ V15              | 30                 | 17                      | 25615                   | 43                 | 57946             |
|↪↪ V3               | 30                 | 17                      | 25528                   | 31                 | 57586             |
|↪↪ V7               | 30                 | 17                      | 25546                   | 35                 | 57662             |
|↪↪ unsorted         | 70                 | 67                      | 486193                  | 66                 | 2598178           |
|↪ Sundermann        | 278                | 0                       | 31012                   | 0                  | 350120            |
# Knowledge Compilation
## Table 2: Experimental results regarding the scalability of DivKC
|Dataset             | #F<sub>total</sub> | #D4 and not DivKC | #not D4 | #DivKC and not D4|
|------------------- | ------------------ | ----------------- | ------- | -----------------|
|Global              | 4656               | 643               | 672     | 114              |
|↪ Lagniez           | 1979               | 256               | 214     | 54               |
|↪↪ Bayesian Network | 1116               | 127               | 70      | 53               |
|↪↪ BMC              | 18                 | 12                | 2       | 0                |
|↪↪ Circuit          | 68                 | 2                 | 8       | 0                |
|↪↪ Configuration    | 35                 | 11                | 1       | 0                |
|↪↪ Handmade         | 68                 | 1                 | 32      | 0                |
|↪↪ Planning         | 557                | 102               | 92      | 1                |
|↪↪ QIF              | 7                  | 1                 | 1       | 0                |
|↪↪ Random           | 104                | 0                 | 2       | 0                |
|↪↪ Scheduling       | 6                  | 0                 | 6       | 0                |
|↪ Soos              | 1896               | 311               | 394     | 54               |
|↪ Plazar            | 503                | 58                | 61      | 6                |
|↪↪ Blasted Real     | 210                | 7                 | 30      | 3                |
|↪↪ Feature Models   | 133                | 13                | 5       | 0                |
|↪↪ V15              | 30                 | 2                 | 5       | 1                |
|↪↪ V3               | 30                 | 2                 | 5       | 1                |
|↪↪ V7               | 30                 | 2                 | 5       | 1                |
|↪↪ unsorted         | 70                 | 32                | 11      | 0                |
|↪ Sundermann        | 278                | 18                | 3       | 0                |
# Approximate Model Counting
## Table 3: Experimental results for Algorithm 3
|Dataset             | #F   | Y<sub>l</sub> &le; &#124;R<sub>F</sub>&#124; | Y<sub>h</sub> &ge; &#124;R<sub>F</sub>&#124; | Coverage | &#124;R<sub>G<sub>P</sub></sub>&#124; &le; &#124;R<sub>F</sub>&#124; &le; &#124;R<sub>G<sub>U</sub></sub>&#124; |
|------------------- | ---- | -------------------------------------------- | -------------------------------------------- | -------- | ----------------------------------------------------------------------------------------------------------------|
|Global              | 3341 | 0.988                                        | 0.869                                        | 0.857    | 1.000                                                                                                           |
|↪ Lagniez           | 1509 | 0.993                                        | 0.914                                        | 0.907    | 1.000                                                                                                           |
|↪↪ Bayesian Network | 919  | 0.992                                        | 0.905                                        | 0.898    | 1.000                                                                                                           |
|↪↪ BMC              | 4    | 1.000                                        | 1.000                                        | 1.000    | 1.000                                                                                                           |
|↪↪ Circuit          | 58   | 1.000                                        | 0.862                                        | 0.862    | 1.000                                                                                                           |
|↪↪ Configuration    | 23   | 1.000                                        | 0.696                                        | 0.696    | 1.000                                                                                                           |
|↪↪ Handmade         | 35   | 1.000                                        | 1.000                                        | 1.000    | 1.000                                                                                                           |
|↪↪ Planning         | 363  | 0.989                                        | 0.934                                        | 0.923    | 1.000                                                                                                           |
|↪↪ QIF              | 5    | 1.000                                        | 1.000                                        | 1.000    | 1.000                                                                                                           |
|↪↪ Random           | 102  | 1.000                                        | 0.961                                        | 0.961    | 1.000                                                                                                           |
|↪↪ Scheduling       | 0    |                                              |                                              |          |                                                                                                                 |
|↪ Soos              | 1191 | 0.981                                        | 0.915                                        | 0.896    | 1.000                                                                                                           |
|↪ Plazar            | 384  | 0.987                                        | 0.815                                        | 0.802    | 1.000                                                                                                           |
|↪↪ Blasted Real     | 173  | 0.994                                        | 0.971                                        | 0.965    | 1.000                                                                                                           |
|↪↪ Feature Models   | 115  | 1.000                                        | 0.478                                        | 0.478    | 1.000                                                                                                           |
|↪↪ V15              | 23   | 1.000                                        | 1.000                                        | 1.000    | 1.000                                                                                                           |
|↪↪ V3               | 23   | 1.000                                        | 1.000                                        | 1.000    | 1.000                                                                                                           |
|↪↪ V7               | 23   | 1.000                                        | 0.957                                        | 0.957    | 1.000                                                                                                           |
|↪↪ unsorted         | 27   | 0.852                                        | 0.815                                        | 0.667    | 1.000                                                                                                           |
|↪ Sundermann        | 257  | 0.996                                        | 0.475                                        | 0.471    | 1.000                                                                                                           |
## Table 4: Experimental results comparing the bounds obtained with Algorithm 3 and with Lemma 1
|Dataset             | #F   | Y<sub>l</sub> &ge; &#124;R<sub>G<sub>P</sub></sub>&#124; | Y<sub>h</sub> &le; &#124;R<sub>G<sub>U</sub></sub>&#124; | Both  | median(r<sub>c</sub>) | max(r<sub>c</sub>)|
|------------------- | ---- | -------------------------------------------------------- | -------------------------------------------------------- | ----- | --------------------- | ------------------|
|Global              | 3341 | 0.834                                                    | 0.869                                                    | 0.752 | 9.58e-9               | 1.0               |
|↪ Lagniez           | 1509 | 0.868                                                    | 0.913                                                    | 0.801 | 7.53e-9               | 1.0               |
|↪↪ Bayesian Network | 919  | 0.958                                                    | 0.905                                                    | 0.867 | 1.62e-6               | 0.0761            |
|↪↪ BMC              | 4    | 1.000                                                    | 1.000                                                    | 1.000 | 0.0                   | 6.67e-5           |
|↪↪ Circuit          | 58   | 0.914                                                    | 0.862                                                    | 0.828 | 1.87e-29              | 8.88e-8           |
|↪↪ Configuration    | 23   | 0.522                                                    | 0.652                                                    | 0.478 | 2.58e-6               | 1.0               |
|↪↪ Handmade         | 35   | 1.000                                                    | 1.000                                                    | 1.000 | 0.0                   | 0.0               |
|↪↪ Planning         | 363  | 0.887                                                    | 0.934                                                    | 0.851 | 3.63e-42              | 0.0103            |
|↪↪ QIF              | 5    | 0.400                                                    | 1.000                                                    | 0.400 | 2.91e-57              | 8.9e-6            |
|↪↪ Random           | 102  | 0.020                                                    | 0.961                                                    | 0.020 | 1.85e-68              | 2.01e-23          |
|↪↪ Scheduling       | 0    |                                                          |                                                          |       |                       |                   |
|↪ Soos              | 1191 | 0.940                                                    | 0.915                                                    | 0.863 | 2.55e-7               | 0.085             |
|↪ Plazar            | 384  | 0.763                                                    | 0.815                                                    | 0.656 | 6.61e-14              | 0.0137            |
|↪↪ Blasted Real     | 173  | 0.942                                                    | 0.971                                                    | 0.913 | 1.06e-13              | 0.0137            |
|↪↪ Feature Models   | 115  | 0.348                                                    | 0.478                                                    | 0.087 | 1.99e-13              | 0.00868           |
|↪↪ V15              | 23   | 1.000                                                    | 1.000                                                    | 1.000 | 1.37e-16              | 0.00121           |
|↪↪ V3               | 23   | 0.957                                                    | 1.000                                                    | 0.957 | 6.68e-17              | 0.000663          |
|↪↪ V7               | 23   | 1.000                                                    | 0.957                                                    | 0.957 | 2.57e-17              | 0.00365           |
|↪↪ unsorted         | 27   | 0.815                                                    | 0.815                                                    | 0.630 | 4.37e-44              | 8.86e-7           |
|↪ Sundermann        | 257  | 0.249                                                    | 0.471                                                    | 0.089 | 1.6e-13               | 0.126             |

## Table 5: Experimental results comparing the accuracy of Algorithm 3 with ApproxMC7
|Dataset|#F|l &le; Y<sub>ApproxMC</sub> &le; h|l &le; Y &le; h|
|-|-|-|-|
| Global | 2782 | 0.977 | 0.881 |
| ↪ Lagniez  | 1386 | 1.000 | 0.882 |
| ↪↪ Bayesian Network | 915 | 1.000 | 0.854 |
| ↪↪ BMC | 4 | 1.000 | 1.000 |
| ↪↪ Circuit | 50 | 1.000 | 0.740 |
| ↪↪ Configuration | 6 | 1.000 | 0.833 |
| ↪↪ Handmade | 15 | 1.000 | 1.000 |
| ↪↪ Planning | 289 | 1.000 | 0.952 |
| ↪↪ QIF | 5 | 1.000 | 1.000 |
| ↪↪ Random | 102 | 1.000 | 0.990 |
| ↪↪ Scheduling | 0 | - | - |
| ↪ Soos  | 1180 | 0.970 | 0.873 |
| ↪ Plazar  | 203 | 0.862 | 0.921 |
| ↪↪ Blasted Real | 172 | 1.000 | 0.930 |
| ↪↪ Feature Models | 3 | 1.000 | 1.000 |
| ↪↪ V15 | 0 | - | - |
| ↪↪ V3 | 0 | - | - |
| ↪↪ V7 | 0 | - | - |
| ↪↪ unsorted | 28 | 0.000 | 0.857 |
| ↪ Sundermann  | 13 | 1.000 | 0.846 |
## Table 6: Experimental results comparing the runtime of Algorithm 3 with ApproxMC7 for 20 run
|Dataset|#F|#DivKC|log<sub>10</sub>(min)|mean|median|log<sub>10</sub>(max)|
|-|-|-|-|-|-|-|
| Global | 2888 | 2265 |  -3.2 | 124.5 |   4.8 |   5.3 |
| ↪ Lagniez  | 1436 | 1179 |  -2.9 |  80.5 |   5.5 |   4.4 |
| ↪↪ Bayesian Network | 964 | 782 |  -2.8 |  32.8 |   5.0 |   4.4 |
| ↪↪ BMC | 4 | 0 |  -2.8 | - | - | - |
| ↪↪ Circuit | 50 | 34 |  -2.6 |   3.5 |   2.5 |   1.3 |
| ↪↪ Configuration | 6 | 6 |   1.2 | 515.1 |  90.6 |   3.3 |
| ↪↪ Handmade | 15 | 15 |   0.0 | 121.2 |   9.9 |   3.1 |
| ↪↪ Planning | 290 | 237 |  -2.9 | 236.4 |   5.7 |   4.1 |
| ↪↪ QIF | 5 | 4 |  -0.1 |   7.3 |   7.4 |   1.2 |
| ↪↪ Random | 102 | 101 |  -0.2 | 100.4 |  68.2 |   2.5 |
| ↪↪ Scheduling | 0 | 0 |  -0.2 | - | - | - |
| ↪ Soos  | 1235 | 953 |  -3.0 |  29.6 |   4.5 |   4.4 |
| ↪ Plazar  | 204 | 122 |  -3.2 |   2.7 |   1.7 |   1.5 |
| ↪↪ Blasted Real | 173 | 113 |  -2.3 |   2.8 |   2.6 |   1.0 |
| ↪↪ Feature Models | 3 | 3 |   0.4 |  13.1 |   7.6 |   1.5 |
| ↪↪ V15 | 0 | 0 |   0.4 | - | - | - |
| ↪↪ V3 | 0 | 0 |   0.4 | - | - | - |
| ↪↪ V7 | 0 | 0 |   0.4 | - | - | - |
| ↪↪ unsorted | 28 | 6 |  -3.2 |   0.6 |   0.5 |   0.3 |
| ↪ Sundermann  | 13 | 11 |  -0.6 | 15904.5 |  29.6 |   5.3 |

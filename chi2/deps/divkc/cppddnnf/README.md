# Projection

## Dependencies

On Ubuntu and Debian-based systems:
```
sudo apt install ninja-build libboost-dev libgmp-dev libboost-random-dev libboost-program-options-dev g++
```

## Building

The `build.ninja` file is generated with:
```
g++ gen.cpp -o gen
./gen
```
or if the `build.ninja` file already exists:
```
ninja generate
```

To build the executable please run:
```
ninja
```

This will create multiple executables, namely:
```
build/appmc
build/lmc
build/pc
build/rhsampler
build/rsampler
build/sampler
```

In the paper we only used `build/appmc` for the approximate model counter
and `build/sampelr` for the sampling algorithm.

To only build those you can use
```
ninja build/appmc build/sampler
```

## Usage

### Appmc

The executable used for approximate model counting is `build/appmc`.
Runnin `build/appmc --help` gives:
```
Allowed options:
  -h [ --help ]                       Display help message
  --verbose                           Display all of the intermediate estimates
                                      as well
  --cnf arg                           path to CNF file
  --alpha arg (=0.01)                 alpha value for the CLT
  --nb arg (=10000)                   the maximum number of samples to use
  --lnb arg (=0)                      the minimum number of samples to use, set
                                      to 0 to disable early stopping (see 
                                      --epsilon)
  --epsilon arg (=1.1000000000000001) the algorithm stops early (before --nb 
                                      samples have been reached) if (Y / 
                                      epsilon) <= Yl and (Y * epsilon) >= Yh
```

The `cnf` argument is the path to the CNF file.
The `alpha` parameter is the parameter used by the central limit theorem to compute
the confidence interval and `nb` is the maximum number of samples to use before the algorithm
stops.

By default, early stopping is disabled and the algorithm will use the maximum number of samples
as given to `--nb`.
If early stopping is desired, the option `--lnb` controls the minimum number of samples to use
before considering early stopping. If it is set to a non-zero value, then early stopping is enabled.
The `--epsilon` option controls early stoppping.
The model counter estimates the model coutn `Y` and uses the central limit theorem to estimate
upper (`Yh`) and lower (`Yl`) bounds.
If early stopping is enabled then the algorithm stop early if the number of samples is greater
than `--lnb` and if `(Y / epsilon) <= Yl` and `(Y * epsilon >= Yh`.

By default, the model counter only display the last estimate (and in case early stopping is enabled,
the last estimate given by each thread of execution).
If you wish to have all the estimates, you may use the `--verbose` option.

This executable assumes that if the `cnf` parameter is set to `t.cnf`, there will be
files called `t.cnf.proj.log`, `t.cnf.pnnf` for the projected formula (`t.cnf.p`) compiled to d-DNNF
and `t.cnf.unnf` for the upper bound (`t.cnf.pup`) compiled to d-DNNF.
These files can be generated with the help of the version of `D4` shipped in this repository
and the `projection` executable.

The current implementation requires our modified version of `D4` to function.
Therefore, to replace `D4` by a different compiler, either the compiler
or the implementation in `cppddnnf` has to be modified.

### Sampler

The executable used for sampling is `build/sampler`.
Runnin `build/sampler --help` gives:
```
Allowed options:
  -h [ --help ]         Display help message
  --cnf arg             path to CNF file
  --k arg (=50)         buffer size to use
  --nb arg (=10)        the number of solutions to generate
```

The `cnf` argument is the path to the CNF file.
The `k` parameter is the buffer size which is to be used.
Larger values mean that the sampler will be close to uniformity.
However, sampling will be more expensive.
The `nb` parameter indicates the desired number of solutions.

This executable assumes that if the `cnf` parameter is set to `t.cnf`, there will be
files called `t.cnf.log`, `t.cnf.pnnf` for the projected formula (`t.cnf.p`) compiled to d-DNNF
and `t.cnf.unnf` for the upper bound (`t.cnf.pup`) compiled to d-DNNF.
These files can be generated with the help of the version of `D4` shipped in this repository
and the `projection` executable.

The current implementation requires our modified version of `D4` to function.
Therefore, to replace `D4` by a different compiler, either the compiler
or the implementation in `cppddnnf` has to be modified.

### RSampler

`build/rsampler` is similar to `build/sampler` but it uses reservoir sampling (and is not
presented in the paper).
We therefore do not present theorems for the bounds of uniformity of `rsampler`.
However, `rsampler` is much more sampler efficient than `sampler` and can therefore be
used with much larger buffer sizes than `sampler`.
This larger buffer size also allows it to be an exact uniform random sampler
if the number of partial orthoghonal assignments to represent the projected formula
`t.cnf.p` is smaller than the buffer size.
To test `sampler` we had to use buffer sizes of 50 to execute the statistical unifomity
tests in reasonable time. To test `rsampler` we used buffer sizes of 50000 instead.

The usage and requirements of `rsampler` are the same as for `sampler`.

Here are the raw test results for both samplers as executed for the paper:
(The bold p-values indicate which tests the sampler passed, i.e.
the p-value is greater than 0.01.)

| |sampler| | |rsampler| | |
|-|-|-|-|-|-|-|
|Test           | #F  | p-value | Time (h) | #F  | p-value | Time (h) |
|VF             | 176 | 0.000   | 26.862   | 176 | 0.000   | 12.896   |
|Birthday       | 140 | 0.005   | 40.611   | 139 | 0.000   | 23.282   |
|SFpC           |  77 | 0.000   | 49.026   |  82 | **0.196** | 34.427   |
|GOF            |  65 | 0.000   | 40.851   |  70 | **0.185** | 25.985   |
|Modbit q = 2   | 178 | **0.199** | 3.124  | 178 | **0.127** | 0.842    |
|Modbit q = 4   | 178 | **0.144** | 3.565  | 178 | **0.124** | 0.877    |
|Modbit q = 8   | 178 | **0.129** | 3.205  | 178 | **0.141** | 0.919    |
|Modbit q = 16  | 178 | 0.000   | 3.291    | 178 | **0.206** | 0.947    |
|Modbit q = 32  | 178 | 0.000   | 8.699    | 178 | **0.277** | 4.598    |
|Modbit q = 64  | 126 | 0.000   | 63.311   | 132 | **0.170** | 44.348   |
|Modbit q = 128 |  83 | 0.000   | 60.208   |  88 | 0.000     | 49.047   |


We can definetly see that `rsampler` passes more tests. Moreover,
for the Modbit test with `q = 8` we see that `sampler` needed 3 hours while `rsampler`
only needed 1 hour.

However, it is important to remember that because `rsampler` uses much larger buffer
sizes, it also uses the exact algorithm much more often. This is even more important
to remember here because the `omega` dataset contains smaller formulae that were used
to keep testing execution times reasonable. Therefore, while `rsampler` is very likely
to be more uniform and faster than `sampler`, the test results regarding `rsampler` in the
above table are probably misleading and optimistic.

## Apptainer

It is also possible to use Apptainer (Singularity) instead of using a native builds.
Simply run
```
apptainer build --fakeroot appmc.sif appmc.def
apptainer build --fakeroot sampler.sif sampler.def
apptainer build --fakeroot rsampler.sif rsampler.def
```
to create the containers.
You may then use the containers just like the native executable presented above.

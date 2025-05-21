# Testing Uniform Random Samplers: Methods, Datasets and Protocols

The `chi2` folder contains the statistical tests as well as the scripts
necessary to build the singularity container that will contain the tests
to run as well as the samplers under test.

The `datasets` folder contains various DIMACS files.

The `utils` folder contains the python d-DNNF parser that is used by the statistical
tests as well as the code necessary to generate our synthetic datasets.
`rsat` can be used to generate random k-CNF formulae
and `blast` can be used to blast random variable assignments into an existing
formula.

The `results` contains various python scripts used to generate the results
in the paper as well as the csv files obtained during our experiments.

## Testing a Sampler

```
cd chi2
make
./chi2.sif --test <statistical test> \
    --min_elem_per_cell <integer (default 10)> \
    -b <batch size (default 1000)> \
    -c <path to cnf file> \
    -s <sampler to test> \
    -a <significance level (usually 0.01)>
```

The available values for `--test` are
```
monobit
modbit
freq_var
sfpc
chisquared
birthday
```

The `modbit` test has an aditional parameter `--modbit_q <integer>`.

The `birthday` test has an additional parameter `--bday_prob <float>`. If this
value is smaller than `1.0` then it is interpreted as the probability to observe a repetition.
If it is greater than `1.0` then it is interpreted as the desired expected number of repetitions
under the null hypothesis. This parameter influences the required sample size.

The available values for `-s` are
```
unigen3
spur
sts
smarch
quicksampler
cmsgen
kus
distaware
walksat
bddsampler
```

## Adding a Sampler

To add a sampler, please modify the `src/samplers.py` file.
You will need to write a function
`getSolutionsFromSampler(inputFile : str, numSolutions : int, newSeed : int) : [frozenset(int)]`
with `inputFile` the path to the DIMACS file, `numSolutions` the requested sample size
and `newSeed` the random seed given to the function (this can increase reproducibility
as well as reduce isses especially if the random number generator of the sampler
is initialized with the current time).
The function is expected to return a list of solutions to the formula.
Each solution is expected to be a frozenset of integers with each integer being a literal
in the usual DIMACS format.

Finally, please modify the function `getSamplerFunction` which should ideally
remain at the end of the file.
This function takes as input the string given to the program via the option
`-s` and returns the appropriate `getSolutionsFromSampler` function depending on the
string.

If your sampler has been installed within the container (preferrably base.def
once the installation is stable enough), you should be able to test your sampler.

## Adding a Test

To add a statistical test, please modify the `src/tests.py` file.
You will need to write a function that performs the statistical test
and takes as input an object of type `Settings` as defined in the
`src/util.py` file.
The `Settings` class contains various parameters given to the program
such as batch size, the sampler function to be used (`Settings.sampler_fn`)
as well as the d-DNNF representing the formula under test.

Once you have added the necessary function, you should add an entry
to the `getTestFunction(str)` which should preferable remain at the end of the file.
This function takes as input a string given to the program via the option `--test`
and returns the appropriate `perform_statistical_test(Settings)` function.

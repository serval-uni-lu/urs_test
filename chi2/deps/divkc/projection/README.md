# Projection

## Dependencies

On Ubuntu and Debian-based systems:
```
sudo apt install ninja-build libboost-program-options-dev g++
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

This will create an executable called `build/projection`

By running `./build/projection --help` you get:

```
Allowed options:
  -h [ --help ]         Display help message
  --cnf arg             path to CNF file
  --timeout arg (=3600) timeout in seconds
```

The `cnf` argument expects a path to a CNF file in DIMACS format.
The `timeout` argument expects a positive integer which is the maximum time in seconds (default is one hour).

Running the program on a cnf file as follows:
```
./build/projection --cnf t.cnf
```
creates multiple files.
The file `t.cnf.p` contains the projected formula, or if the execution was too long,
the partially projected formula. The `t.cnf.pup` file contains an upper bound of the
formula `t.cnf` as described in the paper. The file `t.cnf.log` contains the final projection,
i.e., the set of variables on which `t.cnf` was actually projected to produce
`t.cnf.p` (and as a consequence `t.cnf.pup`).

## Apptainer

It is also possible to use Apptainer (Singularity) instead of using a native build.
Simply run `apptainer build --fakeroot projection.sif projection.def` to create the container.
You may then use the container `projection.sif` just like the native executable presented above.

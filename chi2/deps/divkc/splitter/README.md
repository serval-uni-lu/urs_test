# Splitter

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

This will create an executable called `build/splitter`

By running `./build/splitter --help` you get:

```
Allowed options:
  -h [ --help ]         Display help message
  --cnf arg             path to CNF file
  --nb arg (=2)         the target number of partitions (default:2)
```

The `cnf` argument expects a path to a CNF file in DIMACS format.
The `nb` argument expects a positive integer which is used by the hypergraph partitioner
to determine how many partitions are to be created by the partitioner (the default is set to 2).

Running the program on a cnf file as follows:
```
./build/splitter --cnf t.cnf
```
returns the line `c p show 21 25 28 0` which is to be included in the cnf file passed to the projection executable (in this repository).

## Apptainer

It is also possible to use Apptainer (Singularity) instead of using a native build.
Simply run `apptainer build --fakeroot splitter.sif splitter.def` to create the container.
You may then use the container `splitter.sif` just like the native executable presented above.

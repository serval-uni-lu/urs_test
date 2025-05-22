Use `make` to compile.

Dependencies can be installed on ubuntu with
`apt install libz3-dev make g++ libbsd-dev pkg-config`.

Otherwise, you may use `make singularity` to build the singularity container.

Basic usage:

`./blast <path to cnf> <N : int> <simple | imbalance>`

`N` is the number of solutions to generate and blast into the formula.
If the last parameter is `imbalance`, then for each variable assignment `a`,
for each literal `l \in a`, an assignment `tmp = (a - {l}) U {~l}` is created
and blasted into the formula as well.

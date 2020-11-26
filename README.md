# PIP-algorithms
Parallel In-Place Algorithms

# Run The Tests

To build the project and run all the tests, simply type

```bash
make test
```

Or type the following commands to run a single test case separately

```bash
make test_scan
make test_filter
make test_listRanking
make test_listRanking3
...
```

The full available list can be found by

```bash
make list
```

To cleanup the intermediate files, use the command

```bash
make clean
```

To adjust the input arguments for each test, please modify the corresponding files in the `test/` directory.
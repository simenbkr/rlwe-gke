# RLWE-GAKE

This repository contains the code from my master thesis (TTM4905) at NTNU - "Evaluating Post-Quantum Group Key Exchange".

The structure in this repository is the following:
```
- create_charts | The code used to generate some figures, as well as the raw data.
- gke | Implementation of three different group key exchanges, two of which are post-quantum.
- gake | Implementation of three different group authenticated key exchanges, two of which are post-quantum
- measure-funcs | Source code for speed-testing certain interesting functions.
- simple_msg | Implementation of simple prototype.
```

## Building stuff
To build a binary, cd into the relevant directory, then:
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Of course, you would need the relevant libraries - dgs, mpfr, gmp - installed to support static compilation. The included bare-bones Dockerfile should give an idea of how to get this working.

## Thesis
Should appear at some point at [NTNU Open](https://ntnuopen.ntnu.no/ntnu-xmlui/), probably, maybe.

### Citation:
```
@MASTERTHESIS{simenbkr-eval-pq-gke,
	author = "Simen Been Kristiansen",
	title = "{E}valuating {P}ost-{Q}uantum {G}roup {K}ey {E}xchange",
	school = "{D}epartment of {I}nformation {S}ecurity and {C}ommunication {T}echnology, {NTNU} -- {N}orwegian {U}niversity of {S}cience and {T}echnology",
	Month = "June",
	year = "2020"
```

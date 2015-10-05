# Alchemy 2 - Inference and Learning in Markov Logic
This is a fork of the code located at https://code.google.com/p/alchemy-2/.

Features added in this fork:
* Inference using Gurobi ILP solver.

## Dependencies
* g++ 4.1.2
* Bison 2.3
* Flex 2.5.4
* Perl 5.8.8

You can install these using Homebrew on Mac or apt-get on Ubuntu.

###On OSX
You just need to install gcc and perl. Bison and Flex must be present already.
```
$ brew install gcc perl518
```

###On Ubuntu
Do not install Bison through apt-get as it will install the latest version (v3.0.2). Instead, build it manually.
```
$ wget http://ftp.gnu.org/gnu/bison/bison-2.7.tar.gz
$ tar xfz bison-2.7.tar.gz
$ cd bison-2.7
$ ./configure
$ make
$ sudo make install
```

Flex and Perl can be installed through apt-get.
```
$ sudo apt-get install flex perl
```

### Gurobi
Gurobi is a commercial Integer Linear Program solver which demonstrates very high computational speeds.

Installation: [OSX instructions](http://www.gurobi.com/documentation/5.6/quick-start-guide/installation_mac_os), [Linux instructions](http://www.gurobi.com/documentation/5.6/quick-start-guide/installation_linux)

You also need to obtain a license (Academic or otherwise) to be able to use it.

## Building
On OSX
```
$ cd path/to/alchemy-2/
$ mkdir bin/obj
$ cd src
$ cp makefile_osx makefile
```

On Linux
```
$ cd path/to/alchemy-2/
$ mkdir bin/obj
$ cd src
$ cp makefile_linux makefile
```

Modify the makefile to set the Gurobi directory if it is not installed in the default places.

```
$ make all -j4
```

## Usage
To perform inference with Gurobi, pass the flags "-a -ilp" with the command line arguments. For example,
```
$ ./bin/infer -i exdata/voting-gen.mln -e exdata/voting-test.db -r exdata/voting-test-ilp.out -q Democrat -a -ilp
```

You will need to execute `learnwts.sh` to generate `voting-gen.mln`

## References
The conversion of MAP inference in MLNs to an Integer Linear Program is based on
```
@inproceedings{noessner2013rockit,
  title={RockIt: Exploiting Parallelism and Symmetry for MAP Inference in Statistical Relational Models.},
  author={Noessner, Jan and Niepert, Mathias and Stuckenschmidt, Heiner},
  booktitle={AAAI Workshop: Statistical Relational Artificial Intelligence.}
  year={2013}
}
```

## License
By using Alchemy, you agree to accept the license agreement in license.txt

src/ contains source code and a makefile.
doc/ contains a change log, and a manual in PDF, PostScript and html formats.
exdata/ contains a simple example of Alchemy input files.
bin/ is used to contain compiled executables.

Please refer to the change log at http://alchemy.cs.washington.edu/
for the latest changes to Alchemy.

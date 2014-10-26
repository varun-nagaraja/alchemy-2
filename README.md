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

On OSX 
```
$ brew install gcc bison flex perl518
```

On Ubuntu
```
$ sudo apt-get install bison flex perl
```

### Gurobi
Gurobi is a commercial Integer Linear Program solver which demonstrates very high computational speeds.

Installation: [OSX instructions](http://www.gurobi.com/documentation/5.6/quick-start-guide/installation_mac_os), [Linux instructions](http://www.gurobi.com/documentation/5.6/quick-start-guide/installation_linux)

You also need to obtain a license (Academic or otherwise) to be able to use it.

## Building
On OSX
```
$ cd path/to/alchemy-2/src
$ cp makefile_osx makefile
```

On Linux
```
$ cd path/to/alchemy-2/src
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

## License
By using Alchemy, you agree to accept the license agreement in license.txt

src/ contains source code and a makefile.
doc/ contains a change log, and a manual in PDF, PostScript and html formats.
exdata/ contains a simple example of Alchemy input files.
bin/ is used to contain compiled executables.

Please refer to the change log at http://alchemy.cs.washington.edu/
for the latest changes to Alchemy.

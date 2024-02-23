# MiniballSort

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7867978.svg)](https://doi.org/10.5281/zenodo.7867978)

Sort code for Miniball data using the new FEBEX/MIDAS DAQ from the 2022 campaign onwards. Also now updated to take data from the MBS DAQ.

There is a [Wiki](https://github.com/Miniball/MiniballSort/wiki) page for this code, giving some more detailed instructions of how to analyse data.

Please cite the code using the DOI if you use it for your publications: [doi:10.5281/zenodo.7867979](http://doi.org/10.5281/zenodo.7867978).

## Download

```bash
git clone https://github.com/Miniball/MiniballSort
```

## Compile

```bash
make clean
make
```


## Execute

```
mb_sort
```
if you add the MiniballSort/bin to your PATH variable. You can also add MiniballSort/lib to your (DY)LD_LIBRARY_PATH too.

or
```
./bin/mb_sort
```

The input options are described below

```
use mb_sort with following flags:
[-i      <vector<string>>: List of input files]
[-o      <string        >: Output file for histogram file]
[-s      <string        >: Settings file]
[-c      <string        >: Calibration file]
[-r      <string        >: Reaction file]
[-f                      : Flag to force new ROOT conversion]
[-e                      : Flag to force new event builder (new calibration)]
[-source                 : Flag to define an source only run]
[-ebis                   : Flag to define an EBIS only run, discarding data >4ms after an EBIS event]
[-mbs                    : Flag to define input as MBS data type]
[-spy                    : Flag to run the DataSpy]
[-m      <int           >: Monitor input file every X seconds]
[-p      <int           >: Port number for web server (default 8030)]
[-d      <string        >: Directory to put the sorted data default is /path/to/data/sorted]
[-g                      : Launch the GUI]
[-h                      : Print this help]
```

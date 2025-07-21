# MiniballSort

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7867978.svg)](https://doi.org/10.5281/zenodo.7867978)

Sort code for Miniball data using the new FEBEX/MIDAS DAQ from the 2022 campaign onwards.

This sort code has been updated to take data from the MBS/FEBEX DAQ (.lmd files - 2022) and the MBS/DGF DAQ (.med files - 2001-2018).
Not all features of the old DAQ have yet been implemented, but if you have an old data set that you would like to sort, make a request with the "issues" feature and we will try to implement your requirement.

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
        [-i         <vector<string>>: List of input files]
        [-o         <string        >: Output file for histogram file]
        [-s         <string        >: Settings file]
        [-c         <string        >: Calibration file]
        [-r         <string        >: Reaction file]
        [-f                         : Flag to force new ROOT conversion]
        [-e                         : Flag to force new event builder (new calibration)]
        [-source                    : Flag to define an source only run]
        [-ebis                      : Flag to define an EBIS only run, discarding data >4ms after an EBIS event]
        [-midas                     : Flag to define input as MIDAS data type (FEBEX with Daresbury firmware - default)]
        [-mbs                       : Flag to define input as MBS data type (FEBEX with GSI firmware)]
        [-med                       : Flag to define input as MED data type (DGF and MADC)]
        [-anglefit                  : Flag to run the angle fit]
        [-angledata <string        >: File containing 22Ne segment energies]
        [-spy                       : Flag to run the DataSpy]
        [-m         <int           >: Monitor input file every X seconds]
        [-p         <int           >: Port number for web server (default 8030)]
        [-d         <string        >: Directory to put the sorted data default is /path/to/data/sorted]
        [-g                         : Launch the GUI]
        [-h                         : Print this help]
```


## Dependencies

You will need to have ROOT installed with a minumum standard that your compiler supports C++14.
At the moment it works with v5 or v6, but let me know of any problems by raising an issue on GitHub.
ROOT must be built with GSL library support, otherwise known as MathMore.
To check this is true, you can type: `root-config --has-mathmore` and hope the response is `yes`.
If it isn't, you will need to install the GSL libraries and reconfigure/rebuild ROOT.

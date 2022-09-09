# MiniballSort

Sort code for Miniball data using the new FEBEX/MIDAS DAQ from the 2022 campaign onwards. Also now updated to take data from the MBS DAQ.

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
	[-mbs                    : Flag to define input as MBS data type]
	[-spy                    : Flag to run the DataSpy]
	[-m      <int           >: Monitor input file every X seconds]
	[-p      <int           >: Port number for web server (default 8030)]
	[-d      <string        >: Data directory to add to the monitor]
	[-g                      : Launch the GUI]
	[-h                      : Print this help]
```

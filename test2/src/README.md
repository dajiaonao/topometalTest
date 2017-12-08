Instructions
------------

## How to compile
``` make lib```

## How to run
``` ./runTest.py data_list.txt```

Format of data_list.txt:
``` > cat datalist.txt 
/data/Samples/xRayPol/20171107/out135.pd1
/data/Samples/xRayPol/20171107/out136.pd1
```
The background file can be given in the ./runTest.py file.

The output files are in figs/ directory, including a .eps version and .png version. A text file listing the signals in each file are also generated there.


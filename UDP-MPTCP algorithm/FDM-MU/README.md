## How to compile & run

For original version of FDM-MU

```bash
make run
```

For Hub version of FDM-MU

```bash
make runhub
```



You need to config your configuration file for each version, what a configuration file does it to tell the program the directory of `input` file and `output` file. A sample configuration file is as following



```reStructuredText
testcase_um_9_MPTCP.txt
alloc_hub.txt
```



The first line is the directory of input file, and the second line is the directory of output file.

When you use `make` to run your program, you need to use specific config  **file name** for each of two versions.

The configuration file name for original version is `in.txt`

The configuration file name for hub version is `in_hub.txt`


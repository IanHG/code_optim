Google-benchmark
===============================================================================

Installation
-------------------------------------------------------------------------------

You can download the `google-benchmark` source code from

```sh
https://github.com/google/benchmark.git
```

Google-benchmark uses `CMake`, so it should be easy to build:
```sh
$ cd benchmark
$ mkdir build
$ cd build
$ cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<path_to_install> ../.
$ make
$ make install
```

Perf
===============================================================================

Installation
-------------------------------------------------------------------------------

On Ubuntu `perf` is part of the `linux-tools` package. Installation on Ubuntu:
```sh
$ sudo apt-get install linux-tools-common linux-tools-generic linux-tools-`uname -r`
``` 

Usage
-------------------------------------------------------------------------------

To do performance statistics we can use the `stat` command/tool of `perf`:
```sh
$ perf stat <executable> <executable-args>
```

We can get more detailed output with the `-d` option, which can be chained for even more output,
*e.g.*:
```sh
$ perf stat -d -d -d <executable> <executable-args>
```

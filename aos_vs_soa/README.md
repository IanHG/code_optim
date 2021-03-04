Introduction
-------------------------------------------------------------------------------

In this tutorial, we will look at how the layout of your data may tremendously
affect the performance of your code depending on your data layout and data access patterns.
Specifically we will look at what is called **Array-Of-Structs (AoS)**,
and compare it to its counterpart **Structs-Of-Arrays (SoA)**.
In doing so we will see how the design principles of an **Object-Oriented-Programming (OOP)** approach
can sometimes hinder the performance of a program if applied in a non-optimal way for your data.
Instead we will see that for some common access patterns employing a **Data-Oriented-Design (DOD)** driven 
approach (I use the term loosely here) can vastly increase the performance of our code.
The reason data layout is becoming more and more important is because of modern HardWare (HW) architecture,
which is tuned massively for what is known as **Single-Instruction Multiple-Data (SIMD)** operations,
and the fact that nowadays **Central-Processing-Units (CPU)** are many times faster than **Random-Access-Memory (RAM)**,
which makes getting new data from RAM to CPU a relatively expensive operation, in terms of CPU-cycles.
As data is always moved in full cache-lines (`64` bytes on many systems), 
this makes it vital to utilize all data on each cache-line
for maximum performance.
This need to optimize data access patterns is a trend 
that will only continue and magnify in the coming years.

Array-Of-Structs (AoS) vs. Struct-Of-Arrays (SoA)
-------------------------------------------------------------------------------

In an Array-Of-Structs (also loosely called the Object-Oriented-Programming (OOP)-approach)
we declare a `struct`, and if we need more than one instance we create a vector
of the `struct` type:
```cpp
// AoS
struct data
{
   int i0;
   int i1;
};

// Array of structures
std::vector<data> aos;

...

// Access i0 of first object
int i0_first = aos[0].i0;
```

In a Struct-Of-Arrays (also loosely known as Data-Oriented-Design (DOD)),
we instead create a `struct` holding all our objects, where each member variable
is held in contigous memory, *e.g.* a vector.
```cpp
// SoA
struct SoA
{
   std::vector<int> i0s;
   std::vector<int> i1s;
};

// Structure of arrays
SoA soa;

...

// Access i0 of first "data structure"
int i0_first = soa.i0s[0];
```

Finding cache-line size (coherency line size)
-------------------------------------------------------------------------------

Information in all CPU cache's are for each CPU located in:
```sh
/sys/devices/system/cpu/cpu0/cache/
```
Here will be a directory for each cache level, e.g. `L1`, `L2`, etc..
Looking in the folder for the `L1` we find some files:
```sh
$ ls /sys/devices/system/cpu/cpu0/cache/index0
coherency_line_size
id
level
number_of_sets
physical_line_partition
shared_cpu_list
shared_cpu_map
size
type
uevent
ways_of_associativity
```
The file we want to look at is the `coherency_line_size`.
On my system, which is `x86_64` (like many systems), it is `64` bytes (which is `512` bits):
```sh
$ cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size 
64
```

Another way is to use the `getconf`-tool:
```sh
$ getconf LEVEL1_DCACHE_LINESIZE
64
```
This also return `64` bytes, of course.
You can see all configuration variables with `getconf -a`.
For more informantion see:
```sh
$ man getconf
```

In C/C++ you can also get `L1` cache-line size with the `sysconf(3)` function from `unistd.h` header.
```cpp
#include <unistd.h>
#include <stdio.h>

int main()
{
   long l1_size = sysconf (_SC_LEVEL1_DCACHE_LINESIZE);
   printf("%d\n", l1_size);
   return 0;
}
```
For more information on `sysconf(3)` see:
```sh
man sysconf
```

Using `perf` - Simple profiling
-------------------------------------------------------------------------------

Run `perf` where we zoom in on `Benchmark_Aos/16`:
```sh
$ sudo perf stat -d -d -d ./main.x --benchmark_filter=Benchmark_Aos/16 --benchmark_min_time=3
```

The output from this command will look something like this:
```sh
2021-03-03T10:44:56+01:00
Running ./main.x
Run on (8 X 3400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 0.75, 0.63, 0.51
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------------
Benchmark                 Time             CPU   Iterations
-----------------------------------------------------------
Benchmark_Aos/16     123927 ns       123552 ns        33825

 Performance counter stats for './main.x --benchmark_filter=Benchmark_Aos/16 --benchmark_min_time=3':

          5,602.97 msec task-clock                #    0.994 CPUs utilized          
               858      context-switches          #    0.153 K/sec                  
                 0      cpu-migrations            #    0.000 K/sec                  
             2,183      page-faults               #    0.390 K/sec                  
    18,847,329,320      cycles                    #    3.364 GHz                      (30.61%)
    11,819,954,843      instructions              #    0.63  insn per cycle           (38.36%)
     2,959,981,032      branches                  #  528.288 M/sec                    (38.37%)
           251,498      branch-misses             #    0.01% of all branches          (38.40%)
     2,959,975,662      L1-dcache-loads           #  528.287 M/sec                    (38.47%)
     2,950,509,424      L1-dcache-load-misses     #   99.68% of all L1-dcache hits    (38.50%)
     1,372,686,342      LLC-loads                 #  244.993 M/sec                    (30.82%)
        98,910,371      LLC-load-misses           #    7.21% of all LL-cache hits     (30.82%)
   <not supported>      L1-icache-loads                                             
         1,728,130      L1-icache-load-misses                                         (30.82%)
     2,945,040,730      dTLB-loads                #  525.621 M/sec                    (30.89%)
            94,885      dTLB-load-misses          #    0.00% of all dTLB cache hits   (30.82%)
             9,025      iTLB-loads                #    0.002 M/sec                    (30.78%)
             3,709      iTLB-load-misses          #   41.10% of all iTLB cache hits   (30.71%)
   <not supported>      L1-dcache-prefetches                                        
   <not supported>      L1-dcache-prefetch-misses                                   

       5.638270490 seconds time elapsed

       5.579227000 seconds user
       0.024048000 seconds sys


```
Here we clearly see a lot (over $99\%$!) of `L1-dcache-load-misses`.
The `L1d` is the Level-1 data-cache (different from the `L1i`, which is the Level-1 instruction-cache).
This means that most if not all of the time, the CPU needs to get the data it needs from a higher level cache or RAM,
which in the end means that we take a huge hit to our performance.
The reason we see this many cache misses is that we essentially take a cache miss
for each object we update, as the CPU has to load a new cache-line for each.

Now lets try running `perf` on the `Benchmark_Soa/16` test case:
```sh
$ sudo perf stat -d -d -d ./main.x --benchmark_filter=Benchmark_Soa/16 --benchmark_min_time=3
```

We will get something like the following:
```sh
2021-03-03T10:46:22+01:00
Running ./main.x
Run on (8 X 3400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 0.29, 0.53, 0.49
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------------
Benchmark                 Time             CPU   Iterations
-----------------------------------------------------------
Benchmark_Soa/16       9976 ns         9954 ns       429623

 Performance counter stats for './main.x --benchmark_filter=Benchmark_Soa/16 --benchmark_min_time=3':

          5,377.54 msec task-clock                #    0.994 CPUs utilized          
               854      context-switches          #    0.159 K/sec                  
                 0      cpu-migrations            #    0.000 K/sec                  
             7,164      page-faults               #    0.001 M/sec                  
    18,211,257,249      cycles                    #    3.387 GHz                      (30.76%)
    53,193,684,770      instructions              #    2.92  insn per cycle           (38.55%)
     8,857,025,482      branches                  # 1647.041 M/sec                    (38.70%)
           757,289      branch-misses             #    0.01% of all branches          (38.70%)
    17,695,463,053      L1-dcache-loads           # 3290.625 M/sec                    (38.55%)
     2,212,203,272      L1-dcache-load-misses     #   12.50% of all L1-dcache hits    (38.52%)
        53,950,612      LLC-loads                 #   10.033 M/sec                    (30.73%)
            37,009      LLC-load-misses           #    0.07% of all LL-cache hits     (30.72%)
   <not supported>      L1-icache-loads                                             
         1,588,412      L1-icache-load-misses                                         (30.72%)
    17,760,159,266      dTLB-loads                # 3302.656 M/sec                    (30.72%)
             6,828      dTLB-load-misses          #    0.00% of all dTLB cache hits   (30.58%)
            10,384      iTLB-loads                #    0.002 M/sec                    (30.57%)
               755      iTLB-load-misses          #    7.27% of all iTLB cache hits   (30.72%)
   <not supported>      L1-dcache-prefetches                                        
   <not supported>      L1-dcache-prefetch-misses                                   

       5.407542426 seconds time elapsed

       5.358630000 seconds user
       0.020009000 seconds sys


```
Here we see much less `L1-dcache-load-misses`, as all the variables we are updating are in contigous memory,
and the CPU prefecther has a much easier time predicting what data is needed.

Thinking in terms of *hot* and *cold* data, we can say the `i0` and `i0s` is our hot data
(the data that we work on),
and the rest is our cold data (data which we do not use).
In the AoS approach we end up loading all our cold data when we load a cache-line,
when we only needed to load the hot data, namely the `i0`'s.
In the SoA approach we only load the hot data, *i.e.* the `i0s` we are working on, 
and the cold data is never touched.
Thus in this case we utilize the full cache-line we read.


**Q** & **A**
-------------------------------------------------------------------------------

**Q1**: 
If accessing all `i0`-`i15` for a single "object", which would be faster? AoS or SoA?

**A1**:
In this case AoS would be faster, as all data for a single object will be placed on a single cache-line,
and all loaded at the same time, whereas for SoA, you would have to load a cache-line for variable to update.


**Q2**:
If accessing all `i0`-`i15` for all "objects", which would be faster? AoS or SoA?

**A2**:
In this case AoS and SoA would perform equally well,
given that in AoS you update all members for an object at the same time,
and in SoA you update all `i0s` first, then `i1s`, etc.
This is assuming that all `i0`-`i15` in the AoS approach are updated in the same way
to utilize SIMD (*i.e.* using a Similar-Instruction).

Disclaimer
-------------------------------------------------------------------------------

This particular example is of course concieved and arguably skewed,
but it does still mimic common data layout and access patterns that you would see
in many code bases applying OOP. Also this is not a call to abandon the use of OOP, 
but an encouragement to reason about the implications of applying 
the classical OOP approach of dividing all types into classes or structures,
instantiated as individual objects. OOP and DOD are not each others opposites,
and combining the two makes for a robust and high performance design 
that will run efficiently on todays modern hardware.

Quote
-------------------------------------------------------------------------------

**Mature programmers know that the idea that everything is an object is a myth. 
   Sometimes you really do want simple data structures with procedures operating on them.**
                                                                  *- Robert C. Martin*

Array-Of-Structs (AoS) vs. Struct-Of-Arrays (SoA)
-------------------------------------------------------------------------------

In an Array-Of-Structs (also loosely called the Object-Oriented (OO)-approach)
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

In a Struct-Of-Arrays (also loosely known as Data-Oriented-Design (DoD)),
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
On my system, which is `x86_64` (like many systems), it is `64`:
```sh
$ cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size 
64
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
Here we clearly see a lot (!) of `L1-dcache-load-misses`, which means a huge hit to our performance.


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

**Q** & **A**
-------------------------------------------------------------------------------
**Q**: 
If accessing all `i0`-`i15` for a single "object", which would be faster? AoS or SoA?

**A**:
In this case AoS would be faster, as all data for a single object will be placed on a single cache-line,
and all loaded at the same time, whereas for SoA, you would have to load a cache-line for variable to update.


**Q**:
If accessing all `i0`-`i15` for all "objects", which would be faster? AoS or SoA?

**A**:
In this case AoS and SoA would perform equally well,
given that in AoS you update all members for an object at the same time,
and in SoA you update all `i0s` first, then `i1s`, etc.

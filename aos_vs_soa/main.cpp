#include <benchmark/benchmark.h>

/* Define a simple struct 
 */
struct SimpleStruct
{
   /* We make it the size of a cache line 
    * (sizeof(int) is 4, so 16 * 4 = 64) 
    * All are initialized to 0.
    */
   int i0  = 0;
   int i1  = 0;
   int i2  = 0;
   int i3  = 0;
   int i4  = 0;
   int i5  = 0;
   int i6  = 0;
   int i7  = 0;
   int i8  = 0;
   int i9  = 0;
   int i10 = 0;
   int i11 = 0;
   int i12 = 0;
   int i13 = 0;
   int i14 = 0;
   int i15 = 0;
   
   /* Update a member
    * (for brevity we only implement this for 'i1'
    */
   void update_i0()
   {
      ++i0;
   }
};

/* Define Struct-of-Array */
struct Soa
{
   /* Here we will have an array holding each variable
    */
   std::vector<int> i0s;
   std::vector<int> i1s;
   std::vector<int> i2s;
   std::vector<int> i3s;
   std::vector<int> i4s;
   std::vector<int> i5s;
   std::vector<int> i6s;
   std::vector<int> i7s;
   std::vector<int> i8s;
   std::vector<int> i9s;
   std::vector<int> i10s;
   std::vector<int> i11s;
   std::vector<int> i12s;
   std::vector<int> i13s;
   std::vector<int> i14s;
   std::vector<int> i15s;
   
   /* Initialize the SoA, initialize all i0s etc. to 0 */
   Soa(int N)
      :  i0s(N, 0)
      ,  i1s(N, 0)
      ,  i2s(N, 0)
      ,  i3s(N, 0)
      ,  i4s(N, 0)
      ,  i5s(N, 0)
      ,  i6s(N, 0)
      ,  i7s(N, 0)
      ,  i8s(N, 0)
      ,  i9s(N, 0)
      ,  i10s(N, 0)
      ,  i11s(N, 0)
      ,  i12s(N, 0)
      ,  i13s(N, 0)
      ,  i14s(N, 0)
      ,  i15s(N, 0)
   {
   }
   
   /* Update all i0's. Loop over vector i0s and update */
   void update_i0s()
   {
      for(auto& i : i0s)
      {
         ++i;
      }
   }
};

/* Benchmark the Array-of-Structs approach
 */
static void Benchmark_Aos(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   std::vector<SimpleStruct> aos(N);

   while(s.KeepRunning())
   {
      for(auto& i : aos)
      {
         i.update_i0();
      }
   }
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_Aos)->DenseRange(8, 16);

/* Benchmark the Structs-of-Array approach
 */
static void Benchmark_Soa(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   Soa soa(N);

   while(s.KeepRunning())
   {
      soa.update_i0s();
   }
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_Soa)->DenseRange(8, 16);

/* Create main()-function */
BENCHMARK_MAIN();

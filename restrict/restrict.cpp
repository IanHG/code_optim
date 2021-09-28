#include <benchmark/benchmark.h>

template<class T>
T dot(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
   T dot_result = static_cast<T>(0.0);

   for(int i = 0; i < static_cast<int>(vec1.size()); ++i)
   {
      dot_result += vec1[i] * vec2[i];
   }

   return dot_result;
}

template<class T>
T dot_iterator(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
   T dot_result = static_cast<T>(0.0);
   
   auto iter1 = vec1.begin();
   auto iter2 = vec2.begin();

   for(; iter1 != vec1.end(); ++iter1, ++iter2)
   {
      dot_result += (*iter1) * (*iter2);
   }

   return dot_result;
}

template<class T>
T dot(const T* const vec1, const T* const vec2, int size)
{
   T dot_result = static_cast<T>(0.0);

   for(int i = 0; i < size; ++i)
   {
      dot_result += vec1[i] * vec2[i];
   }

   return dot_result;
}

template<class T>
T dot_restrict(const T* const __restrict vec1, const T* const __restrict vec2, int size)
{
   T dot_result = static_cast<T>(0.0);

   for(int i = 0; i < size; ++i)
   {
      dot_result += vec1[i] * vec2[i];
   }

   return dot_result;
}

static void Benchmark_vector(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   std::vector<double> vec1(N, 1.1);
   std::vector<double> vec2(N, 1.2);
   
   double d;
   while(s.KeepRunning())
   {
      benchmark::DoNotOptimize(d = dot(vec1, vec2));
   }
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_vector)->DenseRange(8, 16);

static void Benchmark_vector_iterator(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   std::vector<double> vec1(N, 1.1);
   std::vector<double> vec2(N, 1.2);
   
   double d;
   while(s.KeepRunning())
   {
      benchmark::DoNotOptimize(d = dot_iterator(vec1, vec2));
   }
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_vector_iterator)->DenseRange(8, 16);

static void Benchmark_pointer(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   double* vec1 = new double[N];
   double* vec2 = new double[N];

   for(int i = 0; i < N; ++i)
   {
      vec1[i] = 1.1;
      vec2[i] = 1.2;
   }
   
   double d;
   while(s.KeepRunning())
   {
      benchmark::DoNotOptimize(d = dot(vec1, vec2, N));
   }

   delete[] vec1;
   delete[] vec2;
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_pointer)->DenseRange(8, 16);

static void Benchmark_pointer_restrict(benchmark::State& s)
{
   // Create 2^I samples
   int N = 1 << s.range(0);

   double* vec1 = new double[N];
   double* vec2 = new double[N];

   for(int i = 0; i < N; ++i)
   {
      vec1[i] = 1.1;
      vec2[i] = 1.2;
   }
   
   double d;
   while(s.KeepRunning())
   {
      benchmark::DoNotOptimize(d = dot_restrict(vec1, vec2, N));
   }

   delete[] vec1;
   delete[] vec2;
}

/* Run benchmark for I in [8; 16] */
BENCHMARK(Benchmark_pointer_restrict)->DenseRange(8, 16);

/* Create main()-function */
BENCHMARK_MAIN();

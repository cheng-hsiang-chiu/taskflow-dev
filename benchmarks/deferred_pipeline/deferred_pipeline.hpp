#include <chrono>

inline void work() {
  size_t N = 4;
  size_t mat1[N][N];
  size_t mat2[N][N];
  size_t res[N][N];

  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      mat1[i][j] = i+1;
      mat2[i][j] = j+1;
    }
  }

  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      res[i][j] = 0;
      for (size_t k = 0; k < N; k++) {
        res[i][j] += mat1[i][k] * mat2[k][j];
      }
    }
  }
  
  //std::this_thread::sleep_for(std::chrono::microseconds(10));
}

std::chrono::microseconds measure_time_taskflow(size_t , size_t, int, size_t);
std::chrono::microseconds measure_time_pthread(size_t, size_t, int, size_t);


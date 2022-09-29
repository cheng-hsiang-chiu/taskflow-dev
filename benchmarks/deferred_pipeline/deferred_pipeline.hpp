#include <chrono>
#include <thread>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <iostream>

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
   
  std::this_thread::sleep_for(std::chrono::microseconds(10));
  
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
}


inline bool verify(
  size_t num_threads, size_t frequency, int deferred, size_t num_frames,
  std::vector<int>& vec) {

  assert(vec.size() == num_frames);

  for (auto& fid : vec) {
    if (fid != 0 && fid%frequency != 0) {
      continue;
    }
    if (fid+deferred < 0 || 
        fid+deferred >= static_cast<int>(num_frames) || 
        (fid+1)%num_threads == 0) {
      continue;
    }

    auto it  = std::find(vec.begin(), vec.end(), fid);
    auto dit = std::find(vec.begin(), vec.end(), fid+deferred);
    
    if (std::distance(dit, it) > 0) {
      return true;
    }
    else {
      return false;
    }
  }
}



std::chrono::microseconds measure_time_taskflow(size_t , size_t, int, size_t, bool);
std::chrono::microseconds measure_time_pthread(size_t, size_t, int, size_t, bool);


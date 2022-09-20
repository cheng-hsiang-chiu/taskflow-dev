#include "deferred_pipeline.hpp"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/pipeline.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>


class Frame {
public:
  bool processed = false;
  size_t fid;
  std::mutex m;                                                                                          
  std::condition_variable  cv;

  Frame(size_t i) : fid{i} {}
};

std::vector<std::unique_ptr<Frame>> video;


void encode_frame(const size_t idx, const size_t num_threads,
  const size_t frequency, const int deferred, const size_t num_frames) {
  
  size_t fid;
  size_t iterations = num_frames/num_threads;

  for (size_t i = 0; i < iterations; ++i) {
    fid = num_threads*i+idx;
    //printf("fid = %ld\n", fid);

    if (fid != 0 && fid%frequency != 0) {
      std::unique_lock lk(video[fid]->m);
      //printf("Thread %ld works on video[%ld]\n",
      //       std::hash<std::thread::id>{}(std::this_thread::get_id()),
      //       fid);
      work();
      video[fid]->processed = true;
      video[fid]->cv.notify_all();
      continue; 
    }

    if (fid+deferred < 0 || fid+deferred >= video.size()) {
      std::unique_lock lk(video[fid]->m);
        //printf("Thread %ld works on video[%ld]\n",
        //       std::hash<std::thread::id>{}(std::this_thread::get_id()),
        //       fid);
      work();
      video[fid]->processed = true;
      video[fid]->cv.notify_all();
    }

    else {
      {
        std::unique_lock lk(video[fid+deferred]->m);
        video[fid+deferred]->cv.wait(lk, [&]{ return video[fid+deferred]->processed; });
      }
      {
        std::unique_lock lk(video[fid]->m);
        //printf("Thread %ld works on video[%ld]\n",
        //       std::hash<std::thread::id>{}(std::this_thread::get_id()),
        //       fid);
        work();
        video[fid]->processed = true;
        video[fid]->cv.notify_all();
      }
    }
  }
}

std::chrono::microseconds measure_time_pthread(
  size_t  num_threads, size_t frequency, int deferred, size_t num_frames) {

  std::chrono::microseconds elapsed;
  
  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_frames; ++i) {
    std::unique_ptr<Frame> p(new Frame(i));
    video.emplace_back(std::move(p));
  }
  
  auto beg = std::chrono::high_resolution_clock::now();
  
  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back(
      std::thread(encode_frame, i, num_threads, frequency, deferred, num_frames));
  }
  
  // join all threads
  for (size_t i = 0; i < num_threads; ++i) {
    threads[i].join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  
  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
  
  return elapsed;
}

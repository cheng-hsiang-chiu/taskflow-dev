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

std::vector<int> vec1;
std::mutex global_m;

void encode_frame(const size_t idx, const size_t num_threads,
  const size_t frequency, const int deferred, const size_t num_frames, bool isverify) {
   
  size_t fid;
  size_t iterations = num_frames/num_threads;

  for (size_t i = 0; i < iterations; ++i) {
    fid = num_threads*i+idx;
    //printf("fid = %ld\n", fid);

    if (fid != 0 && fid%frequency != 0) {
      {
        if (isverify) {
          std::unique_lock lk(global_m);
          vec1.push_back(fid);
        }
      }
      std::unique_lock lk(video[fid]->m);
      //printf("Thread %ld works on video[%ld]\n",
      //       std::hash<std::thread::id>{}(std::this_thread::get_id()),
      //       fid);
      work();
      video[fid]->processed = true;
      video[fid]->cv.notify_all();
      continue; 
    }

    //if (fid+deferred < 0 || fid+deferred >= video.size()) {
    if (fid+deferred < 0 || fid+deferred >= num_frames || ((fid>0) && ((fid+1)%num_threads == 0))) {
      {
        if (isverify) {
          std::unique_lock lk(global_m);
          vec1.push_back(fid);
        }
      }
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
        //printf("Thread %ld waits on video[%ld]\n",
        //       std::hash<std::thread::id>{}(std::this_thread::get_id()),
        //       fid+deferred);
        video[fid+deferred]->cv.wait(lk, [&]{ return video[fid+deferred]->processed; });
      }
      {
        {
          if (isverify) {
            std::unique_lock lk(global_m);
            vec1.push_back(fid);
          }
        }
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
  size_t  num_threads, size_t frequency, int deferred, size_t num_frames,
  bool isverify) {

  std::chrono::microseconds elapsed;
  
  std::vector<std::thread> threads;
  //std::cout << "number of frames = " << num_frames << '\n';
  for (size_t i = 0; i < num_frames; ++i) {
    std::unique_ptr<Frame> p(new Frame(i));
    video.emplace_back(std::move(p));
  }
  if (isverify) {
    vec1.clear();
  }
  auto beg = std::chrono::high_resolution_clock::now();
  
  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back(
      std::thread(encode_frame, i, num_threads, frequency, deferred, num_frames, isverify));
  }
  
  // join all threads
  for (size_t i = 0; i < num_threads; ++i) {
    threads[i].join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  
  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);

  if (isverify) {
    bool passed = verify(num_threads, frequency, deferred, num_frames, vec1);
    if (!passed) {
      std::cout << "Wrong answer\n";
    }
  }
  
  return elapsed;
}

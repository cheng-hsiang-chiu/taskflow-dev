#include "deferred_pipeline.hpp"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/pipeline.hpp>

class Frame {
public:
  bool processed = false;
  size_t fid;

  Frame(size_t i) : fid{i} {}
};

std::chrono::microseconds measure_time_taskflow(
  size_t  num_threads, size_t frequency, int deferred, size_t num_frames) {

  std::chrono::microseconds elapsed;
  
  tf::Taskflow taskflow;
  
  tf::Executor executor(num_threads);

  //for (size_t i = 0; i < num_frames; ++i) {
  //  std::unique_ptr<Frame> p(new Frame(i));
  //  video.emplace_back(std::move(p));
  //}

  auto beg = std::chrono::high_resolution_clock::now();
  
  tf::Pipeline pl(num_threads,
    tf::Pipe{tf::PipeType::SERIAL, [frequency, num_frames, deferred](tf::Pipeflow& pf) {
      if(pf.token() == num_frames) {
        pf.stop();
      }

      else {
        if (pf.token()%frequency == 0 && 
            (pf.token()+deferred > 0 || pf.token()+deferred < num_frames) && 
            pf.token() != 0) {

         switch(pf.deferred()) {
           case 0:
            pf.defer(pf.token()+deferred);
           break;
         }
        }
      }
    }},
    
    // second serial pipe
    tf::Pipe{tf::PipeType::SERIAL, [](tf::Pipeflow& pf){
      work();  
    }}
  );

  taskflow.composed_of(pl);
  executor.run(taskflow).wait();
  
  auto end = std::chrono::high_resolution_clock::now();

  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
  
  return elapsed;
}



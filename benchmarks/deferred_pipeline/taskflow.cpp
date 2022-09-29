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
  size_t  num_threads, size_t frequency, int deferred, size_t num_frames, 
  bool isverify) {

  std::chrono::microseconds elapsed;
  
  tf::Taskflow taskflow;
  
  tf::Executor executor(num_threads);

  std::vector<int> vec;

  auto beg = std::chrono::high_resolution_clock::now();
   
  tf::Pipeline pl(num_threads,
    tf::Pipe{tf::PipeType::SERIAL, [frequency, num_frames, deferred, num_threads, &vec, isverify](tf::Pipeflow& pf) {
      if(pf.token() == num_frames) {
        pf.stop();
      }

      else {
        if (pf.token() != 0 && pf.token()%frequency != 0) {
          if (isverify) {
            vec.push_back(pf.token());  
          }
        }
        else if (pf.token()+deferred < 0 || pf.token()+deferred >= num_frames || 
                 (pf.token()+1)%num_threads == 0) {
          if (isverify) {
            vec.push_back(pf.token());  
          }
        }
        else {
          switch(pf.deferred()) {
            case 0:
              pf.defer(pf.token()+deferred);
            break;

            default:
              if (isverify) {
                vec.push_back(pf.token());
              }
            break;
          }
        }
      }
    }},
    
    // second parallel pipe
    tf::Pipe{tf::PipeType::PARALLEL, [](tf::Pipeflow& pf){
      work();  
    }}
  );

  taskflow.composed_of(pl);
  executor.run(taskflow).wait();
  
  auto end = std::chrono::high_resolution_clock::now();

  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
 
  if (isverify) {  
    bool passed = verify(num_threads, frequency, deferred, num_frames, vec);
    if (!passed) {
      std::cout << "Wrong answer\n";
    }
  }

  return elapsed;
}



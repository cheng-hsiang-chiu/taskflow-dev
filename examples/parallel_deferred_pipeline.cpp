// This program demonstrates how to create a pipeline scheduling framework
// that defers the exection of current scheuling token to the future.  
//
// The pipeline has the following structure:
//
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o

// The scheduling token has the following dependencies:
//    ___________
//   |           |
//   V _____     |
//   |     |     | 
//   |     V     | 
// 1 2 3 4 5 6 7 8 9 10 
//         ^   |   |
//         |___|   |
//         ^       | 
//         |_______|


#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/pipeline.hpp>

int main() {

  tf::Taskflow taskflow("deferred_pipeline");
  tf::Executor executor;

  const size_t num_lines = 4;

  // custom data storage
  std::array<int, num_lines> buffer;

  // the pipeline consists of three pipes (serial-parallel-serial)
  // and up to four concurrent scheduling tokens
  tf::Pipeline pl(num_lines,
    tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
      // generate only 15 scheduling tokens
      if(pf.token() == 15) {
        pf.stop();
      }
      // save the result of this pipe into the buffer
      else {

        if (pf.token() == 5) {
          switch(pf.deferred()) {
            case 0:
              pf.defer(2);
              printf("Scheduling token %zu defers to 2.\n", pf.token());
              pf.defer(7);
              printf("Scheduling token %zu defers to 7.\n", pf.token());
              return;  
            break;

            case 1:
              pf.defer(9);
              printf("Scheduling token %zu defers to 9.\n", pf.token());
              return;
            break;

            case 2:
              printf("stage 1: Scheduling tokens 2, 7 and 9 are scheduled for token %zu.\n", pf.token());
            break;
          }
        }
        else if (pf.token() == 2) {
          switch(pf.deferred()) {
            case 0:
              pf.defer(3);
              printf("Scheduling token %zu defers to 3.\n", pf.token());
              pf.defer(8);
              printf("Scheduling token %zu defers to 8.\n", pf.token());
            break;
            case 1:
              pf.defer(7);
              printf("Scheduling token %zu defers to 7.\n", pf.token());
              pf.defer(11);
              printf("Scheduling token %zu defers to 11.\n", pf.token());
            break;
            default:
              printf("stage 1: Scheduling token 3, 8, 7, 11 are scheduled for token %zu.\n", pf.token());
            break;
          }
        }
        else if (pf.token() == 7) {
          switch(pf.deferred()) {
            case 0:
              pf.defer(10);
              printf("Scheduling token %zu defers to 10.\n", pf.token());
            break;
            default:
              printf("stage 1: Scheduling token 10 is scheduled for token %zu.\n", pf.token());
            break;
          }
        }
        else {
          printf("stage 1: input token = %zu\n", pf.token());
          buffer[pf.line()] = pf.token();
        }
      }
    }},

    tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
      printf(
        "stage 2: input buffer[%zu] = %d from token %zu\n", pf.line(), buffer[pf.line()], pf.token()
      );
      // propagate the previous result to this pipe and increment 
      // it by one
      buffer[pf.line()] = buffer[pf.line()] + 1;
    }},

    tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
      printf(
        "stage 3: input buffer[%zu] = %d from token %zu\n", pf.line(), buffer[pf.line()], pf.token()
      );
      // propagate the previous result to this pipe and increment
      // it by one
      buffer[pf.line()] = buffer[pf.line()] + 1;
    }}
  );
  
  // build the pipeline graph using composition
  tf::Task init = taskflow.emplace([](){ std::cout << "ready\n"; })
                          .name("starting pipeline");
  tf::Task task = taskflow.composed_of(pl)
                          .name("deferred_pipeline");
  tf::Task stop = taskflow.emplace([](){ std::cout << "stopped\n"; })
                          .name("pipeline stopped");

  // create task dependency
  init.precede(task);
  task.precede(stop);
  
  // dump the pipeline graph structure (with composition)
  taskflow.dump(std::cout);

  // run the pipeline
  executor.run(taskflow).wait();
  
  return 0;
}

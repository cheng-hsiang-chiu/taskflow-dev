#include "deferred_pipeline.hpp"
#include <CLI11.hpp>
#include <cstdlib>
#include <cassert>

int main(int argc, char* argv[]) {

  CLI::App app{"Parallel Deferred Pipeline"};

  unsigned num_threads {1};
  app.add_option("-t,--num_threads", num_threads, "number of threads (default=1)");

  unsigned num_rounds {1};
  app.add_option("-r,--num_rounds", num_rounds, "number of rounds (default=1)");

  std::string model = "tf";
  app.add_option("-m,--model", model, "model name pthread|tf (default=tf)")
    ->check([] (const std::string& m) {
      if(m != "pthread" && m != "tf") {
        return "model name should be \"pthread\" or \"tf\"";
      }
      return "";
    });

  size_t frequency {1};
  app.add_option("-f, --frequency", frequency, "frequency of deferred token (default=1)");
  
  int deferred{-1};
  app.add_option("-d, --deferred", deferred, "defer to previous/future deferred token (default=-1)");

  bool verify{false};
  app.add_option("-v, --verify", verify, "whether to verify the result (default=false)");
   
  CLI11_PARSE(app, argc, argv);
  if (static_cast<int>(num_threads) <= deferred) { 
    std::cerr << "Wrong configuration : deferred must be smaller than num_threads\n";
    exit(-1);
  }

  if (deferred == 0) {
    std::cerr << "Wrong configuration : deferred can not be zero\n";
    exit(-1);
  }

  std::cout << "model="       << model       << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds="  << num_rounds  << ' '
            << "frequency="   << frequency   << ' '
            << "deferred="    << deferred    << ' '
            << std::endl;

  std::cout << std::setw(12) << "Size"
            << std::setw(12) << "Runtime"
            << '\n';

  size_t shift = 1;
  size_t max_shift = 21;

  for(size_t i = (1 << shift); i <= (1 << max_shift); i*=2) {

    double runtime {0.0};

    for(unsigned j = 0; j < num_rounds; ++j) {
      
      if(model == "tf") {
        runtime += measure_time_taskflow(num_threads, frequency, deferred, i, verify).count();
      }
      else if(model == "pthread") {
        runtime += measure_time_pthread(num_threads, frequency, deferred, i, verify).count();
      }
    }

    std::cout << std::setw(12) << i
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;

  }
}

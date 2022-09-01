#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/pipeline.hpp>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <mutex>
#include <algorithm>


/*
// ----------------------------------------------------------------------------
// two pipes (SS), L lines, W workers, defer to the previous token
// ----------------------------------------------------------------------------

void pipeline_2P_SS_DeferPreviousToken(size_t L, unsigned w) {

  tf::Executor executor(w);

  const size_t maxN = 100;

  std::vector<std::array<size_t, 2>> mybuffer(L);

  for(size_t N = 0; N <= maxN; N++) {
    
    std::vector<size_t> collection1;
    std::vector<size_t> collection2;
    std::mutex mutex;

    tf::Taskflow taskflow;

    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [N, &collection1, &mybuffer, L](auto& pf) mutable {
        if(pf.token() == N) {
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (pf.token() == 0) {
                //printf("Stage 1 : token %zu on line %zu\n", pf.token() ,pf.line());
                collection1.push_back(pf.token());
                mybuffer[pf.line()][pf.pipe()] = pf.token();           
              }
              else {
                pf.defer(pf.token()-1);
              }
            break;

            case 1:
              //printf("Stage 1 : token %zu on line %zu\n", pf.token(), pf.line());
              collection1.push_back(pf.token());
              mybuffer[pf.line()][pf.pipe()] = pf.token();           
            break;
          }
          REQUIRE(pf.token() % L == pf.line());
        }
      }},

      tf::Pipe{tf::PipeType::SERIAL, [N, &mybuffer, &mutex, &collection2, L](auto& pf) mutable {
        REQUIRE(pf.token() % L == pf.line());
        {
          std::scoped_lock<std::mutex> lock(mutex);
          collection2.push_back(mybuffer[pf.line()][pf.pipe() - 1]);
        }
        //printf("Stage 2 : token %zu at line %zu\n", pf.token(), pf.line());
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    auto test = taskflow.emplace([&](){
      REQUIRE(collection1.size() == N);
      REQUIRE(collection2.size() == N);

      for (size_t i = 0; i < N; ++i) {
        REQUIRE(collection1[i] == i);
        REQUIRE(collection2[i] == i);
      }
    }).name("test");

    pipeline.precede(test);

    executor.run_n(taskflow, 1, [&]() mutable {
      collection1.clear();
      collection2.clear();
    }).get();
  }
}

// two pipes (SS)
TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(1, 1);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(1, 2);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(1, 3);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(1, 4);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(2, 1);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(2, 2);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(2, 3);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(2, 4);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(3, 1);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(3, 2);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(3, 3);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(3, 4);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(4, 1);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(4, 2);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(4, 3);
}

TEST_CASE("Pipeline.2P(SS).DeferPreviousToken.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferPreviousToken(4, 4);
}


// ----------------------------------------------------------------------------
// two pipes (SP), L lines, W workers, defer to the previous token
// ----------------------------------------------------------------------------

void pipeline_2P_SP_DeferPreviousToken(size_t L, unsigned w) {

  tf::Executor executor(w);

  const size_t maxN = 100;

  std::vector<std::array<size_t, 2>> mybuffer(L);

  for(size_t N = 0; N <= maxN; N++) {
    
    std::vector<size_t> collection1;
    std::vector<size_t> collection2;
    std::mutex mutex;

    tf::Taskflow taskflow;

    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [N, &collection1, &mybuffer, L](auto& pf) mutable {
        if(pf.token() == N) {
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (pf.token() == 0) {
                //printf("Stage 1 : token %zu on line %zu\n", pf.token() ,pf.line());
                collection1.push_back(pf.token());
                mybuffer[pf.line()][pf.pipe()] = pf.token();           
              }
              else {
                pf.defer(pf.token()-1);
              }
            break;

            case 1:
              //printf("Stage 1 : token %zu on line %zu\n", pf.token(), pf.line());
              collection1.push_back(pf.token());
              mybuffer[pf.line()][pf.pipe()] = pf.token();           
            break;
          }
          REQUIRE(pf.token() % L == pf.line());
        }
      }},

      tf::Pipe{tf::PipeType::PARALLEL, [N, &mybuffer, &mutex, &collection2, L](auto& pf) mutable {
        REQUIRE(pf.token() % L == pf.line());
        {
          std::scoped_lock<std::mutex> lock(mutex);
          collection2.push_back(mybuffer[pf.line()][pf.pipe() - 1]);
        }
        //printf("Stage 2 : token %zu at line %zu\n", pf.token(), pf.line());
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    auto test = taskflow.emplace([&](){
      REQUIRE(collection1.size() == N);
      REQUIRE(collection2.size() == N);
      sort(collection2.begin(), collection2.end());
  
      for (size_t i = 0; i < N; ++i) {
        REQUIRE(collection1[i] == i);
        REQUIRE(collection2[i] == i);
      }
    }).name("test");

    pipeline.precede(test);

    executor.run_n(taskflow, 1, [&]() mutable {
      collection1.clear();
      collection2.clear();
    }).get();
  }
}

// two pipes (SP)
TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(1, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(1, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(1, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(1, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(2, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(2, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(2, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(2, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(3, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(3, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(3, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(3, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(4, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(4, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(4, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferPreviousToken.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferPreviousToken(4, 4);
}


// ----------------------------------------------------------------------------
// two pipes (SS), L lines, W workers
//
// defer to the next token, pf.defer(pf.token()+1) except the max token
// ----------------------------------------------------------------------------

void pipeline_2P_SS_DeferNextToken(size_t L, unsigned w, tf::PipeType second_type) {

  tf::Executor executor(w);

  const size_t maxN = 100;

  std::vector<int> source(maxN);
  std::iota(source.begin(), source.end(), 0);
  std::vector<size_t> mybuffer(L);

  std::vector<size_t> collection1;
  std::vector<size_t> collection2;

  for(size_t N = 1; N <= maxN; N++) {

    tf::Taskflow taskflow;

    size_t j1 = 0, j2 = 0;
    size_t cnt = 1;
    size_t value = (N-1)%L;
    //std::cout << "N = " << N << ", value = " << value << ", L = " << L << ", W = " << w << '\n';    
    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [value, N, &source, &j1, &mybuffer, L, &collection1](auto& pf) mutable {
        if(pf.token() == N) {
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (pf.token() < N-1) {
                pf.defer(pf.token()+1);
              }
              else {
                //REQUIRE((pf.token()+pf.line())%L == value);
                collection1.push_back(pf.token());
                mybuffer[pf.line()] = pf.token();              
              }
            break;

            case 1:
              //if ((pf.token()+pf.line())%L != value) {
              //  std::cout << N << ", " << pf.token() << ", " << pf.line() << ", " << L << ", " << value << '\n';
              //}
              collection1.push_back(pf.token());
              //REQUIRE((pf.token()+pf.line())%L == value);
              mybuffer[pf.line()] = pf.token();              
            break;
          }
        }

        //REQUIRE(j1 == source[j1]);
        //REQUIRE(pf.token() % L == pf.line());
        //*(pf.output()) = source[j1] + 1;
        //mybuffer[pf.line()][pf.pipe()] = source[j1] + 1;
        //j1++;
      }},

      tf::Pipe{second_type, [value, N, &source, &j2, &mybuffer, L, &collection2](auto& pf) mutable {
        collection2.push_back(mybuffer[pf.line()]);
        //REQUIRE(j2 < N);
        //REQUIRE(pf.token() % L == pf.line());
        //REQUIRE(source[j2] + 1 == mybuffer[pf.line()][pf.pipe() - 1]);
        //REQUIRE(source[j2] + 1 == *(pf.input()));
        //j2++;
        //REQUIRE((mybuffer[pf.line()]+pf.line())%L == value);
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    executor.run(taskflow).wait();
   
    for (size_t i = 0; i < collection1.size(); ++i) {
      REQUIRE(i + collection1[i] == N-1);
      REQUIRE(i + collection2[i] == N-1);
    }
    
    collection1.clear();
    collection2.clear();
    //auto test = taskflow.emplace([&](){
    //  REQUIRE(j1 == N);
    //  REQUIRE(j2 == N);
    //  REQUIRE(pl.num_tokens() == cnt * N);
    //}).name("test");

    //pipeline.precede(test);

    //executor.run_n(taskflow, 3, [&]() mutable {
    //  j1 = 0;
    //  j2 = 0;
    //  for(size_t i = 0; i < mybuffer.size(); ++i){
    //    for(size_t j = 0; j < mybuffer[0].size(); ++j){
    //      mybuffer[i][j] = 0;
    //    }
    //  }
    //  cnt++;
    //}).get();
  }
}

// two pipes 
TEST_CASE("Pipeline.2P(SS).DeferNextToken.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(1, 1, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(1, 2, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(1, 3, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(1, 4, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(2, 1, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(2, 2, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(2, 3, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(2, 4, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(3, 1, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(3, 2, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(3, 3, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(3, 4, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(4, 1, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(4, 2, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(4, 3, tf::PipeType::SERIAL);
}

TEST_CASE("Pipeline.2P(SS).DeferNextToken.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_DeferNextToken(4, 4, tf::PipeType::SERIAL);
}

// ----------------------------------------------------------------------------
// two pipes (SP), L lines, W workers
//
// defer to the next token, pf.defer(pf.token()+1) except the max token
// ----------------------------------------------------------------------------

void pipeline_2P_SP_DeferNextToken(size_t L, unsigned w) {

  tf::Executor executor(w);

  const size_t maxN = 100;

  std::vector<int> source(maxN);
  std::iota(source.begin(), source.end(), 0);
  std::vector<size_t> mybuffer(L);

  std::vector<size_t> collection1;
  std::vector<size_t> collection2;

  for(size_t N = 1; N <= maxN; N++) {

    tf::Taskflow taskflow;

    size_t j1 = 0, j2 = 0;
    size_t cnt = 1;
    size_t value = (N-1)%L;
    //std::cout << "N = " << N << ", value = " << value << ", L = " << L << ", W = " << w << '\n';    
    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [value, N, &source, &j1, &mybuffer, L, &collection1](auto& pf) mutable {
        if(pf.token() == N) {
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (pf.token() < N-1) {
                pf.defer(pf.token()+1);
              }
              else {
                //REQUIRE((pf.token()+pf.line())%L == value);
                collection1.push_back(pf.token());
                mybuffer[pf.line()] = pf.token();              
              }
            break;

            case 1:
              //if ((pf.token()+pf.line())%L != value) {
              //  std::cout << N << ", " << pf.token() << ", " << pf.line() << ", " << L << ", " << value << '\n';
              //}
              collection1.push_back(pf.token());
              //REQUIRE((pf.token()+pf.line())%L == value);
              mybuffer[pf.line()] = pf.token();              
            break;
          }
        }

        //REQUIRE(j1 == source[j1]);
        //REQUIRE(pf.token() % L == pf.line());
        //*(pf.output()) = source[j1] + 1;
        //mybuffer[pf.line()][pf.pipe()] = source[j1] + 1;
        //j1++;
      }},

      tf::Pipe{tf::PipeType::PARALLEL, [value, N, &source, &j2, &mybuffer, L, &collection2](auto& pf) mutable {
        collection2.push_back(mybuffer[pf.line()]);
        //REQUIRE(j2 < N);
        //REQUIRE(pf.token() % L == pf.line());
        //REQUIRE(source[j2] + 1 == mybuffer[pf.line()][pf.pipe() - 1]);
        //REQUIRE(source[j2] + 1 == *(pf.input()));
        //j2++;
        //REQUIRE((mybuffer[pf.line()]+pf.line())%L == value);
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    executor.run(taskflow).wait();
  
    sort(collection2.begin(), collection2.end()); 
    for (size_t i = 0; i < collection1.size(); ++i) {
      REQUIRE(i + collection1[i] == N-1);
      REQUIRE(collection2[i] == i);
    }
    
    collection1.clear();
    collection2.clear();
    //auto test = taskflow.emplace([&](){
    //  REQUIRE(j1 == N);
    //  REQUIRE(j2 == N);
    //  REQUIRE(pl.num_tokens() == cnt * N);
    //}).name("test");

    //pipeline.precede(test);

    //executor.run_n(taskflow, 3, [&]() mutable {
    //  j1 = 0;
    //  j2 = 0;
    //  for(size_t i = 0; i < mybuffer.size(); ++i){
    //    for(size_t j = 0; j < mybuffer[0].size(); ++j){
    //      mybuffer[i][j] = 0;
    //    }
    //  }
    //  cnt++;
    //}).get();
  }
}

// two pipes 
TEST_CASE("Pipeline.2P(SP).DeferNextToken.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(1, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(1, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(1, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(1, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(2, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(2, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(2, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(2, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(3, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(3, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(3, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(3, 4);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(4, 1);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(4, 2);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(4, 3);
}

TEST_CASE("Pipeline.2P(SP).DeferNextToken.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_DeferNextToken(4, 4);
}
*/


// ----------------------------------------------------------------------------
// two pipes (SS), L lines, W workers, mimic 264 frame patterns
// ----------------------------------------------------------------------------

struct Frames {
  char type;
  size_t id;
  std::vector<size_t> defers;
};

std::vector<char> types{'I','B','B','B','P','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','P','B','B','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','P','I','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','B','B','B','P','P'};


void construct_video(std::vector<Frames>& video, const size_t N) {
  for (size_t i = 0; i < N; ++i) {
    video.push_back({types[i], i, std::vector<size_t>{}});

    if (types[i] == 'P') {
      size_t step = 1;
      size_t index;
      while (static_cast<int>(i-step) >= 0) {
        index = i - step;
        if (types[index] == 'P' || types[index] == 'I') {
          video[i].defers.push_back(index);
          break;
        }
        else {
          ++step;
        }
      }
    }
    else if (types[i] == 'B') {
      size_t step = 1;
      size_t index;
      while (static_cast<int>(i-step) >= 0) {
        index = i - step;
        if (types[index] == 'P' || types[index] == 'I') {
          video[i].defers.push_back(index);
          break;
        }
        else {
          ++step;
        }
      }
      step = 1;
      while (i+step < N) {
        index = i + step;
        if (types[index] == 'P' || types[index] == 'I') {
          video[i].defers.push_back(index);
          break;
        }
        else {
          ++step;
        }
      }
    }
  }
  for (size_t i = 0; i < N; ++i) {
    std::cout << "video[" << i << "] has type = " << video[i].type
              << ", and id = " << video[i].id;
    
    if (video[i].defers.size()) {
       std::cout << ", and has depends = ";
       for (size_t j = 0; j < video[i].defers.size(); ++j) {
         std::cout << (video[i].defers[j])
                   << "(frame " << video[video[i].defers[j]].type << ") ";
       }
       std::cout << '\n';
    }
    else {
      std::cout << '\n';
    }
  }
}


void pipeline_2P_SS_264VideoFormat(size_t L, unsigned w) {

  tf::Executor executor(w);

  const size_t maxN = 512;

  std::vector<std::array<size_t, 2>> mybuffer(L);

  for(size_t N = 0; N <= maxN; N++) {
    // declare a x264 format video
    std::vector<Frames> video;
    construct_video(video, N);
    
    std::vector<size_t> collection1;
    std::vector<size_t> collection2;
    std::mutex mutex;

    tf::Taskflow taskflow;

    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [N, &collection1, &mybuffer, L, &video](auto& pf) mutable {
        if(pf.token() == N) {
          printf("Token %zu stops on line %zu\n", pf.token() ,pf.line());
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (video[pf.token()].type == 'I') {
                //printf("Stage 1 : token %zu is a I frame on line %zu\n", pf.token() ,pf.line());
                collection1.push_back(pf.token());
                mybuffer[pf.line()][pf.pipe()] = pf.token();           
              }
              else if (video[pf.token()].type == 'P') {
                //printf("Token %zu is a P frame", pf.token());
                size_t step = 1;
                size_t index = 0;
                while (static_cast<int>(pf.token()-step) >= 0) {
                  index = pf.token()-step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    pf.defer(index);
                    //printf(" defers to token %zu which is a %c frame\n", index, video[index].type);
                    break;
                  }
                  ++step;
                }
              }
              else if (video[pf.token()].type == 'B') {
                //printf("Token %zu is a B frame", pf.token());
                size_t step = 1;
                size_t index = 0;
                
                while (static_cast<int>(pf.token()-step) >= 0) {
                  index = pf.token()-step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    //printf(" defers to token %zu which is a %c frame\n", index, video[index].type);
                    pf.defer(index);
                    break;
                  }
                  ++step;
                }
                step = 1;
                while (pf.token()+step < N) {
                  index = pf.token()+step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    pf.defer(index);
                    //printf(" and token %zu which is a %c frame\n", index, video[index].type);
                    break;
                  }
                  ++step;
                }
              }
            break;

            case 1:
              //printf("Stage 1 : token %zu is deferred 1 time at line %zu\n", pf.token(), pf.line());
              collection1.push_back(pf.token());
              mybuffer[pf.line()][pf.pipe()] = pf.token();           
            break;
          }
        }
      }},

      tf::Pipe{tf::PipeType::SERIAL, [N, &mybuffer, &mutex, &collection2, L](auto& pf) mutable {
        {
          std::scoped_lock<std::mutex> lock(mutex);
          collection2.push_back(mybuffer[pf.line()][pf.pipe() - 1]);
        }
        printf("Stage 2 : token %zu at line %zu\n", pf.token(), pf.line());
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    auto test = taskflow.emplace([&](){
      printf("N = %zu and collection1.size() = %zu\n", N, collection1.size());
      for (size_t i = 0; i < collection1.size(); ++i) {
        printf("collection1[%zu]=%zu, collection2[%zu]=%zu\n", i, collection1[i], i, collection2[i]);
        REQUIRE(collection1[i] == collection2[i]);
      }

      for (size_t i = 0; i < N; ++i) {
        std::vector<size_t>::iterator it;
        std::vector<size_t>::iterator it_dep;

        size_t index_it, index_it_dep;
        
        if (video[i].defers.size()) {
          it = std::find(collection1.begin(), collection1.end(), i);
          index_it = std::distance(collection1.begin(), it);
          if (it == collection1.end()) {
            printf("Token %zu is missing\n", i);
          }
          for (size_t j = 0; j < video[i].defers.size(); ++j) {
            it_dep = std::find(collection1.begin(), collection1.end(), video[i].defers[j]);
            index_it_dep = std::distance(collection1.begin(), it_dep);
            
            REQUIRE(it != collection1.end());
            REQUIRE(it_dep != collection1.end());
            REQUIRE(index_it_dep < index_it);
          }
        }
      }
    }).name("test");

    pipeline.precede(test);

    executor.run_n(taskflow, 1, [&]() mutable {
      collection1.clear();
      collection2.clear();
    }).get();
  }
}

TEST_CASE("Pipeline.2P(SS).264VideoFormat.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(1,1);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(1,2);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(1,3);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(1,4);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(2,1);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(2,2);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(2,3);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(2,4);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(3,1);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(3,2);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(3,3);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(3,4);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(4,1);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(4,2);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(4,3);
}
TEST_CASE("Pipeline.2P(SS).264VideoFormat.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SS_264VideoFormat(4,4);
}



// ----------------------------------------------------------------------------
// two pipes (SP), L lines, W workers, mimic 264 frame patterns
// ----------------------------------------------------------------------------

void pipeline_2P_SP_264VideoFormat(size_t L, unsigned w) {

  tf::Executor executor(w);

  const size_t maxN = 512;

  std::vector<std::array<size_t, 2>> mybuffer(L);

  for(size_t N = 0; N <= maxN; N++) {
    // declare a x264 format video
    std::vector<Frames> video;
    construct_video(video, N);
    
    std::vector<size_t> collection1;
    std::vector<size_t> collection2;
    std::mutex mutex;

    tf::Taskflow taskflow;

    tf::Pipeline pl(
      L,
      tf::Pipe{tf::PipeType::SERIAL, [N, &collection1, &mybuffer, L, &video](auto& pf) mutable {
        if(pf.token() == N) {
          pf.stop();
          return;
        }
        else {
          switch(pf.deferred()) {
            case 0:
              if (video[pf.token()].type == 'I') {
                printf("Stage 1 : token %zu is a I frame on line %zu\n", pf.token() ,pf.line());
                collection1.push_back(pf.token());
                mybuffer[pf.line()][pf.pipe()] = pf.token();           
              }
              else if (video[pf.token()].type == 'P') {
                //printf("Token %zu is a P frame", pf.token());
                size_t step = 1;
                size_t index = 0;
                while (static_cast<int>(pf.token()-step) >= 0) {
                  index = pf.token()-step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    pf.defer(index);
                    //printf(" defers to token %zu which is a %c frame\n", index, video[index].type);
                    break;
                  }
                  ++step;
                }
              }
              else if (video[pf.token()].type == 'B') {
                //printf("Token %zu is a B frame", pf.token());
                size_t step = 1;
                size_t index = 0;
                
                while (static_cast<int>(pf.token()-step) >= 0) {
                  index = pf.token()-step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    //printf(" defers to token %zu which is a %c frame\n", index, video[index].type);
                    pf.defer(index);
                    break;
                  }
                  ++step;
                }
                step = 1;
                while (pf.token()+step < N) {
                  index = pf.token()+step;
                  if (video[index].type == 'P' || video[index].type == 'I') {
                    pf.defer(index);
                    //printf(" and token %zu which is a %c frame\n", index, video[index].type);
                    break;
                  }
                  ++step;
                }
              }
            break;

            case 1:
              printf("Stage 1 : token %zu is deferred 1 time at line %zu\n", pf.token(), pf.line());
              collection1.push_back(pf.token());
              mybuffer[pf.line()][pf.pipe()] = pf.token();           
            break;
          }
        }
      }},

      tf::Pipe{tf::PipeType::PARALLEL, [N, &mybuffer, &mutex, &collection2, L](auto& pf) mutable {
        {
          std::scoped_lock<std::mutex> lock(mutex);
          collection2.push_back(mybuffer[pf.line()][pf.pipe() - 1]);
        }
        printf("Stage 2 : token %zu at line %zu\n", pf.token(), pf.line());
      }}
    );

    auto pipeline = taskflow.composed_of(pl).name("module_of_pipeline");
    auto test = taskflow.emplace([&](){
      REQUIRE(collection1.size() == N);
      REQUIRE(collection2.size() == N);

      for (size_t i = 0; i < N; ++i) {
        std::vector<size_t>::iterator it;
        std::vector<size_t>::iterator it_dep;

        if (video[i].defers.size()) {
          it = std::find(collection1.begin(), collection1.end(), i);

          for (size_t j = 0; j < video[i].defers.size(); ++j) {
            it_dep = std::find(collection1.begin(), collection1.end(), video[i].defers[j]);
            
            REQUIRE(it != collection1.end());
            REQUIRE(it_dep != collection1.end());
          }
        }
      }
    }).name("test");

    pipeline.precede(test);

    executor.run_n(taskflow, 1, [&]() mutable {
      collection1.clear();
      collection2.clear();
    }).get();
  }
}

TEST_CASE("Pipeline.2P(SP).264VideoFormat.1L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(1,1);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.1L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(1,2);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.1L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(1,3);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.1L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(1,4);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.2L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(2,1);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.2L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(2,2);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.2L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(2,3);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.2L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(2,4);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.3L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(3,1);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.3L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(3,2);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.3L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(3,3);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.3L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(3,4);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.4L.1W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(4,1);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.4L.2W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(4,2);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.4L.3W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(4,3);
}
TEST_CASE("Pipeline.2P(SP).264VideoFormat.4L.4W" * doctest::timeout(300)) {
  pipeline_2P_SP_264VideoFormat(4,4);
}


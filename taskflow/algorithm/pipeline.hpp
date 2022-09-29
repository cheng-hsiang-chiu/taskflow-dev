#pragma once

#include "../taskflow.hpp"

/**
@file pipeline.hpp
@brief pipeline include file
*/

namespace tf {

// ----------------------------------------------------------------------------
// Class Definition: Pipeflow
// ----------------------------------------------------------------------------

/**
@class Pipeflow

@brief class to create a pipeflow object used by the pipe callable

Pipeflow represents a <i>scheduling token</i> in the pipeline scheduling
framework. A pipeflow is created by the pipeline scheduler at runtime to
pass to the pipe callable. Users can query the present statistics
of that scheduling token, including the line identifier, pipe identifier,
and token identifier, and build their application algorithms based on
these statistics.
At the first stage, users can explicitly call the stop method
to stop the pipeline scheduler.

@code{.cpp}
tf::Pipe{tf::PipeType::SERIAL, [](tf::Pipeflow& pf){
  std::cout << "token id=" << pf.token()
            << " at line=" << pf.line()
            << " at pipe=" << pf.pipe()
            << '\n';
}};
@endcode

Pipeflow can only be created privately by the tf::Pipeline and
be used through the pipe callable.
*/
class Pipeflow {

  template <typename... Ps>
  friend class Pipeline;

  template <typename P>
  friend class ScalablePipeline;

  public:

  /**
  @brief default constructor
  */
  Pipeflow() = default;

  /**
  @brief queries the line identifier of the present token
  */
  size_t line() const {
    return _line;
  }

  /**
  @brief queries the pipe identifier of the present token
  */
  size_t pipe() const {
    return _pipe;
  }

  /**
  @brief queries the token identifier
  */
  size_t token() const {
    return _token;
  }

  /**
  @brief stops the pipeline scheduling

  Only the first pipe can call this method to stop the pipeline.
  Others have no effect.
  */
  void stop() {
    if(_pipe != 0) {
      TF_THROW("only the first pipe can stop the token");
    }
    _stop = true;
  }

  // chiu
  Pipeflow(const Pipeflow& obj) {
    //_line = obj._line;
    //_pipe = obj._pipe;
    _token = obj._token;
    //_stop = obj._stop;
    _deferred = obj._deferred;
    _defers = obj._defers;
  }

  Pipeflow& operator=(Pipeflow obj) {
    //_line = obj._line;
    //_pipe = obj._pipe;
    _token = obj._token;
    //_stop = obj._stop;
    _deferred = obj._deferred;
    _defers = obj._defers;
    
    return *this;
  }
    
  size_t deferred() const {
    return _deferred;
  }

  void defer(size_t token) {
    _defers.push_back(token);
    //if (token < _token && 
    //    _dependeeTodepender.find(token) != _dependeeTodepender.end()) {
    //  _dependeeTodepender[token].push_back(_token);
    //  _defers.push_back(token);
    //  return;
    //}

    //if (token > _token) {
    //  _dependeeTodepender[token].push_back(_token);
    //  _defers.push_back(token);
    //  return;
    //}
  }
  // chiu
  
  private:

  size_t _line;
  size_t _pipe;
  size_t _token;
  bool   _stop;
  
  // chiu
  size_t _deferred;
  std::list<size_t> _defers;
  // chiu

};

// ----------------------------------------------------------------------------
// Class Definition: PipeType
// ----------------------------------------------------------------------------

/**
@enum PipeType

@brief enumeration of all pipe types
*/
enum class PipeType : int {
  /** @brief parallel type */
  PARALLEL = 1,
  /** @brief serial type */
  SERIAL   = 2
};

// ----------------------------------------------------------------------------
// Class Definition: Pipe
// ----------------------------------------------------------------------------

/**
@class Pipe

@brief class to create a pipe object for a pipeline stage

@tparam C callable type

A pipe represents a stage of a pipeline. A pipe can be either
@em parallel direction or @em serial direction (specified by tf::PipeType)
and is coupled with a callable to invoke by the pipeline scheduler.
The callable must take a referenced tf::Pipeflow object in the first argument:

@code{.cpp}
Pipe{PipeType::SERIAL, [](tf::Pipeflow&){}}
@endcode

The pipeflow object is used to query the statistics of a scheduling token
in the pipeline, such as pipe, line, and token numbers.
*/
template <typename C = std::function<void(tf::Pipeflow&)>>
class Pipe {

  template <typename... Ps>
  friend class Pipeline;

  template <typename P>
  friend class ScalablePipeline;

  public:

  /**
  @brief alias of the callable type
  */
  using callable_t = C;

  /**
  @brief default constructor
  */
  Pipe() = default;

  /**
  @brief constructs the pipe object

  @param d pipe type (tf::PipeType)
  @param callable callable type

  The constructor constructs a pipe with the given direction
  (tf::PipeType::SERIAL or tf::PipeType::PARALLEL) and the given callable. 
  The callable must take a referenced tf::Pipeflow object in the first argument.

  @code{.cpp}
  Pipe{PipeType::SERIAL, [](tf::Pipeflow&){}}
  @endcode

  When creating a pipeline, the direction of the first pipe must be serial
  (tf::PipeType::SERIAL).
  */
  Pipe(PipeType d, C&& callable) :
    _type{d}, _callable{std::forward<C>(callable)} {
  }

  /**
  @brief queries the type of the pipe

  Returns the type of the callable.
  */
  PipeType type() const {
    return _type;
  }

  /**
  @brief assigns a new type to the pipe

  @param type a tf::PipeType variable
  */
  void type(PipeType type) {
    _type = type;
  }

  /**
  @brief assigns a new callable to the pipe

  @tparam U callable type
  @param callable a callable object constructible from std::function<void(tf::Pipeflow&)>

  Assigns a new callable to the pipe with universal forwarding.
  */
  template <typename U>
  void callable(U&& callable) {
    _callable = std::forward<U>(callable);
  }

  private:

  PipeType _type;

  C _callable;
};

// ----------------------------------------------------------------------------
// Class Definition: Pipeline
// ----------------------------------------------------------------------------

/**
@class Pipeline

@brief class to create a pipeline scheduling framework

@tparam Ps pipe types

A pipeline is a composable graph object for users to create a
<i>pipeline scheduling framework</i> using a module task in a taskflow.
Unlike the conventional pipeline programming frameworks (e.g., Intel TBB),
%Taskflow's pipeline algorithm does not provide any data abstraction,
which often restricts users from optimizing data layouts in their applications,
but a flexible framework for users to customize their application data
atop our pipeline scheduling.
The following code creates a pipeline of four parallel lines to schedule
tokens through three serial pipes:

@code{.cpp}
tf::Taskflow taskflow;
tf::Executor executor;

const size_t num_lines = 4;
const size_t num_pipes = 3;

// create a custom data buffer
std::array<std::array<int, num_pipes>, num_lines> buffer;

// create a pipeline graph of four concurrent lines and three serial pipes
tf::Pipeline pipeline(num_lines,
  // first pipe must define a serial direction
  tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
    // generate only 5 scheduling tokens
    if(pf.token() == 5) {
      pf.stop();
    }
    // save the token id into the buffer
    else {
      buffer[pf.line()][pf.pipe()] = pf.token();
    }
  }},
  tf::Pipe{tf::PipeType::SERIAL, [&buffer] (tf::Pipeflow& pf) {
    // propagate the previous result to this pipe by adding one
    buffer[pf.line()][pf.pipe()] = buffer[pf.line()][pf.pipe()-1] + 1;
  }},
  tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf){
    // propagate the previous result to this pipe by adding one
    buffer[pf.line()][pf.pipe()] = buffer[pf.line()][pf.pipe()-1] + 1;
  }}
);

// build the pipeline graph using composition
tf::Task init = taskflow.emplace([](){ std::cout << "ready\n"; })
                        .name("starting pipeline");
tf::Task task = taskflow.composed_of(pipeline)
                        .name("pipeline");
tf::Task stop = taskflow.emplace([](){ std::cout << "stopped\n"; })
                        .name("pipeline stopped");

// create task dependency
init.precede(task);
task.precede(stop);

// run the pipeline
executor.run(taskflow).wait();
@endcode

The above example creates a pipeline graph that schedules five tokens over
four parallel lines in a circular fashion, as depicted below:

@code{.shell-session}
o -> o -> o
|    |    |
v    v    v
o -> o -> o
|    |    |
v    v    v
o -> o -> o
|    |    |
v    v    v
o -> o -> o
@endcode

At each pipe stage, the program propagates the result to the next pipe
by adding one to the result stored in a custom data storage, @c buffer.
The pipeline scheduler will generate five scheduling tokens and then stop.

Internally, tf::Pipeline uses std::tuple to store the given sequence of pipes.
The definition of each pipe can be different, completely decided by the compiler
to optimize the object layout.
After a pipeline is constructed, it is not possible to change its pipes.
If applications need to change these pipes, please use tf::ScalablePipeline.
*/
template <typename... Ps>
class Pipeline {

  //friend class Pipeflow;
  
  static_assert(sizeof...(Ps)>0, "must have at least one pipe");

  /**
  @private
  */
  struct Line {
    std::atomic<size_t> join_counter;
  };

  /**
  @private
  */
  struct PipeMeta {
    PipeType type;
  };

  public:

  /**
  @brief constructs a pipeline object

  @param num_lines the number of parallel lines
  @param ps a list of pipes

  Constructs a pipeline of up to @c num_lines parallel lines to schedule
  tokens through the given linear chain of pipes.
  The first pipe must define a serial direction (tf::PipeType::SERIAL)
  or an exception will be thrown.
  */
  Pipeline(size_t num_lines, Ps&&... ps);

  /**
  @brief constructs a pipeline object

  @param num_lines the number of parallel lines
  @param ps a tuple of pipes

  Constructs a pipeline of up to @c num_lines parallel lines to schedule
  tokens through the given linear chain of pipes.
  The first pipe must define a serial direction (tf::PipeType::SERIAL)
  or an exception will be thrown.
  */
  Pipeline(size_t num_lines, std::tuple<Ps...>&& ps);

  /**
  @brief queries the number of parallel lines

  The function returns the number of parallel lines given by the user
  upon the construction of the pipeline.
  The number of lines represents the maximum parallelism this pipeline
  can achieve.
  */
  size_t num_lines() const noexcept;

  /**
  @brief queries the number of pipes

  The Function returns the number of pipes given by the user
  upon the construction of the pipeline.
  */
  constexpr size_t num_pipes() const noexcept;

  /**
  @brief resets the pipeline

  Resetting the pipeline to the initial state. After resetting a pipeline,
  its token identifier will start from zero as if the pipeline was just
  constructed.
  */
  void reset();

  /**
  @brief queries the number of generated tokens in the pipeline

  The number represents the total scheduling tokens that has been
  generated by the pipeline so far.
  */
  size_t num_tokens() const noexcept;

  /**
  @brief obtains the graph object associated with the pipeline construct

  This method is primarily used as an opaque data structure for creating
  a module task of the this pipeline.
  */
  Graph& graph();

  private:

  Graph _graph;

  size_t _num_tokens;

  std::tuple<Ps...> _pipes;
  std::array<PipeMeta, sizeof...(Ps)> _meta;
  std::vector<std::array<Line, sizeof...(Ps)>> _lines;
  std::vector<Task> _tasks;
  std::vector<Pipeflow> _pipeflows;

  // chiu
  std::queue<Pipeflow> _ready_tokens;
  //std::list<Pipeflow> _deferred_tokens;

  //std::unordered_map<Pipeflow&, Pipeflow&> _deferred_dependeeTodepender;
  //std::unordered_map<Pipeflow&, Pipeflow&> _deferred_dependerTodependee;
  std::unordered_map<size_t, std::vector<size_t>> _dependeeTodepender;
  std::unordered_map<size_t, Pipeflow> _tokenToPipeflow;
  
  //std::mutex _mutex;
  // chiu
  
  template <size_t... I>
  auto _gen_meta(std::tuple<Ps...>&&, std::index_sequence<I...>);

  void _on_pipe(Pipeflow&, Runtime&);
  void _build();
};

// constructor
template <typename... Ps>
Pipeline<Ps...>::Pipeline(size_t num_lines, Ps&&... ps) :
  _pipes     {std::make_tuple(std::forward<Ps>(ps)...)},
  _meta      {PipeMeta{ps.type()}...},
  _lines     (num_lines),
  _tasks     (num_lines + 1),
  _pipeflows (num_lines) {

  if(num_lines == 0) {
    TF_THROW("must have at least one line");
  }

  if(std::get<0>(_pipes).type() != PipeType::SERIAL) {
    TF_THROW("first pipe must be serial");
  }

  reset();
  _build();
}

// constructor
template <typename... Ps>
Pipeline<Ps...>::Pipeline(size_t num_lines, std::tuple<Ps...>&& ps) :
  _pipes     {std::forward<std::tuple<Ps...>>(ps)},
  _meta      {_gen_meta(
    std::forward<std::tuple<Ps...>>(ps), std::make_index_sequence<sizeof...(Ps)>{}
  )},
  _lines     (num_lines),
  _tasks     (num_lines + 1),
  _pipeflows (num_lines) {

  if(num_lines == 0) {
    TF_THROW("must have at least one line");
  }

  if(std::get<0>(_pipes).type() != PipeType::SERIAL) {
    TF_THROW("first pipe must be serial");
  }

  reset();
  _build();
}

// Function: _get_meta
template <typename... Ps>
template <size_t... I>
auto Pipeline<Ps...>::_gen_meta(std::tuple<Ps...>&& ps, std::index_sequence<I...>) {
  return std::array{PipeMeta{std::get<I>(ps).type()}...};
}

// Function: num_lines
template <typename... Ps>
size_t Pipeline<Ps...>::num_lines() const noexcept {
  return _pipeflows.size();
}

// Function: num_pipes
template <typename... Ps>
constexpr size_t Pipeline<Ps...>::num_pipes() const noexcept {
  return sizeof...(Ps);
}

// Function: num_tokens
template <typename... Ps>
size_t Pipeline<Ps...>::num_tokens() const noexcept {
  return _num_tokens;
}

// Function: graph
template <typename... Ps>
Graph& Pipeline<Ps...>::graph() {
  return _graph;
}

// Function: reset
template <typename... Ps>
void Pipeline<Ps...>::reset() {

  _num_tokens = 0;

  for(size_t l = 0; l<num_lines(); l++) {
    _pipeflows[l]._pipe = 0;
    _pipeflows[l]._line = l;
    
    // chiu
    _pipeflows[l]._deferred = 0;
    _pipeflows[l]._defers.clear();
    // chiu
  }

  _lines[0][0].join_counter.store(0, std::memory_order_relaxed);

  for(size_t l=1; l<num_lines(); l++) {
    for(size_t f=1; f<num_pipes(); f++) {
      _lines[l][f].join_counter.store(
        static_cast<size_t>(_meta[f].type), std::memory_order_relaxed
      );
    }
  }

  for(size_t f=1; f<num_pipes(); f++) {
    _lines[0][f].join_counter.store(1, std::memory_order_relaxed);
  }

  for(size_t l=1; l<num_lines(); l++) {
    _lines[l][0].join_counter.store(
      static_cast<size_t>(_meta[0].type) - 1, std::memory_order_relaxed
    );
  }
}

// Procedure: _on_pipe
template <typename... Ps>
void Pipeline<Ps...>::_on_pipe(Pipeflow& pf, Runtime& rt) {
  visit_tuple([&](auto&& pipe){
    using callable_t = typename std::decay_t<decltype(pipe)>::callable_t;
    if constexpr (std::is_invocable_v<callable_t, Pipeflow&>) {
      pipe._callable(pf);
    }
    else if constexpr(std::is_invocable_v<callable_t, Pipeflow&, Runtime&>) {
      pipe._callable(pf, rt);
    }
    else {
      static_assert(dependent_false_v<callable_t>, "un-supported pipe callable type");
    }
  }, _pipes, pf._pipe);
}

// Procedure: _build
template <typename... Ps>
void Pipeline<Ps...>::_build() {

  using namespace std::literals::string_literals;

  FlowBuilder fb(_graph);

  // init task
  _tasks[0] = fb.emplace([this]() {
    return static_cast<int>(_num_tokens % num_lines());
  }).name("cond");

  // line task
  for(size_t l = 0; l < num_lines(); l++) {

    _tasks[l + 1] = fb.emplace([this, l] (tf::Runtime& rt) mutable {

      auto pf = &_pipeflows[l];

      pipeline:

      _lines[pf->_line][pf->_pipe].join_counter.store(
        static_cast<size_t>(_meta[pf->_pipe].type), std::memory_order_relaxed
      );
      
      pf->_deferred = 0;

      if (pf->_pipe == 0) {
        // chiu

        {
          //std::lock_guard lock{_mutex};
          if (!_ready_tokens.empty()) {
            //printf("ready_tokens.size() = %zu\n", _ready_tokens.size());
            //Pipeflow temp = _ready_tokens.front();
            //printf("token %zu has ready_tokens with token %zu\n", pf->_token, temp._token);
            pf->_deferred = _ready_tokens.front()._deferred;
            pf->_token = _ready_tokens.front()._token;
            _ready_tokens.pop();
          }
        }
        
        if (pf->_deferred == 0) {
          pf->_token = _num_tokens;
        }
        
        
        // chiu
        //
      pipeline_on_pipe: 
     
        size_t ori_pf_deferred = pf->_deferred;
        
        if (pf->_stop = false, _on_pipe(*pf, rt); pf->_stop == true) {
          // here, the pipeline is not stopped yet because other
          // lines of tasks may still be running their last stages
          return;
        }
        //printf("_num_tokens = %zu, pf->_token = %zu\n", _num_tokens, pf->_token);
        // chiu
        if (_num_tokens == pf->_token) {
          ++_num_tokens;
          //printf("_num_tokens == pf->_token, ++_num_tokens\n");
        }
       
        {
          //std::lock_guard lock{_mutex};
          //pf->_deferred = pf->_defers.size()?pf->_deferred+1:pf->_deferred;
          //printf("Token %zu has defers size of %zu\n", pf->_token, pf->_defers.size()); 
          //printf("1.defers[0] = %zu\n", *(pf->_defers.begin())); 
          
          if (pf->_defers.size()) {
            //printf("1st check pf->_defers.size() != 0\n");
            ++pf->_deferred;
            for (std::list<size_t>::iterator it = pf->_defers.begin();
              it != pf->_defers.end();) {
          
              //printf("it = %zu\n", *(it)); 
              if (*it >= _num_tokens) {
                _dependeeTodepender[*it].push_back(pf->_token);
                ++it;
              }
              else {
                auto pit = _tokenToPipeflow.find(*it);
                
                //auto pit = std::find_if(
                //  _deferred_tokens.begin(),
                //  _deferred_tokens.end(),
                //  [&it](Pipeflow temp){ return temp._token == *it; }
                //);

                //if (pit != _deferred_tokens.end()) {
                //if (pit != _dependeeTodepender.end()) {
                if (pit != _tokenToPipeflow.end()) {
                  _dependeeTodepender[*it].push_back(pf->_token);
                  ++it;  
                }
                else {
                  it = pf->_defers.erase(it);
                }
              }
            }
          }
          
          if (pf->_defers.size()) {
            //printf("2nd check, pd->_defers.size() != 0\n");
            //++pf->_deferred;
            //printf("%zu has deferred = %zu\n", pf->_token, pf->_deferred); 
            //printf("Push token %zu in unresolved queuen\n", pf->_token); 
            //_deferred_tokens.push_back(*pf);
            
            _tokenToPipeflow[pf->_token] = *pf;
            
            pf->_defers.clear();
            pf->_deferred = 0;
            goto pipeline;
          }
          else if (ori_pf_deferred != pf->_deferred) {
            //printf("ori_pf_deferred != pf->_deferred\n");
            goto pipeline_on_pipe;
          }
        }

        // chiu
      }
      else {
        _on_pipe(*pf, rt);
        // chiu
        if (pf->_pipe == num_pipes()-1) {
          pf->_deferred = 0;
        }
        // chiu
      }
      { 
        //std::lock_guard lock{_mutex};
        if (pf->_pipe == 0) {
          //for (auto& t : _deferred_tokens) {
          //  std::list<size_t>::iterator it = std::find(t._defers.begin(), t._defers.end(), pf->_token);
          //  if (it != t._defers.end()) {
          //    t._defers.erase(it);
          //    if (t._defers.size() == 0) {
          //      _ready_tokens.push(t);
          //    }
          //  }
          //}
         
          for (size_t i = 0; i < _dependeeTodepender[pf->_token].size(); ++i) {
            //printf("here\n");
            size_t target = _dependeeTodepender[pf->_token][i];
            std::list<size_t>::iterator it = std::find(
              _tokenToPipeflow[target]._defers.begin(),
              _tokenToPipeflow[target]._defers.end(), pf->_token);
            _tokenToPipeflow[target]._defers.erase(it);
            if (_tokenToPipeflow[target]._defers.size() == 0) {
              _ready_tokens.push(_tokenToPipeflow[target]); 
            }
          }
          _dependeeTodepender.erase(pf->_token);
        }
      }

      size_t c_f = pf->_pipe;
      size_t n_f = (pf->_pipe + 1) % num_pipes();
      size_t n_l = (pf->_line + 1) % num_lines();

      pf->_pipe = n_f;

      // ---- scheduling starts here ----
      // Notice that the shared variable f must not be changed after this
      // point because it can result in data race due to the following
      // condition:
      //
      // a -> b
      // |    |
      // v    v
      // c -> d
      //
      // d will be spawned by either c or b, so if c changes f but b spawns d
      // then data race on f will happen

      //{ 
      //  std::lock_guard lock{_mutex};
      //  if (c_f == 0) {
      //    for (auto& t : _deferred_tokens) {
      //      std::list<size_t>::iterator it = std::find(t._defers.begin(), t._defers.end(), pf->_token);
      //      if (it != t._defers.end()) {
      //        t._defers.erase(it);
      //        if (t._defers.size() == 0) {
      //          _ready_tokens.push(t);
      //        }
      //      }
      //    }
      //    //printf("From token %zu : Ready queue has %zu tokens\n", pf->_token, _ready_tokens.size()); 
      //    //if (!_ready_tokens.empty()) {
      //    //  Pipeflow temp = _ready_tokens.front();
      //    //  //printf("token %zu has ready_tokens with token %zu\n", pf->_token, temp._token);
      //    //  temp._line = n_l;
      //    //  temp._pipe = 0;
      //    //  _ready_tokens.pop();
      //    //  //goto pipeline;
      //    //  _pipeflows[n_l] = temp;
      //    //}
      //  }
      //}
      // chiu
      std::array<int, 2> retval;
      size_t n = 0;

      // downward dependency
      if(_meta[c_f].type == PipeType::SERIAL &&
         _lines[n_l][c_f].join_counter.fetch_sub(
           1, std::memory_order_acq_rel) == 1
        ) {
        retval[n++] = 1;
      }

      // forward dependency
      if(_lines[pf->_line][n_f].join_counter.fetch_sub(
          1, std::memory_order_acq_rel) == 1
        ) {
        retval[n++] = 0;
      }
      
      // notice that the task index starts from 1
      switch(n) {
        case 2: {
          //{
          //  std::lock_guard lock{_mutex};
          //  if (c_f == 0 && !_ready_tokens.empty()) {
          //    Pipeflow temp = _ready_tokens.front();
          //    //printf("token %zu has ready_tokens with token %zu\n", pf->_token, temp._token);
          //    temp._line = n_l;
          //    temp._pipe = 0;
          //    _ready_tokens.pop();
          //    //goto pipeline;
          //    _pipeflows[n_l] = temp;
          //  }
          //}
          //{
          //  std::lock_guard lock{_mutex};
          //  if (c_f == num_pipes()-1 && !_ready_tokens.empty()) {
          //    Pipeflow temp = _ready_tokens.front();
          //    //printf("case = 2, current pipe = %zu, and new token = %zu\n", c_f, temp._token);
          //    //temp._line = pf->_line;
          //    //temp._pipe = 0;
          //    _ready_tokens.pop();
          //    //goto pipeline;
          //    pf->_pipe = 0;
          //    pf->_deferred = temp._deferred;
          //    pf->_token = temp._token;
          //    pf->_defers.clear();
          //  }
          //}
          rt.schedule(_tasks[n_l+1]);
          goto pipeline;
        }
        case 1: {
          // downward dependency 
          if (retval[0] == 1) {
            //{
            //  std::lock_guard lock{_mutex};
            //  if (c_f == 0 && !_ready_tokens.empty()) {
            //    Pipeflow temp = _ready_tokens.front();
            //    //printf("token %zu has ready_tokens with token %zu\n", pf->_token, temp._token);
            //    temp._line = n_l;
            //    temp._pipe = 0;
            //    _ready_tokens.pop();
            //    //goto pipeline;
            //    _pipeflows[n_l] = temp;
            //  }
            //}
            pf = &_pipeflows[n_l];
          }
          // forward dependency
          //else {
          //  {
          //    std::lock_guard lock{_mutex};
          //    if (c_f == num_pipes()-1 && !_ready_tokens.empty()) {
          //      Pipeflow temp = _ready_tokens.front();
          //      //printf("case = 1, current pipe = %zu, and new token = %zu\n", c_f, temp._token);
          //      //temp._line = pf->_line;
          //      //temp._pipe = 0;
          //      _ready_tokens.pop();
          //      //goto pipeline;
          //      //pf = &temp;
          //      pf->_pipe = 0;
          //      pf->_deferred = temp._deferred;
          //      pf->_token = temp._token;
          //      pf->_defers.clear();
          //    }
          //  }
          //}
          goto pipeline;
        }
      }
    }).name("rt-"s + std::to_string(l));

    _tasks[0].precede(_tasks[l+1]);
  }
}

// ----------------------------------------------------------------------------
// Class Definition: ScalablePipeline
// ----------------------------------------------------------------------------

/**
@class ScalablePipeline

@brief class to create a scalable pipeline object

@tparam P type of the iterator to a range of pipes

A scalable pipeline is a composable graph object for users to create a
<i>pipeline scheduling framework</i> using a module task in a taskflow.
Unlike tf::Pipeline that instantiates all pipes upon the construction time,
tf::ScalablePipeline allows variable assignments of pipes using range iterators.
Users can also reset a scalable pipeline to a different range of pipes
between runs. The following code creates a scalable pipeline of four
parallel lines to schedule tokens through three serial pipes in a custom storage,
then resetting the pipeline to a new range of five serial pipes:

@code{.cpp}
tf::Taskflow taskflow("pipeline");
tf::Executor executor;

const size_t num_lines = 4;

// create data storage
std::array<int, num_lines> buffer;

// define the pipe callable
auto pipe_callable = [&buffer] (tf::Pipeflow& pf) mutable {
  switch(pf.pipe()) {
    // first stage generates only 5 scheduling tokens and saves the
    // token number into the buffer.
    case 0: {
      if(pf.token() == 5) {
        pf.stop();
      }
      else {
        printf("stage 1: input token = %zu\n", pf.token());
        buffer[pf.line()] = pf.token();
      }
      return;
    }
    break;

    // other stages propagate the previous result to this pipe and
    // increment it by one
    default: {
      printf(
        "stage %zu: input buffer[%zu] = %d\n", pf.pipe(), pf.line(), buffer[pf.line()]
      );
      buffer[pf.line()] = buffer[pf.line()] + 1;
    }
    break;
  }
};

// create a vector of three pipes
std::vector< tf::Pipe<std::function<void(tf::Pipeflow&)>> > pipes;

for(size_t i=0; i<3; i++) {
  pipes.emplace_back(tf::PipeType::SERIAL, pipe_callable);
}

// create a pipeline of four parallel lines based on the given vector of pipes
tf::ScalablePipeline pl(num_lines, pipes.begin(), pipes.end());

// build the pipeline graph using composition
tf::Task init = taskflow.emplace([](){ std::cout << "ready\n"; })
                        .name("starting pipeline");
tf::Task task = taskflow.composed_of(pl)
                        .name("pipeline");
tf::Task stop = taskflow.emplace([](){ std::cout << "stopped\n"; })
                        .name("pipeline stopped");

// create task dependency
init.precede(task);
task.precede(stop);

// dump the pipeline graph structure (with composition)
taskflow.dump(std::cout);

// run the pipeline
executor.run(taskflow).wait();

// reset the pipeline to a new range of five pipes and starts from
// the initial state (i.e., token counts from zero)
for(size_t i=0; i<2; i++) {
  pipes.emplace_back(tf::PipeType::SERIAL, pipe_callable);
}
pl.reset(pipes.begin(), pipes.end());

executor.run(taskflow).wait();
@endcode

The above example creates a pipeline graph that schedules five tokens over
four parallel lines in a circular fashion, first going through three serial pipes
and then five serial pipes:

@code{.shell-session}
# initial construction of three serial pipes
o -> o -> o
|    |    |
v    v    v
o -> o -> o
|    |    |
v    v    v
o -> o -> o
|    |    |
v    v    v
o -> o -> o

# resetting to a new range of five serial pipes
o -> o -> o -> o -> o
|    |    |    |    |
v    v    v    v    v
o -> o -> o -> o -> o
|    |    |    |    |
v    v    v    v    v
o -> o -> o -> o -> o
|    |    |    |    |
v    v    v    v    v
o -> o -> o -> o -> o
@endcode

Each pipe has the same type of `%tf::Pipe<%std::function<void(%tf::Pipeflow&)>>`
and is kept in a vector that is amenable to change.
We construct the scalable pipeline using two range iterators pointing to the
beginning and the end of the vector.
At each pipe stage, the program propagates the result to the next pipe
by adding one to the result stored in a custom data storage, @c buffer.
The pipeline scheduler will generate five scheduling tokens and then stop.

A scalable pipeline is move-only.
*/
template <typename P>
class ScalablePipeline {

  /**
  @private
  */
  struct Line {
    std::atomic<size_t> join_counter;
  };

  public:

  /**
  @brief pipe type
  */
  using pipe_t = typename std::iterator_traits<P>::value_type;

  /**
  @brief default constructor
  */
  ScalablePipeline() = default;

  /**
  @brief constructs an empty scalable pipeline object

  @param num_lines the number of parallel lines

  An empty scalable pipeline does not have any pipes.
  The pipeline needs to be reset to a valid range of pipes
  before running.
  */
  ScalablePipeline(size_t num_lines);

  /**
  @brief constructs a scalable pipeline object

  @param num_lines the number of parallel lines
  @param first iterator to the beginning of the range
  @param last iterator to the end of the range

  Constructs a pipeline from the given range of pipes specified in
  <tt>[first, last)</tt> using @c num_lines parallel lines.
  The first pipe must define a serial direction (tf::PipeType::SERIAL)
  or an exception will be thrown.

  Internally, the scalable pipeline copies the iterators
  from the specified range. Those pipe callables pointed to by
  these iterators must remain valid during the execution of the pipeline.
  */
  ScalablePipeline(size_t num_lines, P first, P last);

  /**
  @brief disabled copy constructor
  */
  ScalablePipeline(const ScalablePipeline&) = delete;

  /**
  @brief move constructor

  Constructs a pipeline from the given @c rhs using move semantics
  (i.e. the data in @c rhs is moved into this pipeline).
  After the move, @c rhs is in a state as if it is just constructed.
  The behavior is undefined if @c rhs is running during the move.
  */
  ScalablePipeline(ScalablePipeline&& rhs);

  /**
  @brief disabled copy assignment operator
  */
  ScalablePipeline& operator = (const ScalablePipeline&) = delete;

  /**
  @brief move constructor

  Replaces the contents with those of @c rhs using move semantics
  (i.e. the data in @c rhs is moved into this pipeline).
  After the move, @c rhs is in a state as if it is just constructed.
  The behavior is undefined if @c rhs is running during the move.
  */
  ScalablePipeline& operator = (ScalablePipeline&& rhs);

  /**
  @brief queries the number of parallel lines

  The function returns the number of parallel lines given by the user
  upon the construction of the pipeline.
  The number of lines represents the maximum parallelism this pipeline
  can achieve.
  */
  size_t num_lines() const noexcept;

  /**
  @brief queries the number of pipes

  The Function returns the number of pipes given by the user
  upon the construction of the pipeline.
  */
  size_t num_pipes() const noexcept;

  /**
  @brief resets the pipeline

  Resets the pipeline to the initial state. After resetting a pipeline,
  its token identifier will start from zero.
  */
  void reset();

  /**
  @brief resets the pipeline with a new range of pipes

  @param first iterator to the beginning of the range
  @param last iterator to the end of the range

  The member function assigns the pipeline to a new range of pipes
  specified in <tt>[first, last)</tt> and resets the pipeline to the
  initial state. After resetting a pipeline, its token identifier will
  start from zero.

  Internally, the scalable pipeline copies the iterators
  from the specified range. Those pipe callables pointed to by
  these iterators must remain valid during the execution of the pipeline.
  */
  void reset(P first, P last);

  /**
  @brief resets the pipeline to a new line number and a
         new range of pipes

  @param num_lines number of parallel lines
  @param first iterator to the beginning of the range
  @param last iterator to the end of the range

  The member function resets the pipeline to a new number of
  parallel lines and a new range of pipes specified in
  <tt>[first, last)</tt>, as if the pipeline is just constructed.
  After resetting a pipeline, its token identifier will start from zero.

  Internally, the scalable pipeline copies the iterators
  from the specified range. Those pipe callables pointed to by
  these iterators must remain valid during the execution of the pipeline.
  */
  void reset(size_t num_lines, P first, P last);

  /**
  @brief queries the number of generated tokens in the pipeline

  The number represents the total scheduling tokens that has been
  generated by the pipeline so far.
  */
  size_t num_tokens() const noexcept;

  /**
  @brief obtains the graph object associated with the pipeline construct

  This method is primarily used as an opaque data structure for creating
  a module task of the this pipeline.
  */
  Graph& graph();

  private:

  Graph _graph;

  size_t _num_tokens{0};

  std::vector<P> _pipes;
  std::vector<Task> _tasks;
  std::vector<Pipeflow> _pipeflows;
  std::unique_ptr<Line[]> _lines;

  void _on_pipe(Pipeflow&, Runtime&);
  void _build();

  Line& _line(size_t, size_t);
};

// constructor
template <typename P>
ScalablePipeline<P>::ScalablePipeline(size_t num_lines) :
  _tasks     (num_lines + 1),
  _pipeflows (num_lines) {

  if(num_lines == 0) {
    TF_THROW("must have at least one line");
  }

  _build();
}

// constructor
template <typename P>
ScalablePipeline<P>::ScalablePipeline(size_t num_lines, P first, P last) :
  _tasks     (num_lines + 1),
  _pipeflows (num_lines) {

  if(num_lines == 0) {
    TF_THROW("must have at least one line");
  }

  reset(first, last);
  _build();
}

// move constructor
template <typename P>
ScalablePipeline<P>::ScalablePipeline(ScalablePipeline&& rhs) :
  _graph      {std::move(rhs._graph)},
  _num_tokens {rhs._num_tokens},
  _pipes      {std::move(rhs._pipes)},
  _tasks      {std::move(rhs._tasks)},
  _pipeflows  {std::move(rhs._pipeflows)},
  _lines      {std::move(rhs._lines)} {

  rhs._num_tokens = 0;
}

// move assignment operator
template <typename P>
ScalablePipeline<P>& ScalablePipeline<P>::operator = (ScalablePipeline&& rhs) {
  _graph      = std::move(rhs._graph);
  _num_tokens = rhs._num_tokens;
  _pipes      = std::move(rhs._pipes);
  _tasks      = std::move(rhs._tasks);
  _pipeflows  = std::move(rhs._pipeflows);
  _lines      = std::move(rhs._lines);
  rhs._num_tokens = 0;
  return *this;
}

// Function: num_lines
template <typename P>
size_t ScalablePipeline<P>::num_lines() const noexcept {
  return _pipeflows.size();
}

// Function: num_pipes
template <typename P>
size_t ScalablePipeline<P>::num_pipes() const noexcept {
  return _pipes.size();
}

// Function: num_tokens
template <typename P>
size_t ScalablePipeline<P>::num_tokens() const noexcept {
  return _num_tokens;
}

// Function: graph
template <typename P>
Graph& ScalablePipeline<P>::graph() {
  return _graph;
}

// Function: _line
template <typename P>
typename ScalablePipeline<P>::Line& ScalablePipeline<P>::_line(size_t l, size_t p) {
  return _lines[l*num_pipes() + p];
}

template <typename P>
void ScalablePipeline<P>::reset(size_t num_lines, P first, P last) {

  if(num_lines == 0) {
    TF_THROW("must have at least one line");
  }

  _graph.clear();
  _tasks.resize(num_lines + 1);
  _pipeflows.resize(num_lines);

  reset(first, last);

  _build();
}

// Function: reset
template <typename P>
void ScalablePipeline<P>::reset(P first, P last) {

  size_t num_pipes = static_cast<size_t>(std::distance(first, last));

  if(num_pipes == 0) {
    TF_THROW("pipeline cannot be empty");
  }

  if(first->type() != PipeType::SERIAL) {
    TF_THROW("first pipe must be serial");
  }

  _pipes.resize(num_pipes);

  size_t i=0;
  for(auto itr = first; itr != last; itr++) {
    _pipes[i++] = itr;
  }

  _lines = std::make_unique<Line[]>(num_lines() * _pipes.size());

  reset();
}

// Function: reset
template <typename P>
void ScalablePipeline<P>::reset() {

  _num_tokens = 0;

  for(size_t l = 0; l<num_lines(); l++) {
    _pipeflows[l]._pipe = 0;
    _pipeflows[l]._line = l;
  }

  _line(0, 0).join_counter.store(0, std::memory_order_relaxed);

  for(size_t l=1; l<num_lines(); l++) {
    for(size_t f=1; f<num_pipes(); f++) {
      _line(l, f).join_counter.store(
        static_cast<size_t>(_pipes[f]->type()), std::memory_order_relaxed
      );
    }
  }

  for(size_t f=1; f<num_pipes(); f++) {
    _line(0, f).join_counter.store(1, std::memory_order_relaxed);
  }

  for(size_t l=1; l<num_lines(); l++) {
    _line(l, 0).join_counter.store(
      static_cast<size_t>(_pipes[0]->type()) - 1, std::memory_order_relaxed
    );
  }
}

// Procedure: _on_pipe
template <typename P>
void ScalablePipeline<P>::_on_pipe(Pipeflow& pf, Runtime& rt) {
    
  using callable_t = typename pipe_t::callable_t;

  if constexpr (std::is_invocable_v<callable_t, Pipeflow&>) {
    _pipes[pf._pipe]->_callable(pf);
  }
  else if constexpr(std::is_invocable_v<callable_t, Pipeflow&, Runtime&>) {
    _pipes[pf._pipe]->_callable(pf, rt);
  }
  else {
    static_assert(dependent_false_v<callable_t>, "un-supported pipe callable type");
  }
}

// Procedure: _build
template <typename P>
void ScalablePipeline<P>::_build() {

  using namespace std::literals::string_literals;

  FlowBuilder fb(_graph);

  // init task
  _tasks[0] = fb.emplace([this]() {
    return static_cast<int>(_num_tokens % num_lines());
  }).name("cond");

  // line task
  for(size_t l = 0; l < num_lines(); l++) {

    _tasks[l + 1] = fb.emplace([this, l] (tf::Runtime& rt) mutable {

      auto pf = &_pipeflows[l];

      pipeline:

      _line(pf->_line, pf->_pipe).join_counter.store(
        static_cast<size_t>(_pipes[pf->_pipe]->type()), std::memory_order_relaxed
      );

      if (pf->_pipe == 0) {
        pf->_token = _num_tokens;
        if (pf->_stop = false, _on_pipe(*pf, rt); pf->_stop == true) {
          // here, the pipeline is not stopped yet because other
          // lines of tasks may still be running their last stages
          return;
        }
        ++_num_tokens;
      }
      else {
        _on_pipe(*pf, rt);
      }

      size_t c_f = pf->_pipe;
      size_t n_f = (pf->_pipe + 1) % num_pipes();
      size_t n_l = (pf->_line + 1) % num_lines();

      pf->_pipe = n_f;

      // ---- scheduling starts here ----
      // Notice that the shared variable f must not be changed after this
      // point because it can result in data race due to the following
      // condition:
      //
      // a -> b
      // |    |
      // v    v
      // c -> d
      //
      // d will be spawned by either c or b, so if c changes f but b spawns d
      // then data race on f will happen

      std::array<int, 2> retval;
      size_t n = 0;

      // downward dependency
      if(_pipes[c_f]->type() == PipeType::SERIAL &&
         _line(n_l, c_f).join_counter.fetch_sub(
           1, std::memory_order_acq_rel) == 1
        ) {
        retval[n++] = 1;
      }

      // forward dependency
      if(_line(pf->_line, n_f).join_counter.fetch_sub(
          1, std::memory_order_acq_rel) == 1
        ) {
        retval[n++] = 0;
      }

      // notice that the task index starts from 1
      switch(n) {
        case 2: {
          rt.schedule(_tasks[n_l+1]);
          goto pipeline;
        }
        case 1: {
          if (retval[0] == 1) {
            pf = &_pipeflows[n_l];
          }
          goto pipeline;
        }
      }
    }).name("rt-"s + std::to_string(l));

    _tasks[0].precede(_tasks[l+1]);
  }
}

}  // end of namespace tf -----------------------------------------------------






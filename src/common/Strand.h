#pragma once

#include "common/Assert.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

namespace dibeast {

using IoStrand = boost::asio::strand<boost::asio::io_context::executor_type>;

template <typename Executor>
bool runningInThisThread(const boost::asio::strand<Executor>& strand) {
  return strand.running_in_this_thread();
}

// I/O objects keep their executor type-erased; recover the strand made by make_strand(io_context).
inline bool runningInThisThread(const boost::asio::any_io_executor& executor) {
  const auto* strand = executor.target<IoStrand>();

  return strand != nullptr && strand->running_in_this_thread();
}

}  // namespace dibeast

#define DIBEAST_ASSERT_ON_STRAND(executor) DIBEAST_ASSERT(::dibeast::runningInThisThread(executor))

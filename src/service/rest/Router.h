#pragma once

#include "dibeast/Result.h"
#include "dibeast/Types.h"
#include "service/rest/RequestContext.h"

#include <boost/beast/http/verb.hpp>
#include <boost/url/params_view.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace dibeast::service {

/**
 @brief Maps method + path pattern (e.g. /apps/{name}/run) to a handler.
 **/
class Router {
 public:
  struct Match {
    std::vector<std::string_view> captures;
    boost::urls::params_view query;
  };

  using Respond = std::move_only_function<void(Result<Reply>)>;
  using Handler = std::function<void(const Request&, const RequestContext&, const Match&, Respond)>;

  void add(boost::beast::http::verb method, std::string_view pattern, Handler handler);

  void route(const Request& request, const RequestContext& context, Respond respond) const;

 private:
  struct Route {
    boost::beast::http::verb method;
    std::vector<std::string> segments;
    Handler handler;
  };

  std::vector<Route> routes_;
};

}  // namespace dibeast::service

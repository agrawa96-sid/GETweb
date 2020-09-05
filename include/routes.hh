#ifndef INCLUDE_ROUTES_HH_
#define INCLUDE_ROUTES_HH_

#include <functional>
#include <string>
#include <utility>

#include "http_messages.hh"

// You may find handling routes by using these functions (that you need to implement) helpful

std::string handle_cgi_bin(const HttpRequest& request, int sock);
std::string handle_cgi_bin_post(const HttpRequest& request);
HttpResponse handle_htdocs(const HttpRequest& request);
HttpResponse handle_default(const HttpRequest& request);

typedef std::function<HttpResponse(const HttpRequest&)> Route_Handler_t;

typedef std::pair<const std::string, const Route_Handler_t> Route_t;

#endif  // INCLUDE_ROUTES_HH_

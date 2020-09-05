#include "http_messages.hh"

// You may find implementing this function and using it in server.cc helpful

HttpResponse handle_htdocs(const HttpRequest& request) {
  HttpResponse response;
  response.http_version = request.http_version;
  // TODO: Task 1.3
  // Get the request URI, verify the file exists and serve it
  return response;
}

// Copyright 2020 Sidhartha Agrawal | Spring 2020 | CS 252 Lab 5

#include <string.h>

#include "http_messages.hh"

const std::map <
    const int,
        const std::string > default_status_reasons = {
            {
                100,
                "Continue"
            },
            {
                101,
                "Switching Protocols"
            },
            {
                200,
                "OK"
            },
            {
                201,
                "Created"
            },
            {
                202,
                "Accepted"
            },
            {
                203,
                "Non-Authoritative Information"
            },
            {
                204,
                "No Content"
            },
            {
                205,
                "Reset Content"
            },
            {
                206,
                "Partial Content"
            },
            {
                300,
                "Multiple Choices"
            },
            {
                301,
                "Moved Permanently"
            },
            {
                302,
                "Found"
            },
            {
                303,
                "See Other"
            },
            {
                304,
                "Not Modified"
            },
            {
                305,
                "Use Proxy"
            },
            {
                307,
                "Temporary Redirect"
            },
            {
                400,
                "Bad Request"
            },
            {
                401,
                "Unauthorized"
            },
            {
                402,
                "Payment Required"
            },
            {
                403,
                "Forbidden"
            },
            {
                404,
                "Not Found"
            },
            {
                405,
                "Method Not Allowed"
            },
            {
                406,
                "Not Acceptable"
            },
            {
                407,
                "Proxy Authentication Required"
            },
            {
                408,
                "Request Time-out"
            },
            {
                409,
                "Conflict"
            },
            {
                410,
                "Gone"
            },
            {
                411,
                "Length Required"
            },
            {
                412,
                "Precondition Failed"
            },
            {
                413,
                "Request Entity Too Large"
            },
            {
                414,
                "Request-URI Too Large"
            },
            {
                415,
                "Unsupported Media Type"
            },
            {
                416,
                "Requested range not satisfiable"
            },
            {
                417,
                "Expectation Failed"
            },
            {
                500,
                "Internal Server Error"
            },
            {
                501,
                "Not Implemented"
            },
            {
                502,
                "Bad Gateway"
            },
            {
                503,
                "Service Unavailable"
            },
            {
                504,
                "Gateway Time-out"
            },
            {
                505,
                "HTTP Version not supported"
            }
        };

std::string HttpResponse::to_string() const {
    // Declaration of variables
    std::stringstream ss;
    if (cgi) {
        ss << "HTTP/1.1 200 OK\r\n";
        ss << message_body;
        return ss.str();
    }
    int msgsize = message_body.size();

    // Adding HTTP Version

    ss << http_version;

    // If file not found

    if (strcmp(message_body.c_str(), "~@NF@~") == 0) {
        ss << " 404 Not Found\r\n";
    } else if (strcmp(message_body.c_str(),
        "WWW-Authenticate: Basic realm=\"myhttpd-cs252\"") == 0) {
        ss << " 401 Unauthorized\r\n";
    } else if (strcmp(message_body.c_str(), "~@!HACK!@~") == 0) {
        ss << " 403 Forbidden\r\n";
    } else {
        ss << " 200 OK\r\n";
    }

    // Adding connection status

    ss << "Connection: close\r\n";

    // If unauthorized access, quit!

    if (strcmp(message_body.c_str(), "~@!HACK!@~") == 0) {
        return ss.str();
    }

    // If no credentials, request it

    if (strcmp(message_body.c_str(),
      "WWW-Authenticate: Basic realm=\"myhttpd-cs252\"") == 0) {
        ss << message_body << "\r\n" << "\r\n";
        ss << message_body << "\r\n" << "\r\n";
        return ss.str();
    }

    // Add the content type and length
    ss << "Content-Type: " << type << "\r\n";
    ss << "Content-Length: " << len << "\r\n" << "\r\n";

    // If file existed, send the data

    if (strcmp(message_body.c_str(), "~@NF@~") != 0) {
        ss << message_body << "\r\n" << "\r\n";
    }
    return ss.str();
}

void HttpRequest::print() const {
    // Magic string to help with autograder
    std::cout << "\\\\==////REQ\\\\\\\\==////" << std::endl;

    std::cout << "Method: {" << method << "}" << std::endl;
    std::cout << "Request URI: {" << request_uri << "}" << std::endl;
    std::cout << "Query string: {" << query << "}" << std::endl;
    std::cout << "HTTP Version: {" << http_version << "}" << std::endl;

    std::cout << "Headers: " << std::endl;
    for (auto kvp = headers.begin(); kvp != headers.end(); kvp++) {
        std::cout << "field-name: " << kvp ->
          first << "; field-value: " << kvp -> second << std::endl;
    }

    std::cout << "Message body length: " <<
      message_body.length() << std::endl <<
        message_body << std::endl;

    // Magic string to help with autograder
    std::cout << "//==\\\\\\\\REQ////==\\\\" << std::endl;
}

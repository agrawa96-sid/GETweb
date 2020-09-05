// Copyright 2020 Sidhartha Agrawal | Spring 2020 | CS 252 Lab 5

#include <unistd.h>

#include <semaphore.h>

#include<sys/stat.h>

#include<sys/types.h>

#include<sys/wait.h>

#include<fcntl.h>

#include<pthread.h>

#include <string.h>

#include <dirent.h>

#include <time.h>

#include <chrono>

#include<thread>

#include <functional>

#include <ctime>

#include <fstream>

#include <map>

#include <iterator>

#include <set>

#include <algorithm>

#include <iostream>

#include <sstream>

#include <vector>

#include <tuple>

#include <string>


#include "server.hh"

#include "socket.hh"

#include "http_messages.hh"

#include "errors.hh"

#include "misc.hh"

#include "routes.hh"






// Declaration of Global Variables

sem_t sema;
char a[256];
char m[256];
char s[256];
char route[256];
char responseCode[256];

// Function to Update Stat File

void updateStat(auto s, int ctr, int max, int min, std::string a,
  std::string b, int max1, int min1) {
    // Opening the stat file

    char path[1024];
    getcwd(path, 1024);
    char *cur = path, * const end1 = path + sizeof(path);
    cur += strlen(path);
    cur += snprintf(cur, end1-cur, "%s", "/http-root-dir/stats/stat.txt");
    int file = open(path, O_TRUNC | O_CREAT | O_WRONLY, 0664);

    // Adding name and uptime

    write(file, "Name: Sidhartha Agrawal\n", 24);
    write(file, "Uptime: ", 8);
    auto end = std::chrono::steady_clock::now();
    int time = std::chrono::duration_cast<std::chrono::seconds>
      (end - s).count();
    std::string str = std::to_string(time);
    write(file, str.c_str(), str.length());
    write(file, " secs\n", 6);

    // Adding the total requests served

    write(file, "Total Requests Serviced: ", 25);
    std::string str1 = std::to_string(ctr);
    write(file, str1.c_str(), str1.length());
    write(file, "\n", 1);
    str1 = std::to_string(max);

    // Adding the maximum service time

    write(file, "Maximum Service Time: ", 22);
    write(file, str1.c_str(), str1.length());
    write(file, " ms  (", 6);
    str1 = std::to_string(max1);
    write(file, str1.c_str(), str1.length());
    write(file, " microseconds) ", 15);
    write(file, a.c_str(), a.length());
    write(file, "\n", 1);

    // Adding the minimum service time

    write(file, "Minimum Service Time: ", 22);
    str1 = std::to_string(min);
    write(file, str1.c_str(), str1.length());
    write(file, " ms  (", 6);
    str1 = std::to_string(min1);
    write(file, str1.c_str(), str1.length());
    write(file, " microseconds) ", 15);
    write(file, b.c_str(), b.length());
    write(file, "\n", 1);

    // Adding Last Updated Dialog

    write(file, "Last Updated: ", 14);
    auto timenow = std::chrono::system_clock::to_time_t
    (std::chrono::system_clock::now());
    str1 = std::ctime(&timenow);
    write(file, str1.c_str(), str1.length());
    write(file, "\n", 1);

    // Closing the file

    close(file);
}

// Function to log the processes

void logProcess(std::string respCode, std::string route) {
    // Opening the log file

    char path[1024];
    getcwd(path, 1024);
    char *cur = path, * const end = path + sizeof(path);
    cur += strlen(path);
    cur += snprintf(cur, end-cur, "%s", "/http-root-dir/myhttpd.log");
    FILE * fp;
    char c;
    fp = fopen("http-root-dir/loghelp", "r");
    int file = open(path, O_APPEND | O_CREAT | O_WRONLY, 0664);

    // Adding the time of process

    auto timenow =
    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::string str1 = std::ctime(&timenow);
    write(file, str1.c_str(), str1.length());
    write(file, "     ", 5);

    // Adding the IP address

    while (1) {
        c = fgetc(fp);
        if (c == EOF)
            break;
        write(file, & c, 1);
    }
    fclose(fp);

    // Adding the route and response code

    write(file, " ", 1);
    write(file, route.c_str(), route.length());
    write(file, " ", 1);
    write(file, respCode.c_str(), respCode.length());
    write(file, "\n", 1);

    // Closing the file

    close(file);
}

Server::Server(SocketAcceptor
    const & acceptor): _acceptor(acceptor) {}

void Server::run_linear() const {
    // Getting the start time of the server

    auto start = std::chrono::steady_clock::now();
    clock_t tStart = clock();
    int ctr = 1;
    int min = 0;
    int max = 0;
    int min1 = 0;
    int max1 = 0;

    while (1) {
        // Updating the stats on each process

        updateStat(start, ctr, max, min, m, s, max1, min1);

        // Saving the start time

       auto start_p = std::chrono::steady_clock::now();
        clock_t pStart = clock();
        Socket_t sock = _acceptor.accept_connection();
        handle(sock);

        // Saving the end time

        auto end_p = std::chrono::steady_clock::now();

        // Calculating the time to serve the request

       int diff = std::chrono::duration_cast
          < std::chrono::milliseconds > (end_p - start_p).count();
        int diff1 = std::chrono::duration_cast
          < std::chrono::microseconds > (end_p - start_p).count();
        // Updating the min. & max. service time and updating stats accordingly

        if (min == 0 && max == 0) {
            updateStat(start, ctr, diff, diff, a, a, diff1, diff1);
            min = diff;
            min1 = diff1;
            max = diff;
            max1 = diff1;
            snprintf(m, sizeof(m), "%s", a);
            snprintf(s, sizeof(s), "%s", a);
        } else if (diff < min) {
            min = diff;
            min1 = diff1;
            snprintf(m, sizeof(m), "%s", a);
            updateStat(start, ctr, max, min, m, s, max1, min1);
        } else if (diff > max) {
            max = diff;
            max1 = diff1;
            snprintf(s, sizeof(s), "%s", a);
            updateStat(start, ctr, max, min, m, s, max1, min1);
        }

        // Adding the process to the log file

        logProcess(responseCode, route);

        // Incrementing the number of requests served

        ctr++;
    }
}

void Server::run_fork() const {
    while (1) {
        // Accepting the request

        Socket_t sock = _acceptor.accept_connection();

        // Forking a new process

        int slave = fork();
        if (slave == 0) {
            // Handling Request

            handle(sock);
            _exit(0);
        } else if (slave < 0) {
            perror("Fork Err");
        }
    }
}

/* A function that returns a thread running handle using Lambda Expression */

std::thread Server::callHandle(Socket_t sock) const {
    std::thread thr = std::thread([sockt_conv = std::move(sock), this] {
        this -> handle(sockt_conv);
    });
    return thr;
}

void Server::run_thread() const {
    while (1) {
        // Accepting Connection

        Socket_t sock = _acceptor.accept_connection();
        if (sock > 0) {
            // Handling request

            std::thread t = callHandle(std::move(sock));
            t.detach();
        }
    }
}

struct ThreadParams {
    const Server * server;
    Socket_t sock;
};

void dispatchThread(ThreadParams * p) {
    while (1) {
        sem_wait(&sema);
        Socket_t sock = p -> server -> _acceptor1.accept_connection();
        if (sock >= 0) {
            p -> server -> handle(sock);
        }
        sem_post(&sema);
    }
    delete p;
}

void Server::run_thread_pool(const int num_threads) const {
    // Initializing Sempahore

    sem_init(&sema, 0, 1);

    // Creating a new ThreadParam

    ThreadParams * threadP = new ThreadParams;
    threadP -> server = this;
    threadP -> sock = NULL;

    // Creating a pool of num_threads posix threads
    pthread_t thr[10000];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    for (int i = 0; i < num_threads; i++) {
        ThreadParams * threadParams = new ThreadParams;
        threadParams -> server = this;
        threadParams -> sock = NULL;
        pthread_create(&thr[i], &attr, (void*(*)(void*)) dispatchThread,
          reinterpret_cast<void *> (threadParams));
        pthread_detach(thr[i]);
    }
    dispatchThread(threadP);
    free(threadP);
}

void tokenize
  (std::string const &str, const char delim,  std::vector<std::string> &out ) {
        int start;
        int end = 0;

        // Iterating through string and tokenizing

        while ((start = str.find_first_not_of(delim, end))
          != std::string::npos) {
                end = str.find(delim, start);
                out.push_back(str.substr(start, end - start));
        }
}
int parse_request(const Socket_t & sock, HttpRequest *
    const request, std::vector < std::string > * tokens) {
    // Declaration of Variables

    std::string req;
    const char delim = ' ';

    // Reading first line

    req = sock -> readline();
    if (req.length() == 0) {
        return 1;
    }
    request -> message_body.append(req);

    // Tokenizing the first line

    tokenize(req, delim, * tokens);

    // Reading remaining lines

    while (req.compare("\r\n") != 0 && !req.empty()) {
        req = sock -> readline();
        request -> message_body.append(req);
    }
    return 0;
}

int file_finder(HttpResponse & r, std::vector < std::string > &tokens) {
    char path[1024];
    int len;
    std::string dir_to_root = std::string("/http-root-dir");

    len = 0;

    // Declaration of Variables

    FILE * file;
    struct stat s;
    char buffer[128];
    char c;
    int j = 0;

    // Acquiring the path to current directory

    getcwd(path, 1024);
    snprintf(route, sizeof(route), "%s", "/htdocs");

    // Bulding the path

    if (strcmp(tokens[1].c_str(), "/stats") == 0) {
      snprintf(route, sizeof(route), "%s", "/stats");
    }
    if (strcmp(tokens[1].c_str(), "/logs") == 0) {
      snprintf(route, sizeof(route), "%s", "/logs");
      snprintf(responseCode, sizeof(responseCode), "%s", "200 OK");
    }
    char *cur = path, * const end1 = path + sizeof(path);
    cur += strlen(path);
    cur += snprintf(cur, end1-cur, "%s", dir_to_root.c_str());
    if (strcmp(tokens[1].c_str(), "/stats") == 0) {
        cur += snprintf(cur, end1-cur, "%s", "/stats/stat.txt");
    } else if (strcmp(tokens[1].c_str(), "/logs") == 0) {
        cur += snprintf(cur, end1-cur, "%s", "/myhttpd.log");
    } else {
        cur += snprintf(cur, end1-cur, "%s", "/htdocs");
        cur += snprintf(cur, end1-cur, "%s", tokens[1].c_str());
    }

    // Checking if Directory exists

    if (stat(path, & s) == 0) {
        if (s.st_mode & S_IFDIR) {
            // It is a directory
            std::string indx = std::string("/index.html");
            tokens[1].append(indx);
            cur += snprintf(cur, end1-cur, "%s", indx.c_str());
        }
    }

    // Opening file to read

    file = fopen(path, "r");

    // If file exists

    if (file > 0) {
        // Finding the length of file

        fseek(file, 0, SEEK_END);
        len = ftell(file) + 1;
        fseek(file, 0, SEEK_SET);

        r.len = len;
        // Reading the data into buffer

        while (size_t rc = fread(buffer, 1, sizeof(buffer), file)) {
            r.message_body.append(buffer, rc);
        }

        // Closing the file

        fclose(file);
        return 1;
    }
    r.len = 0;
    return 0;
}

std::string content_type_set(std::string name) {
    // Function to return the type of data sent as param

    if (name.size() == 0)
        return "";
    else if (strstr(name.c_str(), ".html") != 0)
        return "text/html";
    else if (strstr(name.c_str(), ".png") != 0)
        return "image/png";
    else if (strstr(name.c_str(), ".gif") != 0)
        return "image/gif";
    else if (strstr(name.c_str(), ".svg") != 0)
        return "image/svg+xml";
    else if (strstr(name.c_str(), ".ico") != 0)
        return "image/ico";
    else if (strstr(name.c_str(), ".jpg") != 0)
        return "image/jpeg";
    else
        return "text/plain";
}

struct comp1 {
    template < typename T >
        bool operator()(const T & l,
            const T & r) const {
            if (l.second != r.second)
                return l.second > r.second;

            return l.first > r.first;
        }
};

struct comp {
    template < typename T >
        bool operator()(const T & l,
            const T & r) const {
            if (l.second != r.second)
                return l.second < r.second;

            return l.first < r.first;
        }
};

// Function to build browsable directories in HTML form

std::string dirHtmlBuild(HttpRequest req) {
    char path[1024];
    std::string dir_to_root = std::string("/http-root-dir/htdocs");
    getcwd(path, 1024);
    char *cur = path, * const end1 = path + sizeof(path);
    cur += strlen(path);
    cur += snprintf(cur, end1-cur, "%s", dir_to_root.c_str());
    int mode = -1;
    if (strstr(req.request_uri.c_str(), "?n=A/") != 0)
        mode = 0;
    else if (strstr(req.request_uri.c_str(), "?n=D/") != 0)
        mode = 1;
    else if (strstr(req.request_uri.c_str(), "?d=A/") != 0)
        mode = 2;
    else if (strstr(req.request_uri.c_str(), "?d=D/") != 0)
        mode = 3;
    if (mode == -1) {
        cur += snprintf(cur, end1-cur, "%s", req.request_uri.c_str());
    } else {
        char b[100];
        snprintf(b, sizeof(b), "%s", req.request_uri.c_str());
        char * ptr = b;
        int i = 0;
        while (*ptr != '?') {
            i++;
            ptr++;
        }
        strncat(path, b, i);
    }
    DIR * dir;
    struct dirent * ent;
    std::string msg;
    msg.append("<!DOCTYPE html>\n");
    msg.append("<html>\n<head>\n<title>Browsing Dir: ");
    msg.append(req.request_uri.c_str());
    msg.append("</title>\n</head>\n");
    msg.append("<body>\n<table>\n");
    msg.append("<td><a href=\"../\"><- Parent Directory</a>");
    msg.append("</td><td align=\"right\">");
    msg.append("</td><td>&nbsp;</td></tr>");
    if (mode == 0 || mode == -1) {
        msg.append("<th><a href=\"?n=D/\">Name</a></th><th>");
        msg.append("<a href=\"?d=A/\">Date modified</a></th>");
        msg.append("<tr><th colspan=\"10\"><hr></th></tr>");
    } else if (mode == 1) {
        msg.append("<th><a href=\"?n=A/\">Name</a></th><th>");
        msg.append("<a href=\"?d=A/\">Date modified</a>");
        msg.append("</th><tr><th colspan=\"10\"><hr></th></tr>");
    } else if (mode == 2) {
        msg.append("<th><a href=\"?n=A/\">Name</a></th><th>");
        msg.append("<a href=\"?d=D/\">Date modified");
        msg.append("</a></th><tr><th colspan=\"10\"><hr></th></tr>");
    } else if (mode == 3) {
        msg.append("<th><a href=\"?n=A/\">Name</a></th><th>");
        msg.append("<a href=\"?d=A/\">Date modified");
        msg.append("</a></th><tr><th colspan=\"10\"><hr></th></tr>");
    }
    std::map < std::string, std::string > files;
    if (dir = opendir(path)) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent -> d_name, ".") == 0 ||
              strcmp(ent -> d_name, "..") == 0)
                continue;
            std::string filename = path;
            struct stat result;
            filename.append("/");
            filename.append(ent -> d_name);
            if (stat(filename.c_str(), & result) == 0) {
                time_t t = result.st_mtime;
                struct tm * tm;
                char buf[200];
                tm = localtime(&t);
                strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S ", tm);
                std::string name;
                name = ent -> d_name;
                if (result.st_mode & S_IFDIR)
                    name.append("/");
                files.insert(std::pair < std::string, std::string >(name, buf));
            }
        }
        closedir(dir);
    }
    std::map < std::string, std::string > ::iterator itr;
    std::set < std::pair < std::string, std::string > ,
      comp > set(files.begin(), files.end());
    std::set < std::pair < std::string, std::string > ,
      comp1 > set1(files.begin(), files.end());
    if (mode == 0 || mode == -1) {
        for (itr = files.begin(); itr != files.end(); ++itr) {
            msg.append("<td><a href=\"");
            msg.append(itr -> first);
            msg.append("\">");
            msg.append(itr -> first);
            msg.append("</a></td><td align=\"right\">");
            msg.append(itr -> second);
            msg.append("</td><td align=\"right\"></td><td>&nbsp;</td></tr>");
        }
    }
    std::map < std::string, std::string > ::reverse_iterator it;
    if (mode == 1) {
        for (it = files.rbegin(); it != files.rend(); it++) {
            msg.append("<td><a href=\"");
            msg.append(it -> first);
            msg.append("\">");
            msg.append(it -> first);
            msg.append("</a></td><td align=\"right\">");
            msg.append(it -> second);
            msg.append("</td><td align=\"right\"></td><td>&nbsp;</td></tr>");
        }
    }
    if (mode == 2) {
        for (auto
            const & pair: set) {
            msg.append("<td><a href=\"");
            msg.append(pair.first);
            msg.append("\">");
            msg.append(pair.first);
            msg.append("</a></td><td align=\"right\">");
            msg.append(pair.second);
            msg.append("</td><td align=\"right\"></td><td>&nbsp;</td></tr>\n");
        }
    }
    if (mode == 3) {
        for (auto
            const & pair: set1) {
            msg.append("<td><a href=\"");
            msg.append(pair.first);
            msg.append("\">");
            msg.append(pair.first);
            msg.append("</a></td><td align=\"right\">");
            msg.append(pair.second);
            msg.append("</td><td align=\"right\"></td><td>&nbsp;</td></tr>\n");
        }
    }
    msg.append("<tr><th colspan=\"10\"><hr></th></tr></table>");
    msg.append("CS 252 HTTP Server | Spring 2020 | Sidhartha Agrawal");
    msg.append("</body><style>body { background-image:");
    msg.append("url('https://pbs.twimg.com/media/DGLPqnFUIAAFjuG.jpg');");
    msg.append("background-repeat: no-repeat; background-attachment: ");
    msg.append("fixed;background-position: 50% 65%; background-size:");
    msg.append("cover;  } </style></html>");
    return msg;
}

void Server::handle(const Socket_t & sock) const {
    // Declaration of Variables
    if (sock == NULL) {
        return;
    }
    snprintf(route, sizeof(route), "%s", "/logs");
    int cgi = 0;
    HttpRequest request;
    HttpResponse resp;
    std::vector < std::string > tokens;
    int file_exists = 0;
    resp.cgi = 0;

    // Parsing the request

    if (parse_request(sock, & request, & tokens))
        return;

    // Removing unnecessary delimeters

    snprintf(a, sizeof(a), "%s", tokens[1].c_str());

    for (int i = 0; i < tokens.size(); i++) {
        if (strstr(tokens[i].c_str(), "\r\n")) {
            tokens[i].pop_back();
            tokens[i].pop_back();
        }
    }

    // If no Authorization, prompt for credentials

    if (strstr(request.message_body.c_str(), "Authorization") == 0) {
        resp.http_version = tokens[2];
        resp.message_body =
          std::string("WWW-Authenticate: Basic realm=\"myhttpd-cs252\"");
        sock -> write(resp.to_string());
        return;
    }

    // Check for validity of credentials

    if (strstr(request.message_body.c_str(), "Authorization") != 0) {
        if (strncmp(strstr(request.message_body.c_str(), "Basic") + 6,
          "YWdyYXNpZDpodHRw", 16) != 0) {
            resp.http_version = tokens[2];
            resp.message_body =
              std::string("WWW-Authenticate: Basic realm=\"myhttpd-cs252\"");
            sock -> write(resp.to_string());
            return;
        }
    }

    // Populating the request

    request.method = tokens[0];
    request.request_uri = tokens[1];
    request.http_version = tokens[2];
    // Check if the URI ends as a directory

    int serve_dir = 0;
    resp.cgi = 0;
    int s = sock -> getsock();
    if (strcmp(tokens[0].c_str(), "GET") == 0) {
        if (strncmp(tokens[1].c_str(), "/cgi-bin", 8) == 0) {
            cgi = 1;
            resp.cgi = 1;
            resp.message_body = handle_cgi_bin(request, s);
            snprintf(responseCode, sizeof(responseCode), "%s", "200 OK");
            snprintf(route, sizeof(route), "%s", "/cgi-bin");
            if (strcmp(resp.message_body.c_str(), "loadable") == 0)
                return;
            if (strcmp(resp.message_body.c_str(), "~!@#ERR#@!~") == 0) {
                snprintf(responseCode, sizeof(responseCode), "%s", "404 Not Found");
                return;
            }
        } else {
            char path[1024];
            struct stat s;
            std::string dir_to_root = std::string("/http-root-dir/htdocs");
            getcwd(path, 1024);
            char *cur = path, * const end1 = path + sizeof(path);
            cur += strlen(path);
            cur += snprintf(cur, end1-cur, "%s", dir_to_root.c_str());
            if (strcmp(tokens[1].c_str(), "/") == 0) {
                snprintf(cur, end1-cur, "%s", "/index.html");
            } else {
                snprintf(cur, end1-cur, "%s", tokens[1].c_str());
            }
            char path1[1024];
            const char * ptr = & tokens[1].c_str()[1];
            snprintf(path1, sizeof(path1), "%s", ptr);
            if (strstr(request.message_body.c_str(), "GET /stats ") != 0) {
                file_exists = file_finder(resp, tokens);
            } else if (strstr(request.message_body.c_str(), "GET /logs") != 0) {
                file_exists = file_finder(resp, tokens);
            } else if (strstr(path1, "/") == 0) {
                file_exists = file_finder(resp, tokens);
            } else if (strchr(tokens[1].c_str(), '?') != 0) {
                resp.message_body = dirHtmlBuild(request);
                resp.len = resp.message_body.length();
                serve_dir = 1;
            } else if (stat(path, & s) == 0) {
                if (s.st_mode & S_IFDIR) {
                    resp.message_body = dirHtmlBuild(request);
                    resp.len = resp.message_body.length();
                    serve_dir = 1;
                } else {
                    file_exists = file_finder(resp, tokens);
                }
            }
        }
    }


    // Populating the response

    resp.type = content_type_set(tokens[1]);
    if (tokens[1].back() == '/' && tokens[1].length() > 1) {
        resp.type = "text/html";
    }
    resp.http_version = tokens[2];

    // If file doesn't exist, setting up a custom verifiable string

    if (!file_exists && !cgi && !serve_dir) {
        resp.message_body = std::string("~@NF@~");
    }

    // Clearing tokens for next run

    while (tokens.size() != 0)
        tokens.pop_back();

    // Delivering the response to client

    sock -> write(resp.to_string());

    // Getting the responseCode

    std::string res = resp.to_string();
    int i = 0;
    for (i = 0; i < res.length(); i++) {
        if (res[i] == ' ')
            break;
    }
    int j = 0;
    for (j = i + 1; i < res.length() - 1; j++) {
        if (res[j + 1] == '\n')
            break;
    }
    j -= i;
    res = res.substr(i, j);
    if (res.length() == 0)
        res = "200 OK";
    snprintf(responseCode, sizeof(responseCode), "%s", res.c_str());
}

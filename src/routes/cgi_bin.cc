// Copyright 2020 Sidhartha Agrawal | Spring 2020 | CS 252 Lab 5

#include <unistd.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <string.h>

#include <dlfcn.h>

#include <link.h>

#include <errno.h>

#include <stdio.h>

#include<stdlib.h>

#include<iostream>

#include<string>

#include <vector>

#include <algorithm>

#include <iterator>

#include <map>

#include "socket.hh"

#include "http_messages.hh"

typedef void(*httprunfunc)(int ssock, const char * querystring);

// Declaring Global Variables

std::map < std::string, void * > loaded;

int loadableModulesHandler(const HttpRequest & request, int ssock) {
    // Declaring Variables

    httprunfunc run;
    char str[100];
    snprintf(str, sizeof(str), "%s", request.request_uri.c_str() + 9);
    std::map < std::string, void * > ::iterator it;
    char query[1024];
    char pth[1024];
    int cpy = 0;
    int pos = 0;
    int pos1 = 0;
    void * lib;

    // Checking if query exists and bulding query string and filename

    for (int i = 0; i < request.request_uri.length(); i++) {
        if (cpy) {
            query[pos] = request.request_uri[i];
            pos++;
        }
        if (!cpy && request.request_uri[i] != '?') {
            pth[pos1] = request.request_uri[i];
            pos1++;
        }
        if (request.request_uri[i] == '?')
            cpy = 1;
    }

    // Building the path

    char path[1024];
    char *cur = path, * const end = path + sizeof(path);
    cur += snprintf(cur, end-cur, "%s", "/homes/agrawa96/cs252/lab5-src/http-root-dir");
    if (cur < end) {
      cur += snprintf(cur, end-cur, "%s",  pth);
}


    // Checking if the file was already loaded

    it = loaded.find(pth);
    if (it != loaded.end()) {
        lib = it-> second;
    } else {
        printf("New call to DLOPEN()\n");

        // Creating new dlopen

        lib = dlopen(path, RTLD_LAZY);

        // Adding the filename, lib pair to map

        loaded.insert(std::pair < std::string, void* > (std::string(pth), lib));

        // Err Checking

        if (lib == NULL) {
            fprintf(stderr, "%s not found\n", str);
            perror("dlopen");
            return 1;
        }
    }

    // Initializing the function

    run = (httprunfunc) dlsym(lib, "httprun");

    // ERR Checking

    if (run == NULL) {
        perror("dlsym: httprun not found:");
        return 1;
    }

    // Creating a new Process

    int status = fork();

    // If child

    if (status == 0) {
        run(ssock, query);
        _exit(0);
    }  else if (status < 0) {
        perror("Error forking");
        return 1;
    }  else if (status > 0) {
        waitpid(status, NULL, 0);
    }
    return 0;
}

std::string handle_cgi_bin(const HttpRequest & request, int sock) {
    // Checking if servicing Loadable Modules
    if (strstr(request.request_uri.c_str(), ".so") != 0) {
        int val = 0;
        val = loadableModulesHandler(request, sock);
        std::map < std::string, void * > ::iterator it;
        if (val)
            return "~!@#ERR#@!~";

        return "loadable";
    }
    int args1 = 0;

    // Checking if there are arguments

    if (strchr(request.request_uri.c_str(), '?') != 0) {
        args1 = 1;
    }

    // Declaration of Variables

    char path[1024];
    getcwd(path, 1024);
    int dfltin = dup(0);
    int dfltout = dup(1);
    char *cur = path, * const end = path + sizeof(path);
    cur += strlen(path);
    cur += snprintf(cur, end-cur, "%s", "/http-root-dir");
        char query[1024];
        int cpy = 0;
        int pos = 0;
        int pos1 = 0;
        char pth[1024];

        // Extracting query string and filename

        for (int i = 0; i < request.request_uri.length(); i++) {
            if (request.request_uri[i] == '?') {
                cpy = 1;
                i++;
                pth[pos1] = '\0';
            }
            if (cpy) {
                query[pos] = request.request_uri[i];
                pos++;
            } else if (!cpy && request.request_uri[i] != '?') {
                pth[pos1] = request.request_uri[i];
                pos1++;
            }
        }
        query[pos] = '\0';

        // Creating a Pipe

        int fdpipe[2];
        pipe(fdpipe);
        cur = path + strlen(path);
        if (strchr(request.request_uri.c_str(), '?') == 0)
          snprintf(cur, end-cur, "%s", request.request_uri.c_str());
        else
          snprintf(cur, end-cur, "%s", pth);
        cur = path + strlen(path);
        snprintf(cur, end-cur, "%s", "\0");
        char ** args = new char * [2];
        args[0] = strdup(path);
        args[1] = NULL;
        const char * first_arg = args[0];
        char op[10000];

        // Creating a new Process

        int status = fork();

        // If child

        if (status == 0) {
            dup2(fdpipe[1], 1);
            close(fdpipe[1]);
            close(fdpipe[0]);
            setenv("REQUEST_METHOD", request.method.c_str(), 1);
            setenv("QUERY_STRING", query, 1);
            execvp(first_arg, args);
            _exit(1);
        } else if (status < 0) {
            perror("Error forking");
            exit(1);
        } else if (status > 0) {
            waitpid(status, NULL, 0);
            close(fdpipe[1]);
            char c;
            int i = 0;

            // Read the PIPE Output to Buffer

            while (read(fdpipe[0], & c, 1)) {
                op[i++] = c;
            }
            op[i] = '\0';
            close(fdpipe[0]);
        }

        // Restore defaults

        dup2(dfltin, 0);
        dup2(dfltout, 1);
        close(dfltin);
        close(dfltout);
        delete(args[0]);
        delete(args);
        return std::string(op);
   /*
    // If there are no arguments

    if (!args1) {
        strcat(path, dir_to_root.c_str());
        int fdpipe[2];
        pipe(fdpipe);
        strcat(path, str);
        char ** args = new char * [2];
        args[0] = strdup(path);
        args[1] = NULL;
        const char * first_arg = args[0];
        char op[10000];
        char * buffer;
        int status = fork();
        if (status == 0) {
            dup2(fdpipe[1], 1);
            close(fdpipe[1]);
            close(fdpipe[0]);
            setenv("REQUEST_METHOD", request.method.c_str(), 1);
            setenv("QUERY_STRING", " ", 1);
            execvp(first_arg, args);
            _exit(1);
        } else if (status < 0) {
            perror("Error forking");
            exit(1);
        } else if (status > 0) {
            waitpid(status, NULL, 0);
            close(fdpipe[1]);
            char c;
            int i = 0;
            while (read(fdpipe[0], & c, 1)) {
                op[i++] = c;
            }
            op[i] = '\0';
            close(fdpipe[0]);
        }
        dup2(dfltin, 0);
        dup2(dfltout, 1);
        close(dfltin);
        close(dfltout);
        delete(args[0]);
        delete(args);
        return std::string(op);
    }
    dup2(dfltin, 0);
    dup2(dfltout, 1);
    close(dfltin);
    close(dfltout);*/
}

std::string handle_cgi_bin_post(const HttpRequest & request) {
    std::cout << request.message_body << "\n";
}

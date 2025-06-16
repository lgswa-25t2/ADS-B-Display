#pragma once
#ifndef HttpClientH
#define HttpClientH

#include <string>
#include <windows.h>
#include <curl/curl.h>
#include "json.hpp"  // Make sure this path points to the correct location of json.hpp

using json = nlohmann::json;

class HttpClient {
private:
    CURL* curl;
    HMODULE hCurlLib;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string response;
    std::string lastError;

    // Function pointers for dynamic loading
    typedef CURL* (*curl_easy_init_t)(void);
    typedef void (*curl_easy_cleanup_t)(CURL*);
    typedef CURLcode (*curl_easy_setopt_t)(CURL*, CURLoption, ...);
    typedef CURLcode (*curl_easy_perform_t)(CURL*);
    typedef void (*curl_global_init_t)(long);
    typedef void (*curl_global_cleanup_t)(void);
    typedef const char* (*curl_easy_strerror_t)(CURLcode);

    curl_easy_init_t p_curl_easy_init;
    curl_easy_cleanup_t p_curl_easy_cleanup;
    curl_easy_setopt_t p_curl_easy_setopt;
    curl_easy_perform_t p_curl_easy_perform;
    curl_global_init_t p_curl_global_init;
    curl_global_cleanup_t p_curl_global_cleanup;
    curl_easy_strerror_t p_curl_easy_strerror;

public:
    HttpClient();
    ~HttpClient();
    
    bool init();
    void cleanup();
    std::string get(const char* url);
    
    // New method for JSON parsing
    json getJson(const char* url);
    
    // Error handling
    const std::string& getLastError() const { return lastError; }
    
private:
    bool loadCurlFunctions();
};

#endif 
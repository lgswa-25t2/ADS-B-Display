#pragma once
#ifndef HttpClientH
#define HttpClientH

// Windows 매크로 충돌 방지
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <string>
#include <map>
#include <windows.h>

// OpenSSL 매크로 충돌 방지
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_NO_SHARED

#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>

// OpenSSL 버전 호환성 매크로
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    #define OPENSSL_VERSION_3_PLUS
#elif OPENSSL_VERSION_NUMBER >= 0x10100000L
    #define OPENSSL_VERSION_1_1_PLUS
#else
    #define OPENSSL_VERSION_1_0
#endif

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
    typedef struct curl_slist* (*curl_slist_append_t)(struct curl_slist*, const char*);
	typedef void (*curl_slist_free_all_t)(struct curl_slist*);
	typedef CURLcode (*curl_easy_getinfo_t)(CURL*, CURLINFO, ...);

    curl_easy_init_t p_curl_easy_init;
    curl_easy_cleanup_t p_curl_easy_cleanup;
    curl_easy_setopt_t p_curl_easy_setopt;
    curl_easy_perform_t p_curl_easy_perform;
	curl_global_init_t p_curl_global_init;
    curl_global_cleanup_t p_curl_global_cleanup;
	curl_easy_strerror_t p_curl_easy_strerror;
    curl_slist_append_t p_curl_slist_append;
	curl_slist_free_all_t p_curl_slist_free_all;
	curl_easy_getinfo_t p_curl_easy_getinfo;

public:
    HttpClient();
    ~HttpClient();
    
    bool init();
    void cleanup();
    std::string get(const char* url);
    
    // New method for JSON parsing
    json getJson(const char* url);

    // POST method for sending JSON data
    int post(const std::string& url, const std::string& data, std::string& response,
			 const std::map<std::string, std::string>& headers = {});

    // Error handling
    const std::string& getLastError() const { return lastError; }
    
private:
    bool loadCurlFunctions();
};

#endif
#include "HttpClient.h"

size_t HttpClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* str = (std::string*)userp;
    str->append((char*)contents, realsize);
    return realsize;
}

HttpClient::HttpClient() : curl(nullptr), hCurlLib(NULL) {
}

HttpClient::~HttpClient() {
    cleanup();
}

bool HttpClient::loadCurlFunctions() {
    hCurlLib = LoadLibraryA("libcurl.dll");
    if (!hCurlLib) {
        lastError = "Failed to load libcurl.dll";
        return false;
    }

    p_curl_easy_init = (curl_easy_init_t)GetProcAddress(hCurlLib, "curl_easy_init");
    p_curl_easy_cleanup = (curl_easy_cleanup_t)GetProcAddress(hCurlLib, "curl_easy_cleanup");
    p_curl_easy_setopt = (curl_easy_setopt_t)GetProcAddress(hCurlLib, "curl_easy_setopt");
    p_curl_easy_perform = (curl_easy_perform_t)GetProcAddress(hCurlLib, "curl_easy_perform");
    p_curl_global_init = (curl_global_init_t)GetProcAddress(hCurlLib, "curl_global_init");
    p_curl_global_cleanup = (curl_global_cleanup_t)GetProcAddress(hCurlLib, "curl_global_cleanup");
    p_curl_easy_strerror = (curl_easy_strerror_t)GetProcAddress(hCurlLib, "curl_easy_strerror");

    if (!p_curl_easy_init || !p_curl_easy_cleanup || !p_curl_easy_setopt || 
        !p_curl_easy_perform || !p_curl_global_init || !p_curl_global_cleanup ||
        !p_curl_easy_strerror) {
		lastError = "Failed to load one or more curl functions";
        FreeLibrary(hCurlLib);
        hCurlLib = NULL;
        return false;
    }

    return true;
}

bool HttpClient::init() {
    if (!loadCurlFunctions()) {
        return false;
    }

    p_curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = p_curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return false;
    }
    return true;
}

void HttpClient::cleanup() {
    if (curl) {
        p_curl_easy_cleanup(curl);
        curl = nullptr;
    }
    if (hCurlLib) {
        p_curl_global_cleanup();
        FreeLibrary(hCurlLib);
        hCurlLib = NULL;
    }
}

std::string HttpClient::get(const char* url) {
	if (!curl) {
        lastError = "CURL not initialized";
        return "";
    }

    response.clear();
    
    p_curl_easy_setopt(curl, CURLOPT_URL, url);
    p_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    p_curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    p_curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    p_curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    // Disable SSL verification
    p_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    p_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
	CURLcode res = p_curl_easy_perform(curl);
	if (res != CURLE_OK) {
		lastError = p_curl_easy_strerror(res);
		return "";
    }

	return response;
}

json HttpClient::getJson(const char* url) {
	std::string response = get(url);

	if (response.empty()) {
        return json::object();  // Return empty JSON object in case of error
    }
    
    try {
        return json::parse(response);
    } catch (const json::parse_error& e) {
        lastError = std::string("JSON parsing error: ") + e.what();
        return json::object();
    }
} 
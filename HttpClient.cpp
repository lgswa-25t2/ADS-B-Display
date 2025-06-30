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
	hCurlLib = LoadLibraryA("../../lib/curl/libcurl.dll");
    if (!hCurlLib) {
        lastError = "Failed to load libcurl.dll";
		return false;
    }

    p_curl_easy_init = (curl_easy_init_t)GetProcAddress(hCurlLib, "curl_easy_init");
    p_curl_easy_cleanup = (curl_easy_cleanup_t)GetProcAddress(hCurlLib, "curl_easy_cleanup");
    p_curl_easy_setopt = (curl_easy_setopt_t)GetProcAddress(hCurlLib, "curl_easy_setopt");
    p_curl_easy_perform = (curl_easy_perform_t)GetProcAddress(hCurlLib, "curl_easy_perform");
    p_curl_easy_getinfo = (curl_easy_getinfo_t)GetProcAddress(hCurlLib, "curl_easy_getinfo");
    p_curl_global_init = (curl_global_init_t)GetProcAddress(hCurlLib, "curl_global_init");
    p_curl_global_cleanup = (curl_global_cleanup_t)GetProcAddress(hCurlLib, "curl_global_cleanup");
    p_curl_easy_strerror = (curl_easy_strerror_t)GetProcAddress(hCurlLib, "curl_easy_strerror");
	p_curl_slist_append = (curl_slist_append_t)GetProcAddress(hCurlLib, "curl_slist_append");
	p_curl_slist_free_all = (curl_slist_free_all_t)GetProcAddress(hCurlLib, "curl_slist_free_all");

	if (!p_curl_easy_init || !p_curl_easy_cleanup || !p_curl_easy_setopt ||
        !p_curl_easy_perform || !p_curl_easy_getinfo || !p_curl_global_init || 
        !p_curl_global_cleanup || !p_curl_easy_strerror || !p_curl_slist_append || 
        !p_curl_slist_free_all) {
		lastError = "Failed to load one or more curl functions";
        printf("Error: Failed to load one or more curl functions\n");
        FreeLibrary(hCurlLib);
        hCurlLib = NULL;
        return false;
    }

    return true;
}

bool HttpClient::init() {
    // Initialize OpenSSL (static linking) - Version compatible
#ifdef OPENSSL_VERSION_3_PLUS
    // OpenSSL 3.0+ automatically initializes on first use
    // No explicit initialization needed
#elif defined(OPENSSL_VERSION_1_1_PLUS)
    // OpenSSL 1.1.x - automatic initialization, no explicit initialization needed
    // SSL_library_init(), SSL_load_error_strings(), OpenSSL_add_all_algorithms() are deprecated
#else
    // OpenSSL 1.0.x - legacy initialization
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
#endif
    
    // Load curl functions
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
    
    // OpenSSL cleanup - Version compatible
#ifdef OPENSSL_VERSION_3_PLUS
    // OpenSSL 3.0+ automatically cleans up
    // No explicit cleanup needed
#elif defined(OPENSSL_VERSION_1_1_PLUS)
    // OpenSSL 1.1.x - automatic cleanup, no explicit cleanup needed
    // EVP_cleanup() and ERR_free_strings() are deprecated
#else
    // OpenSSL 1.0.x - legacy cleanup
    EVP_cleanup();
    ERR_free_strings();
#endif
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

int HttpClient::post(const std::string& url, const std::string& data, std::string& response,
                    const std::map<std::string, std::string>& headers) {
    if (!curl) {
        lastError = "CURL not initialized";
        printf("Error: CURL not initialized\n");
        return -1;
    }

    if (!p_curl_easy_getinfo) {
        lastError = "curl_easy_getinfo function not loaded";
        printf("Error: curl_easy_getinfo function not loaded\n");
        return -1;
    }

	response.clear();

    // Set URL
    p_curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set POST data
    p_curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    p_curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());

    // Set write callback
    p_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    p_curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	// Set headers
    struct curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        std::string header_line = header.first + ": " + header.second;
        header_list = p_curl_slist_append(header_list, header_line.c_str());
    }

    if (header_list) {
        p_curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }

    // Set other options
    p_curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    p_curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // Disable SSL verification
    p_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    p_curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // Perform the request
	CURLcode res = p_curl_easy_perform(curl);

    // Get HTTP response code - 안전하게 처리
    long http_code = 0;
    if (p_curl_easy_getinfo && curl) {
		CURLcode info_res = p_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (info_res != CURLE_OK) {
            printf("Warning: Failed to get HTTP response code: %s\n", p_curl_easy_strerror(info_res));
            printf("Warning: CURLcode %d means: %s\n", info_res, p_curl_easy_strerror(info_res));
            
            // 대안: CURLINFO_HTTP_CODE 사용 (구버전 호환성)
            printf("post 3336 - Trying alternative method with CURLINFO_HTTP_CODE\n");
            info_res = p_curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &http_code);
            printf("post 3337 - Alternative method result: %d, http_code: %ld\n", info_res, http_code);
            
            if (info_res != CURLE_OK) {
                printf("Warning: Alternative method also failed: %s\n", p_curl_easy_strerror(info_res));
                
                // 최후의 수단: 직접 값 사용 (CURLINFO_RESPONSE_CODE = 2097154)
                printf("post 3338 - Trying with direct value 2097154 (CURLINFO_RESPONSE_CODE)\n");
                info_res = p_curl_easy_getinfo(curl, (CURLINFO)2097154, &http_code);
                printf("post 3339 - Direct value method result: %d, http_code: %ld\n", info_res, http_code);
                
                if (info_res != CURLE_OK) {
					printf("Warning: All methods failed to get HTTP response code\n");
                    http_code = 0; // 기본값 설정
                }
            }
        }
    } else {
        printf("Warning: curl_easy_getinfo function not available or curl handle is NULL\n");
        http_code = 0; // 기본값 설정
    }

    // Clean up headers
    if (header_list) {
        p_curl_slist_free_all(header_list);
    }

    if (res != CURLE_OK) {
        lastError = p_curl_easy_strerror(res);
        printf("Error: CURL perform failed: %s\n", lastError.c_str());
        return -1;
    }

	printf("post 444 - Returning HTTP code: %ld\n", http_code);
    
    // HTTP 400 에러인 경우 응답 내용 출력
    if (http_code == 400) {
        printf("post 444a - HTTP 400 Bad Request detected!\n");
        printf("post 444b - Response length: %zu\n", response.length());
		printf("post 444c - Response content: %s\n", response.c_str());
        printf("post 444d - This usually means:\n");
        printf("post 444e - 1. Invalid URL format\n");
        printf("post 444f - 2. Missing required headers\n");
        printf("post 444g - 3. Invalid JSON format in POST data\n");
        printf("post 444h - 4. Authentication issues\n");
        printf("post 444i - 5. Missing Content-Type header\n");
    }

	// HTTP 상태 코드를 얻지 못했더라도 요청은 성공했을 수 있음
    if (http_code == 0) {
        printf("Warning: Could not get HTTP response code, assuming success (200)\n");
        return 200; // 기본적으로 성공으로 간주
    }

    return static_cast<int>(http_code);
}
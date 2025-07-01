// Prevent WinCrypt defines from being overridden by OpenSSL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER
#define OPENSSL_NO_WINCRYPT
#define OPENSSL_NO_ENGINE
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_NO_EC
#define OPENSSL_NO_DH
#define OPENSSL_NO_DSA
#define OPENSSL_NO_RSA
#define OPENSSL_NO_AES
#define OPENSSL_NO_DES
#define OPENSSL_NO_RC4
#define OPENSSL_NO_MD5

#include "OAuth2Helper.h"
#include "../json.hpp"
#include "../HttpClient.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

OAuth2Helper::OAuth2Helper()
    : token_uri_("https://oauth2.googleapis.com/token"), initialized_(false), 
      hCryptoLib(NULL), hSslLib(NULL) {
    // Initialize function pointers to NULL
    p_BIO_new_mem_buf = nullptr;
    p_BIO_free = nullptr;
    p_PEM_read_bio_PrivateKey = nullptr;
    p_EVP_MD_CTX_new = nullptr;
    p_EVP_sha256 = nullptr;
    p_EVP_DigestSignInit = nullptr;
    p_EVP_DigestSign = nullptr;
    p_EVP_PKEY_free = nullptr;
    p_EVP_MD_CTX_free = nullptr;
}

OAuth2Helper::~OAuth2Helper() {
    CleanupOpenSSL();
}

bool OAuth2Helper::LoadOpenSSLFunctions() {
#if 0
	// Load libcrypto.dll

	UTF8String utf8str = ExtractFilePath(ExtractFileDir(Application->ExeName));
	std::string filename = "..\\lib\\curl\\libcrypto.dll";
	std::string dllPath = utf8str.c_str() + filename;

	hCryptoLib = LoadLibraryA(dllPath.c_str());
	if (!hCryptoLib) {
		SetLastError("Failed to load libcrypto.dll");
		return false;
	}

	// Load libssl.dll
    std::string filename1 = "..\\lib\\curl\\libssl.dll";
	std::string dllPath1 = utf8str.c_str() + filename1;

	hSslLib = LoadLibraryA(dllPath1.c_str());
	if (!hSslLib) {
		SetLastError("Failed to load libssl.dll");
        FreeLibrary(hCryptoLib);
        hCryptoLib = NULL;
        return false;
    }

	// Get function addresses from libcrypto.dll
	p_BIO_new_mem_buf = (BIO_new_mem_buf_t)GetProcAddress(hCryptoLib, "BIO_new_mem_buf");
	p_BIO_free = (BIO_free_t)GetProcAddress(hCryptoLib, "BIO_free");
	p_PEM_read_bio_PrivateKey = (PEM_read_bio_PrivateKey_t)GetProcAddress(hCryptoLib, "PEM_read_bio_PrivateKey");
	p_EVP_MD_CTX_new = (EVP_MD_CTX_new_t)GetProcAddress(hCryptoLib, "EVP_MD_CTX_new");
	p_EVP_sha256 = (EVP_sha256_t)GetProcAddress(hCryptoLib, "EVP_sha256");
	p_EVP_DigestSignInit = (EVP_DigestSignInit_t)GetProcAddress(hCryptoLib, "EVP_DigestSignInit");
	p_EVP_DigestSign = (EVP_DigestSign_t)GetProcAddress(hCryptoLib, "EVP_DigestSign");
	p_EVP_PKEY_free = (EVP_PKEY_free_t)GetProcAddress(hCryptoLib, "EVP_PKEY_free");
	p_EVP_MD_CTX_free = (EVP_MD_CTX_free_t)GetProcAddress(hCryptoLib, "EVP_MD_CTX_free");

	// Check if all functions were loaded successfully
	if (!p_BIO_new_mem_buf || !p_BIO_free || !p_PEM_read_bio_PrivateKey ||
		!p_EVP_MD_CTX_new || !p_EVP_sha256 || !p_EVP_DigestSignInit ||
		!p_EVP_DigestSign || !p_EVP_PKEY_free || !p_EVP_MD_CTX_free) {
		SetLastError("Failed to load one or more OpenSSL functions");
		CleanupOpenSSL();
		return false;
	}
#endif

    return true;
}

void OAuth2Helper::CleanupOpenSSL() {
    if (hCryptoLib) {
        FreeLibrary(hCryptoLib);
        hCryptoLib = NULL;
    }
    if (hSslLib) {
        FreeLibrary(hSslLib);
        hSslLib = NULL;
    }
    
    // Reset function pointers
    p_BIO_new_mem_buf = nullptr;
    p_BIO_free = nullptr;
    p_PEM_read_bio_PrivateKey = nullptr;
    p_EVP_MD_CTX_new = nullptr;
    p_EVP_sha256 = nullptr;
    p_EVP_DigestSignInit = nullptr;
    p_EVP_DigestSign = nullptr;
    p_EVP_PKEY_free = nullptr;
    p_EVP_MD_CTX_free = nullptr;
}

bool OAuth2Helper::Initialize(const std::string& credentials_path) {
    // Load OpenSSL functions first
    if (!LoadOpenSSLFunctions()) {
        return false;
    }
    
    return LoadCredentials(credentials_path);
}

bool OAuth2Helper::LoadCredentials(const std::string& credentials_path) {
    try {
        std::ifstream cred_file(credentials_path);
        if (!cred_file.is_open()) {
            SetLastError("Failed to open credentials file: " + credentials_path);
            return false;
        }

        json credentials;
        cred_file >> credentials;
        cred_file.close();

        // Extract required fields
        if (!credentials.contains("private_key") ||
            !credentials.contains("client_email") ||
            !credentials.contains("token_uri")) {
            SetLastError("Missing required fields in credentials file");
            return false;
        }

        private_key_ = credentials["private_key"].get<std::string>();
        client_email_ = credentials["client_email"].get<std::string>();
        token_uri_ = credentials["token_uri"].get<std::string>();

        initialized_ = true;
        return true;
    }
    catch (const std::exception& e) {
        SetLastError("Failed to load credentials: " + std::string(e.what()));
        return false;
    }
}

std::string OAuth2Helper::GetAccessToken() {
	if (!initialized_) {
        SetLastError("OAuth2 helper not initialized");
        return "";
    }

	// Check if current token is still valid
	if (IsTokenValid()) {
        return access_token_;
    }

    // Create new JWT and exchange for token
	if (!CreateJWT()) {
        return "";
	}

    return access_token_;
}

bool OAuth2Helper::IsTokenValid() const {
	if (access_token_.empty()) {
		return false;
	}

	// Check if token has expired (with 5 minute buffer)
    auto now = std::chrono::system_clock::now();
    auto buffer = std::chrono::minutes(5);
    return now < (token_expiry_ - buffer);
}

bool OAuth2Helper::CreateJWT() {
	try {
		std::string header = CreateJWTHeader();
		std::string claim_set = CreateJWTClaimSet();
		std::string signature = SignJWT(header, claim_set);

		if (signature.empty()) {
			return false;
        }

        // Combine JWT parts
        std::string jwt = header + "." + claim_set + "." + signature;

		// Exchange JWT for access token
		return ExchangeJWTForToken(jwt);
    }
    catch (const std::exception& e) {
        SetLastError("Failed to create JWT: " + std::string(e.what()));
        return false;
    }
}

std::string OAuth2Helper::CreateJWTHeader() {
    json header;
    header["alg"] = "RS256";
    header["typ"] = "JWT";

	std::string header_str = header.dump();
	return Base64UrlEncode(header_str);
}

std::string OAuth2Helper::CreateJWTClaimSet() {
    auto now = std::chrono::system_clock::now();
    auto expiry = now + std::chrono::hours(1);

    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    std::time_t expiry_t = std::chrono::system_clock::to_time_t(expiry);

    json claim_set;
    claim_set["iss"] = client_email_;
    claim_set["scope"] = "https://www.googleapis.com/auth/bigquery";
    claim_set["aud"] = token_uri_;
    claim_set["exp"] = expiry_t;
    claim_set["iat"] = now_t;

    std::string claim_set_str = claim_set.dump();
    return Base64UrlEncode(claim_set_str);
}

std::string OAuth2Helper::SignJWT(const std::string& header, const std::string& claim_set) {
    try {
        // Create the data to sign
        std::string data_to_sign = header + "." + claim_set;

        // Load private key
        void* bio = p_BIO_new_mem_buf(private_key_.c_str(), -1);
        if (!bio) {
            SetLastError("Failed to create BIO for private key");
            return "";
        }

        void* pkey = p_PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        p_BIO_free(bio);

        if (!pkey) {
            SetLastError("Failed to read private key");
            return "";
        }

        // Create signature context
        void* ctx = p_EVP_MD_CTX_new();
        if (!ctx) {
            p_EVP_PKEY_free(pkey);
            SetLastError("Failed to create signature context");
            return "";
        }

        // Initialize signature
        if (p_EVP_DigestSignInit(ctx, nullptr, p_EVP_sha256(), nullptr, pkey) != 1) {
            p_EVP_MD_CTX_free(ctx);
            p_EVP_PKEY_free(pkey);
            SetLastError("Failed to initialize signature");
            return "";
        }

        // Sign the data
        size_t sig_len;
        if (p_EVP_DigestSign(ctx, nullptr, &sig_len,
                          reinterpret_cast<const unsigned char*>(data_to_sign.c_str()),
                          data_to_sign.length()) != 1) {
            p_EVP_MD_CTX_free(ctx);
            p_EVP_PKEY_free(pkey);
            SetLastError("Failed to calculate signature length");
            return "";
        }

        std::vector<unsigned char> signature(sig_len);
        if (p_EVP_DigestSign(ctx, signature.data(), &sig_len,
                          reinterpret_cast<const unsigned char*>(data_to_sign.c_str()),
                          data_to_sign.length()) != 1) {
            p_EVP_MD_CTX_free(ctx);
            p_EVP_PKEY_free(pkey);
            SetLastError("Failed to create signature");
            return "";
        }

        // Clean up
        p_EVP_MD_CTX_free(ctx);
        p_EVP_PKEY_free(pkey);

        // Convert signature to base64url
        std::string sig_str(reinterpret_cast<char*>(signature.data()), sig_len);
        return Base64UrlEncode(sig_str);
    }
    catch (const std::exception& e) {
        SetLastError("Failed to sign JWT: " + std::string(e.what()));
        return "";
    }
}

bool OAuth2Helper::ExchangeJWTForToken(const std::string& jwt) {
    try {
        HttpClient http_client;

        // Initialize HTTP client
        if (!http_client.init()) {
            SetLastError("Failed to initialize HTTP client: " + http_client.getLastError());
            return false;
		}

        // Prepare request body as form-urlencoded (not JSON)
        // JWT contains dots and base64 characters that need URL encoding
        std::string encoded_jwt = UrlEncode(jwt);
        std::string request_data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + encoded_jwt;

        // Prepare headers
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/x-www-form-urlencoded";

		// Make HTTP POST request
		std::string response;

		int status_code = http_client.post(token_uri_, request_data, response, headers);

        if (status_code != 200) {
            SetLastError("Token exchange failed with status code: " + std::to_string(status_code));
            return false;
        }

		// Parse response
        json response_json = json::parse(response);

        if (!response_json.contains("access_token")) {
            SetLastError("No access token in response");
            return false;
        }

		access_token_ = response_json["access_token"].get<std::string>();

        // Calculate expiry time
        int expires_in = response_json.value("expires_in", 3600);
        auto now = std::chrono::system_clock::now();
        token_expiry_ = now + std::chrono::seconds(expires_in);

        return true;
    }
    catch (const std::exception& e) {
        SetLastError("Failed to exchange JWT for token: " + std::string(e.what()));
        return false;
    }
}

std::string OAuth2Helper::Base64UrlEncode(const std::string& input) {
    // Simple base64url encoding (for production, use a proper library)
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string encoded;
    int val = 0, valb = -6;

    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

	// Convert to base64url (replace + with -, / with _, remove padding)
    std::replace(encoded.begin(), encoded.end(), '+', '-');
    std::replace(encoded.begin(), encoded.end(), '/', '_');

    // Remove padding
    while (encoded.back() == '=') {
        encoded.pop_back();
    }

	return encoded;
}

std::string OAuth2Helper::UrlEncode(const std::string& input) {
    std::string encoded;
    for (char c : input) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            char hex[4];
            sprintf(hex, "%%%02X", (unsigned char)c);
            encoded += hex;
        }
    }
    return encoded;
}

void OAuth2Helper::SetLastError(const std::string& error) {
    last_error_ = error;
}

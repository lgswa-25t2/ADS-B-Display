#ifndef OAUTH2_HELPER_H
#define OAUTH2_HELPER_H

// Windows 매크로 충돌 방지
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

// Prevent OpenSSL from overriding Windows crypto definitions
#define OPENSSL_NO_WINCRYPT
#define OPENSSL_NO_ENGINE
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_NO_EC
#define OPENSSL_NO_DH
#define OPENSSL_NO_DSA

// Define Windows crypto types before OpenSSL includes them
#ifndef WINCRYPT_DEFINED
#define WINCRYPT_DEFINED
#define CRYPT_STRING_BASE64 0x00000001
#define CRYPT_STRING_HEX 0x00000004
#define CRYPT_STRING_HEXRAW 0x0000000c
#define CRYPT_STRING_HEXASCII 0x00000005
#define CRYPT_STRING_BASE64HEADER 0x00000000
#define CRYPT_STRING_BASE64REQUESTHEADER 0x00000003
#define CRYPT_STRING_HEXADDR 0x0000000a
#define CRYPT_STRING_HEXASCIIADDR 0x0000000b
#define CRYPT_STRING_HEXRAWADDR 0x0000000e
#define CRYPT_STRING_BASE64X509CRLHEADER 0x00000009
#endif

// Disable specific OpenSSL features that conflict with Windows
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

#include <string>
#include <chrono>
#include <vector>
#include <windows.h>

// OpenSSL 함수 포인터 타입들 (void* 사용으로 타입 충돌 방지)
typedef void* (*BIO_new_mem_buf_t)(const void*, int);
typedef int (*BIO_free_t)(void*);
typedef void* (*PEM_read_bio_PrivateKey_t)(void*, void**, int (*)(char*, int, int, void*), void*);
typedef void* (*EVP_MD_CTX_new_t)(void);
typedef const void* (*EVP_sha256_t)(void);
typedef int (*EVP_DigestSignInit_t)(void*, void**, const void*, void*, void*);
typedef int (*EVP_DigestSign_t)(void*, unsigned char*, size_t*, const unsigned char*, size_t);
typedef void (*EVP_PKEY_free_t)(void*);
typedef void (*EVP_MD_CTX_free_t)(void*);

class OAuth2Helper {
public:
    OAuth2Helper();
    ~OAuth2Helper();

    // Initialize with service account credentials
    bool Initialize(const std::string& credentials_path);

    // Get access token (will refresh if expired)
    std::string GetAccessToken();

    // Check if token is valid
    bool IsTokenValid() const;

    // Get last error
    std::string GetLastError() const { return last_error_; }

private:
    std::string private_key_;
    std::string client_email_;
    std::string token_uri_;
    std::string access_token_;
    std::chrono::system_clock::time_point token_expiry_;
    std::string last_error_;
    bool initialized_;

    // OpenSSL DLL handles and function pointers
    HMODULE hCryptoLib;
    HMODULE hSslLib;
    
    // OpenSSL function pointers
    BIO_new_mem_buf_t p_BIO_new_mem_buf;
    BIO_free_t p_BIO_free;
    PEM_read_bio_PrivateKey_t p_PEM_read_bio_PrivateKey;
    EVP_MD_CTX_new_t p_EVP_MD_CTX_new;
    EVP_sha256_t p_EVP_sha256;
    EVP_DigestSignInit_t p_EVP_DigestSignInit;
    EVP_DigestSign_t p_EVP_DigestSign;
    EVP_PKEY_free_t p_EVP_PKEY_free;
    EVP_MD_CTX_free_t p_EVP_MD_CTX_free;

    // Helper methods
    bool LoadCredentials(const std::string& credentials_path);
    bool LoadOpenSSLFunctions();
    bool CreateJWT();
    bool ExchangeJWTForToken(const std::string& jwt);
    std::string Base64UrlEncode(const std::string& input);
    std::string UrlEncode(const std::string& input);
    std::string CreateJWTHeader();
    std::string CreateJWTClaimSet();
    std::string SignJWT(const std::string& header, const std::string& claim_set);
    void SetLastError(const std::string& error);
    void CleanupOpenSSL();
};

#endif // OAUTH2_HELPER_H

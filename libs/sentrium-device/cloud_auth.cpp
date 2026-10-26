#include <sentrium/cloud_auth.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>

#include <array>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>

namespace sentrium::device {
namespace {

class HashAlgorithm {
public:
    HashAlgorithm(LPCWSTR name, ULONG flags) {
        if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&handle_,name,nullptr,flags))) {
            throw std::runtime_error("Cloud device cryptography provider unavailable");
        }
    }
    ~HashAlgorithm() { if (handle_) BCryptCloseAlgorithmProvider(handle_,0); }
    HashAlgorithm(const HashAlgorithm&) = delete;
    HashAlgorithm& operator=(const HashAlgorithm&) = delete;
    [[nodiscard]] BCRYPT_ALG_HANDLE get() const noexcept { return handle_; }
private:
    BCRYPT_ALG_HANDLE handle_ = nullptr;
};

std::array<std::uint8_t,16> md5(std::string_view input) {
    HashAlgorithm alg(BCRYPT_MD5_ALGORITHM,0);
    std::array<std::uint8_t,16> out{};
    if (!BCRYPT_SUCCESS(BCryptHash(
        alg.get(), nullptr, 0,
        reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
        static_cast<ULONG>(input.size()),out.data(),
        static_cast<ULONG>(out.size())))) {
        throw std::runtime_error("Cloud device MD5 hash failed");
    }
    return out;
}

std::array<std::uint8_t,32> hmacSha256(
    std::string_view key, std::string_view message) {
    HashAlgorithm alg(BCRYPT_SHA256_ALGORITHM,BCRYPT_ALG_HANDLE_HMAC_FLAG);
    std::array<std::uint8_t,32> out{};
    if (!BCRYPT_SUCCESS(BCryptHash(
        alg.get(),
        reinterpret_cast<PUCHAR>(const_cast<char*>(key.data())),
        static_cast<ULONG>(key.size()),
        reinterpret_cast<PUCHAR>(const_cast<char*>(message.data())),
        static_cast<ULONG>(message.size()),out.data(),
        static_cast<ULONG>(out.size())))) {
        throw std::runtime_error("Cloud device HMAC-SHA256 failed");
    }
    return out;
}

std::array<std::uint8_t,32> pbkdf2Sha256(
    std::string_view key, std::string_view salt) {
    HashAlgorithm alg(BCRYPT_SHA256_ALGORITHM,BCRYPT_ALG_HANDLE_HMAC_FLAG);
    std::array<std::uint8_t,32> out{};
    if (!BCRYPT_SUCCESS(BCryptDeriveKeyPBKDF2(
        alg.get(),
        reinterpret_cast<PUCHAR>(const_cast<char*>(key.data())),
        static_cast<ULONG>(key.size()),
        reinterpret_cast<PUCHAR>(const_cast<char*>(salt.data())),
        static_cast<ULONG>(salt.size()),
        20000ULL,out.data(),static_cast<ULONG>(out.size()),0))) {
        throw std::runtime_error("Cloud device PBKDF2 failed");
    }
    return out;
}

std::string upperHex(const std::array<std::uint8_t,16>& raw) {
    constexpr char kHex[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(32);
    for (const std::uint8_t b:raw) {
        out.push_back(kHex[b >> 4]);
        out.push_back(kHex[b & 15]);
    }
    return out;
}

std::string base64(const std::array<std::uint8_t,32>& raw) {
    constexpr char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(44);
    for (std::size_t i=0;i<raw.size();i+=3) {
        const std::uint32_t bits =
            (std::uint32_t(raw[i]) << 16) |
            (i+1<raw.size()?std::uint32_t(raw[i+1]) << 8:0) |
            (i+2<raw.size()?std::uint32_t(raw[i+2]):0);
        out.push_back(alphabet[(bits >> 18)&63]);
        out.push_back(alphabet[(bits >> 12)&63]);
        out.push_back(i+1<raw.size()?alphabet[(bits >> 6)&63]:'=');
        out.push_back(i+2<raw.size()?alphabet[bits & 63]:'=');
    }
    return out;
}

} // namespace

CloudDeviceAuth::~CloudDeviceAuth() noexcept {
    if (!device_key_hex.empty()) {
        SecureZeroMemory(device_key_hex.data(),device_key_hex.size());
    }
    if (!signature_base64.empty()) {
        SecureZeroMemory(signature_base64.data(),signature_base64.size());
    }
    SecureZeroMemory(address_key.data(),address_key.size());
}

CloudDeviceAuth prepareCloudDeviceAuth(
    std::string_view username,std::string_view password,
    std::string_view device_randsalt,std::uint32_t nonce,
    std::uint64_t unix_seconds,std::string_view payload) {
    // Control-byte rejection also avoids unsafe downstream XML fields; XML
    // escaping and actual request generation belong to the protocol layer.
    auto checked = [](std::string_view v,std::size_t limit) {
        if (v.empty() || v.size()>limit) return false;
        for (unsigned char c:v) {
            if (c<0x20 || c==0x7f) return false;
        }
        return true;
    };
    if (!checked(username,128) || !checked(password,256) ||
        !checked(device_randsalt,128) ||
        payload.size()>4096 || nonce==0 || unix_seconds==0) {
        throw std::invalid_argument("Cloud device authentication fields invalid");
    }

    std::string raw = std::string(username)+":Login to "+
        std::string(device_randsalt)+":"+std::string(password);
    CloudDeviceAuth output;
    try {
        output.device_key_hex = upperHex(md5(raw));
    } catch (...) {
        SecureZeroMemory(raw.data(),raw.size());
        throw;
    }
    SecureZeroMemory(raw.data(),raw.size());
    const std::string nonceStr=std::to_string(nonce);
    const std::string toSign=nonceStr+std::to_string(unix_seconds)+std::string(payload);
    output.signature_base64=base64(hmacSha256(output.device_key_hex,toSign));
    output.address_key=pbkdf2Sha256(output.device_key_hex,nonceStr);
    return output;
}


std::vector<std::uint8_t> cryptCloudAddress(
    const std::array<std::uint8_t,32>& derived_key,
    const std::array<std::uint8_t,16>& iv,
    std::string_view input) {
    if (input.size() > 4096) {
        throw std::invalid_argument("Cloud device address exceeds maximum");
    }
    if (input.empty()) return {};

    HashAlgorithm aes(BCRYPT_AES_ALGORITHM,0);
    constexpr wchar_t kEcb[] = BCRYPT_CHAIN_MODE_ECB;
    if (!BCRYPT_SUCCESS(BCryptSetProperty(
        aes.get(),BCRYPT_CHAINING_MODE,
        reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(kEcb)),
        static_cast<ULONG>(sizeof(kEcb)),0))) {
        throw std::runtime_error("Cloud AES mode unavailable");
    }

    DWORD objectSize=0;
    DWORD resultLength=0;
    if (!BCRYPT_SUCCESS(BCryptGetProperty(
        aes.get(),BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectSize),
        static_cast<ULONG>(sizeof(objectSize)),&resultLength,0)) ||
        objectSize==0 || objectSize>4096) {
        throw std::runtime_error("Cloud AES key object unavailable");
    }

    std::vector<std::uint8_t> keyObject(objectSize);
    BCRYPT_KEY_HANDLE key=nullptr;
    if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(
        aes.get(),&key,keyObject.data(),objectSize,
        const_cast<PUCHAR>(derived_key.data()),
        static_cast<ULONG>(derived_key.size()),0))) {
        SecureZeroMemory(keyObject.data(),keyObject.size());
        throw std::runtime_error("Cloud AES key preparation failed");
    }

    // CNG's ECB encrypt acts only on the evolving 16-byte feedback block.
    // OFB never pads and works for arbitrary plaintext lengths.
    std::array<std::uint8_t,16> feedback=iv;
    std::vector<std::uint8_t> out(input.size());
    try {
        for (std::size_t pos=0;pos<input.size();pos+=feedback.size()) {
            std::array<std::uint8_t,16> stream{};
            ULONG written=0;
            if (!BCRYPT_SUCCESS(BCryptEncrypt(
                key,feedback.data(),static_cast<ULONG>(feedback.size()),
                nullptr,nullptr,0,
                stream.data(),static_cast<ULONG>(stream.size()),
                &written,0)) || written!=static_cast<ULONG>(stream.size())) {
                throw std::runtime_error("Cloud AES keystream failed");
            }
            feedback=stream;
            for (std::size_t i=0;i<feedback.size() && pos+i<input.size();++i) {
                out[pos+i]=static_cast<std::uint8_t>(
                    static_cast<unsigned char>(input[pos+i]) ^ stream[i]);
            }
            SecureZeroMemory(stream.data(),stream.size());
        }
    } catch (...) {
        BCryptDestroyKey(key);
        SecureZeroMemory(keyObject.data(),keyObject.size());
        SecureZeroMemory(feedback.data(),feedback.size());
        throw;
    }
    BCryptDestroyKey(key);
    SecureZeroMemory(keyObject.data(),keyObject.size());
    SecureZeroMemory(feedback.data(),feedback.size());
    return out;
}

} // namespace sentrium::device

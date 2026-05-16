#include "cuckoo_filter.hpp"

CuckooFilter::CuckooFilter(size_t capacity) {
    this->capacity = capacity + BUCKET_SIZE - (capacity % BUCKET_SIZE);
    bucket_cnt = capacity / BUCKET_SIZE;

    for (size_t i = 0; i < bucket_cnt; i++) {
        table.emplace_back();
        table.back().list.resize(BUCKET_SIZE);
    }
}

CuckooFilter::~CuckooFilter() {
    table.clear();
    table.shrink_to_fit();
}

void CuckooFilter::clearFilter() {
    table.clear();
    table.shrink_to_fit();
}

uint16_t CuckooFilter::getFingerprint(const std::string& item) {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(item.c_str()), item.size(), md5);

    uint16_t result = 0;
    for (size_t i = 0; i < FINGERPRINT_LEN; i++)
        result |= (static_cast<uint16_t>(md5[i]) << (i * sizeof(uint64_t)));

    return result;
}

size_t CuckooFilter::getBucketIndex(const std::string& item) {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(item.c_str()), item.size(), md5);

    size_t result = 0;
    for (size_t i = 0; i < HASH_LEN; i++)
        result |= (static_cast<size_t>(md5[i]) << (i * sizeof(uint64_t)));

    result %= bucket_cnt;

    return result;
}

size_t CuckooFilter::getAltBucketIndex(const size_t& firstBucketIndex, const uint16_t& fingerprint) {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(&fingerprint), sizeof(fingerprint), md5);

    size_t hash = 0;
    for (size_t i = 0; i < HASH_LEN; i++)
        hash |= (static_cast<size_t>(md5[i]) << (i * sizeof(uint64_t)));

    hash %= bucket_cnt;
    size_t result = firstBucketIndex ^ hash; // XOR

    return result;
}
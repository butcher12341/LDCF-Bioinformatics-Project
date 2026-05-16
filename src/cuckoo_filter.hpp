#ifndef CUCKOO_FILTER_HPP
#define CUCKOO_FILTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define BUCKET_SIZE 4
#define FINGERPRINT_LEN 2   // bytes
#define HASH_LEN 8          // bytes

typedef struct{
	std::vector<uint16_t> list;
} Bucket;

class CuckooFilter {
private:
    size_t capacity;
    size_t bucket_cnt;
    std::vector<Bucket> table;
    
    uint16_t getFingerprint(const std::string& item);
    size_t getBucketIndex(const std::string& item);
    size_t getAltBucketIndex(const size_t& firstBucketIndex, const uint16_t& fingerprint);
public:
    CuckooFilter(size_t capacity);
    ~CuckooFilter();
    bool insert(const std::string& item);
    bool contains(const std::string& item) const;
    bool remove(const std::string& item);
    void clearFilter();
};

#endif // CUCKOO_FILTER_HPP
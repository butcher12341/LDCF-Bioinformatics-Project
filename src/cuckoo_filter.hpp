#ifndef CUCKOO_FILTER_HPP
#define CUCKOO_FILTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <openssl/md5.h>

#define BUCKET_SIZE 4
#define FINGERPRINT_LEN 2   // bytes
#define HASH_LEN 8          // bytes
#define MAX_DEPTH 500

typedef struct{
	std::vector<uint16_t> bucket;
} Bucket;

/*
 * Author: Andrija Macek
 */
class CuckooFilter {
private:
    size_t capacity;
    size_t size;
    size_t bucketCnt;
    std::vector<Bucket> table;
    
    // Get the fingerprint of the item using MD5 hash
    uint16_t getFingerprint(const std::string& item) const;
    // Get the first bucket index of the item using MD5 hash
    size_t getBucketIndex(const std::string& item) const;
    // Get the second bucket index of the item using the first bucket index and the fingerprint
    size_t getAltBucketIndex(const size_t& firstBucketIndex, const uint16_t& fingerprint) const;
    // Move fingerprint from bucket index to alternative bucket, returns false if not possible
    bool changeBucket(const size_t& bucketIndex, const uint16_t& fingerprint, size_t depth);
public:
    // Create the cuckoo filter object with the given capacity
    CuckooFilter(size_t capacity);
    // Destroy the cuckoo filter object
    ~CuckooFilter();
    // Prints the first few buckets of the filter
    void printFilter();
    // Get the current number of fingerprints stored in the filter
    size_t getSize() { return size; }
    // Get the max number of fingerprints that can be stored in the filter
    size_t getCapacity() { return capacity; }
    // Get the current load factor of the filter
    double getLoadFactor() { return static_cast<double>(size)/static_cast<double>(capacity); }
    // Insert the item into the cuckoo filter, returns false if not possible
    bool insert(const std::string& item);
    // Returns true if item exist in filter with high probability, returns false if it doesn't
    bool contains(const std::string& item) const;
    // Remove item from the filter, returns false if item isn't in the filter
    bool remove(const std::string& item);
    // Clear the filter and release the memory, ie. for spliting the filter
    void clearFilter();
};

#endif // CUCKOO_FILTER_HPP
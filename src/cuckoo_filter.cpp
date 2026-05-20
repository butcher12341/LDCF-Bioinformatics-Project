#include "cuckoo_filter.hpp"

CuckooFilter::CuckooFilter(size_t capacity) {
    // Get the first bigger capacity that is divisible by the bucket size
    this->capacity = capacity + BUCKET_SIZE - (capacity % BUCKET_SIZE);
    // Get the appropriate number of buckets for that capacity
    bucketCnt = capacity / BUCKET_SIZE;
    // Bit Twiddling Hack to get the next bigger power of 2 number
    // Need power of 2 buckets to use as bitmask for indexes
    bucketCnt--;
    for (size_t i = 1; i < (sizeof(size_t) * sizeof(uint64_t)); i *= 2)
        bucketCnt |= bucketCnt >> i;
    bucketCnt++;
    // Adjust capacity acording to the number of buckets
    this->capacity = bucketCnt * BUCKET_SIZE;

    for (size_t i = 0; i < bucketCnt; i++)
        table.emplace_back();
    size = 0;
}

CuckooFilter::~CuckooFilter() {
    table.clear();
    table.shrink_to_fit();
}

void CuckooFilter::printFilter() const {
    if (capacity == 0)
        return;

    size_t loopCnt = bucketCnt > BUCKET_SIZE ? BUCKET_SIZE : bucketCnt;
    for (size_t i = 0; i < loopCnt; i++) {
        for (size_t j = 0; j < BUCKET_SIZE; j++) {
            if (j < table[i].bucket.size())
                std::cout << table[i].bucket[j] << " ";
            else
                std::cout << "- ";
        }
        std::cout << std::endl;
    }
    if (loopCnt < bucketCnt)
        std::cout << "...." << std::endl;
    std::cout << std::endl;
}

double CuckooFilter::getLoadFactor() const {
    if (capacity > 0)
        return static_cast<double>(size)/static_cast<double>(capacity);
    else
        return 0;
}

bool CuckooFilter::insert(const std::string& item) {
    // This is ok because C++ evaluates one at a time
    // If capacity is 0, getLoadFactor won't be called
    if (capacity == 0 || getLoadFactor() == 1)
        return false;

    uint32_t fingerprint = getFingerprint(item);
    size_t firstBucket = getBucketIndex(item);

    return insert(fingerprint, firstBucket);
}

bool CuckooFilter::insert(const uint32_t fingerprint, const size_t bucket) {
    if (capacity == 0 || getLoadFactor() == 1)
        return false;

    size_t secondBucket = getAltBucketIndex(bucket, fingerprint);
    int randomNum = rand() % BUCKET_SIZE;

    if (table[bucket].bucket.size() < BUCKET_SIZE)
        table[bucket].bucket.push_back(fingerprint);
    else if (table[secondBucket].bucket.size() < BUCKET_SIZE)
        table[secondBucket].bucket.push_back(fingerprint);
    else if (changeBucket(bucket, table[bucket].bucket[randomNum], 1, randomNum))
        table[bucket].bucket.push_back(fingerprint);
    else if (changeBucket(secondBucket, table[secondBucket].bucket[randomNum], 1, randomNum))
        table[secondBucket].bucket.push_back(fingerprint);
    else
        return false;
    size++;
    return true;
}

bool CuckooFilter::contains(const std::string& item) const {
    if (capacity == 0)
        return false;

    uint32_t fingerprint = getFingerprint(item);
    size_t firstBucket = getBucketIndex(item);
    size_t secondBucket = getAltBucketIndex(firstBucket, fingerprint);

    for (auto i = table[firstBucket].bucket.begin(); i != table[firstBucket].bucket.end(); i++)
        if (i[0] == fingerprint)
            return true;
    for (auto i = table[secondBucket].bucket.begin(); i != table[secondBucket].bucket.end(); i++)
        if (i[0] == fingerprint)
            return true;

    return false;
}

bool CuckooFilter::remove(const std::string& item) {
    if (capacity == 0)
        return false;

    uint32_t fingerprint = getFingerprint(item);
    size_t firstBucket = getBucketIndex(item);
    size_t secondBucket = getAltBucketIndex(firstBucket, fingerprint);

    for (auto i = table[firstBucket].bucket.begin(); i != table[firstBucket].bucket.end(); i++) {
        if (i[0] == fingerprint) {
            table[firstBucket].bucket.erase(i);
            size--;
            return true;
        }
    }
    for (auto i = table[secondBucket].bucket.begin(); i != table[secondBucket].bucket.end(); i++) {
        if (i[0] == fingerprint) {
            table[secondBucket].bucket.erase(i);
            size--;
            return true;
        }
    }

    return false;
}

void CuckooFilter::clearFilter() {
    table.clear();
    table.shrink_to_fit();
    size = 0;
    capacity = 0;
}

uint32_t CuckooFilter::getFingerprint(const std::string& item) {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(item.c_str()), item.size(), md5);

    uint32_t result = 0;
    for (size_t i = 0; i < FINGERPRINT_LEN; i++)
        result |= (static_cast<uint32_t>(md5[i]) << (i * sizeof(uint64_t)));

    return result;
}

size_t CuckooFilter::getBucketIndex(const std::string& item) const {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(item.c_str()), item.size(), md5);

    size_t result = 0;
    for (size_t i = 0; i < HASH_LEN; i++)
        result |= (static_cast<size_t>(md5[i]) << (i * sizeof(uint64_t)));

    // Use bucket count as bitmask for the index
    result &= (bucketCnt - 1);

    return result;
}

size_t CuckooFilter::getAltBucketIndex(const size_t& firstBucketIndex, const uint32_t& fingerprint) const {
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(&fingerprint), sizeof(fingerprint), md5);

    size_t hash = 0;
    for (size_t i = 0; i < HASH_LEN; i++)
        hash |= (static_cast<size_t>(md5[i]) << (i * sizeof(uint64_t)));

    hash |= 1; // So that the 2 buckets aren't same
    size_t result = firstBucketIndex ^ hash; // XOR
    // Use bucket count as bitmask for the index
    result &= (bucketCnt - 1);

    return result;
}

bool CuckooFilter::changeBucket(const size_t& bucketIndex, const uint32_t fingerprint, size_t depth, size_t pos) {
    if (depth == MAX_DEPTH)
        return false;

    size_t altBucket = getAltBucketIndex(bucketIndex, fingerprint);

    if (table[altBucket].bucket.size() < BUCKET_SIZE) {
        table[altBucket].bucket.push_back(fingerprint);
        table[bucketIndex].bucket.erase(table[bucketIndex].bucket.begin() + pos);
        return true;
    }

    size_t randomNum = rand() % BUCKET_SIZE;
    if (changeBucket(altBucket, table[altBucket].bucket[randomNum], depth + 1, randomNum)) {
        table[altBucket].bucket.push_back(fingerprint);
        for (size_t i = 0; i < table[bucketIndex].bucket.size(); i++) {
            if (table[bucketIndex].bucket[i] == fingerprint) {
                table[bucketIndex].bucket.erase(table[bucketIndex].bucket.begin() + i);
                return true;
            }
        }
    }

    return false;
}
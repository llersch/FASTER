#pragma once

#include "core/faster.h"
#include <cstring>
#include <type_traits>

template<typename T>
static T fnv1a(const void* data, size_t size)
{
    static_assert(
        std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
        "fnv1a only supports 32 bits and 64 bits variants."
    );

    static constexpr T INIT = std::is_same<T, uint32_t>::value ? 2166136261u : 14695981039346656037ul;
    static constexpr T PRIME = std::is_same<T, uint32_t>::value ? 16777619 : 1099511628211ul;

    auto src = reinterpret_cast<const char*>(data);
    T sum = INIT;
    while (size--)
    {
        sum = (sum ^ *src) * PRIME;
        src++;
    }
    return sum;
}

class Key {
public:
    /// This constructor is called when creating a Context so we keep track of memory containing key
    Key(const char* key, const uint64_t key_length)
        : temp_buffer{ key }
        , key_length_{ key_length } {
    }

    /// This constructor is called when record is being allocated so we can freely copy into our buffer
    Key(const Key& other) {
        key_length_ = other.key_length_;
        temp_buffer = NULL;
        if (other.temp_buffer == NULL) {
            memcpy(buffer(), other.buffer(), key_length_);
        } else {
            memcpy(buffer(), other.temp_buffer, key_length_);
        }
    }

    /// This destructor ensures we don't leak memory due to Key objects not allocated on HybridLog
    ~Key() {
        if (this->temp_buffer != NULL) {
            //free((void*)temp_buffer);
        }
    }

    /// Methods and operators required by the (implicit) interface:
    inline uint32_t size() const {
        return static_cast<uint32_t>(sizeof(Key) + key_length_);
    }
    inline KeyHash GetHash() const {
        if (this->temp_buffer != NULL) {
            uint64_t h = fnv1a<uint64_t>(temp_buffer, key_length_);
            return KeyHash(h);
        }
        uint64_t h = fnv1a<uint64_t>(buffer(), key_length_);
        return KeyHash(h);
    }

    /// Comparison operators.
    inline bool operator==(const Key& other) const {
        if (this->key_length_ != other.key_length_) return false;
        if (this->temp_buffer != NULL) {
            return memcmp(temp_buffer, other.buffer(), key_length_) == 0;
        }
        return memcmp(buffer(), other.buffer(), key_length_) == 0;
    }

    inline bool operator!=(const Key& other) const {
        if (this->key_length_ != other.key_length_) return true;
        if (this->temp_buffer != NULL) {
            return memcmp(temp_buffer, other.buffer(), key_length_) != 0;
        }
        return memcmp(buffer(), other.buffer(), key_length_) != 0;
    }

private:
    uint64_t key_length_;
    const char* temp_buffer;

    inline const char* buffer() const {
        return reinterpret_cast<const char*>(this + 1);
    }
    inline char* buffer() {
        return reinterpret_cast<char*>(this + 1);
    }
};

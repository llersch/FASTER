#include "key.h"
#include "value.h"
#include "core/faster.h"

class UpsertContext : public IAsyncContext {
public:
    typedef Key key_t;
    typedef Value value_t;

    UpsertContext(const char* key, uint64_t key_length, const char* value, uint32_t value_length)
        : key_{ key, key_length }
        , value_buffer_{ value }
        , value_length_{ value_length } {
    }

    /// Copy (and deep-copy) constructor.
    UpsertContext(const UpsertContext& other)
        : key_{ other.key_ }
        , value_buffer_{ other.value_buffer_ }
        , value_length_{ other.value_length_ } {
    }

    /// The implicit and explicit interfaces require a key() accessor.
    inline const Key& key() const {
        return key_;
    }

    inline uint32_t value_size() const {
        return sizeof(Value) + value_length_;
    }

    /// Non-atomic and atomic Put() methods.
    inline void Put(Value& value) {
        value.gen_lock_.store(0);
        value.size_ = sizeof(Value) + value_length_;
        value.length_ = value_length_;
        std::memcpy(value.buffer(), value_buffer_, value_length_);
    }

    inline bool PutAtomic(Value& value) {
        bool replaced;
        while(!value.gen_lock_.try_lock(replaced) && !replaced) {
            std::this_thread::yield();
        }
        if(replaced) {
            // Some other thread replaced this record.
            return false;
        }
        if(value.size_ < sizeof(Value) + value_length_) {
            // Current value is too small for in-place update.
            value.gen_lock_.unlock(true);
            return false;
        }
        // In-place update overwrites length and buffer, but not size.
        // In-place update overwrites length and buffer, but not size.
        value.length_ = value_length_;
        std::memcpy(value.buffer(), value_buffer_, value_length_);
        value.gen_lock_.unlock(false);
        return true;
    }

protected:
    /// The explicit interface requires a DeepCopy_Internal() implementation.
    Status DeepCopy_Internal(IAsyncContext*& context_copy) {
        return IAsyncContext::DeepCopy_Internal(*this, context_copy);
    }

private:
    Key key_;
    const char* value_buffer_;
    uint32_t value_length_;
}; 

class ReadContext : public IAsyncContext {
public:
    typedef Key key_t;
    typedef Value value_t;

    ReadContext(const char* key, uint64_t key_length, char* out)
        : key_{ key, key_length }
        , output_length{ 0 }
        , output_buffer{ out } {
    }

    /// Copy (and deep-copy) constructor.
    ReadContext(const ReadContext& other)
        : key_{ other.key_ }
        , output_length{ 0 } {
    }

    /// The implicit and explicit interfaces require a key() accessor.
    inline const Key& key() const {
        return key_;
    }

    inline void Get(const Value& value) {
        // All reads should be atomic (from the mutable tail).
        //ASSERT_TRUE(false);
    }

    inline void GetAtomic(const Value& value) {
        GenLock before, after;
        do {
        before = value.gen_lock_.load();
        output_length = value.length_;
        std::memcpy(output_buffer, value.buffer(), value.length_);
        //output_bytes[0] = value.buffer()[0];
        //output_bytes[1] = value.buffer()[value.length_ - 1];
        after = value.gen_lock_.load();
        } while(before.gen_number != after.gen_number);
    }

protected:
    /// The explicit interface requires a DeepCopy_Internal() implementation.
    Status DeepCopy_Internal(IAsyncContext*& context_copy) {
        return IAsyncContext::DeepCopy_Internal(*this, context_copy);
    }

private:
    Key key_;
public:
    uint8_t output_length;
    // Extract two bytes of output.
    char* output_buffer;
};

#pragma once

#include "genlock.h"
#include <cstdint>

class UpsertContext;
class ReadContext;

class Value {
public:
    Value()
        : gen_lock_{ 0 }
        , size_{ 0 }
        , length_{ 0 } {
    }

    inline uint32_t size() const {
        return size_;
    }

    friend class UpsertContext;
    friend class ReadContext;

private:
    AtomicGenLock gen_lock_;
    uint32_t size_;
    uint32_t length_;

    inline const uint8_t* buffer() const {
        return reinterpret_cast<const uint8_t*>(this + 1);
    }
    inline uint8_t* buffer() {
        return reinterpret_cast<uint8_t*>(this + 1);
    }
};

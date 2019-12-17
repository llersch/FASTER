#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct faster_t faster_t;

extern faster_t* faster_create();
extern void faster_close(faster_t* kv);
extern bool faster_get(faster_t* kv, const char* key, uint64_t key_len);
extern bool faster_put(faster_t* kv, const char* key, uint64_t key_len,
                                     const char* val, uint64_t val_len);

#ifdef __cplusplus
}
#endif

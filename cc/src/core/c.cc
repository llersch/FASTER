#include "key.h"
#include "value.h"
#include "context.h"

#include "c.h"
#include "device/null_disk.h"

extern "C" {

struct faster_t {
    typedef FASTER::core::FasterKv<Key, Value, FASTER::device::NullDisk> FasterKv;

    FasterKv* rep;
};

faster_t* faster_create(uint64_t table_size, uint64_t log_size, const char* filename) {
    faster_t* kv = new faster_t;
    kv->rep = new faster_t::FasterKv(table_size, log_size, filename);
    kv->rep->StartSession();
    return kv;
}

void faster_close(faster_t* kv) {
    Guid token;
    kv->rep->Checkpoint(nullptr, nullptr, token);
    kv->rep->CompletePending(false);
    kv->rep->Refresh();
    kv->rep->CompletePending(true);
    kv->rep->StopSession();
    delete kv->rep;
    delete kv;
}

void faster_start_session(faster_t* kv) {
    kv->rep->StartSession();
}

void faster_stop_session(faster_t* kv) {
    kv->rep->StopSession();
}

uint64_t faster_get(faster_t* kv, const char* key, uint64_t key_len, char* out) {
    auto callback = [](IAsyncContext* ctxt, Status results)
    {
        CallbackContext<ReadContext> context{ ctxt };
    };

    ReadContext ctx{ key, key_len, out};
    Status result = kv->rep->Read(ctx, callback, 1);
    return result == Status::Ok ? ctx.output_length : 0;
}

bool faster_put(faster_t* kv, const char* key, uint64_t key_len,
                                const char* val, uint64_t val_len) {
    auto callback = [](IAsyncContext* ctxt, Status results)
    {
        CallbackContext<UpsertContext> context{ ctxt };
    };

    UpsertContext ctx{ key, key_len, val, val_len };
    Status result = kv->rep->Upsert(ctx, callback, 1);

    return result == Status::Ok;
}

};

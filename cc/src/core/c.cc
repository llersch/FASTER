#include "key.h"
#include "value.h"
#include "context.h"

#include "c.h"
#include "device/file_system_disk.h"

extern "C" {

struct faster_t {
    typedef FASTER::environment::QueueIoHandler QueueIoHandler;
    typedef FASTER::device::FileSystemDisk<QueueIoHandler, 1073741824L> FileSystemDisk;
    typedef FASTER::core::FasterKv<Key, Value, FileSystemDisk> FasterKv;

    FasterKv* rep;
};

faster_t* faster_create() {
    static constexpr uint64_t kKeySpace = (1L << 15);
    faster_t* kv = new faster_t;
    kv->rep = new FASTER::core::FasterKv<Key, Value, FASTER::device::FileSystemDisk<FASTER::environment::QueueIoHandler, 1073741824L>>(kKeySpace, 192ULL << 20, "sample_storage");
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

bool faster_get(faster_t* kv, const char* key, uint64_t key_len) {
    auto callback = [](IAsyncContext* ctxt, Status results)
    {
        CallbackContext<ReadContext> context{ ctxt };
    };

    ReadContext ctx{ key, key_len };
    Status result = kv->rep->Read(ctx, callback, 1);
    return result == Status::Ok;
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

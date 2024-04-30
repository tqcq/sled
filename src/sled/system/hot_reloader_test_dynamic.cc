#include "sled/lang/attributes.h"
#include <stdio.h>

void
crash()
{
    int *addr = nullptr;
    (void) ++*addr;
}

SLED_EXPORT int
sled_hot_loader_main(void *ctx, int op)
{
    struct ctx_impl {
        void *p;
        void *userdata;
    };

    void *userdata = reinterpret_cast<ctx_impl *>(ctx)->userdata;
    int *value_ptr = reinterpret_cast<int *>(userdata);
    int crash_type = *value_ptr;
    *value_ptr     = -1;
    fprintf(stderr, "plugin sled_hot_loader_main: op=%d, crash=%d\n", op, crash_type);
    // LOGD("plugin ", "sled_hot_loader_main: op={}, crash={}", op, crash_type);

    switch (op) {
        // On Load
    case 0: {
        if (crash_type == 0) { crash(); }
        break;
    }
        // On Step
    case 1: {
        if (crash_type == 1) { crash(); }
        break;
    }
        // OnUnload
    case 2: {
        if (crash_type == 2) { crash(); }

        break;
    }
        // OnClose
    case 3: {
        if (crash_type == 3) { crash(); }
        break;
    }
    default:
        break;
    }
    return 0;
}

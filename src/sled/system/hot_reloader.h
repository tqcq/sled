#ifndef SLED_SYSTEM_HOT_LOADER_H
#define SLED_SYSTEM_HOT_LOADER_H

#pragma once
#include "sled/nonstd/string_view.h"
#include "sled/optional.h"
#include <memory>

// hot reload library should implement
// sled_hot_loader_main function
// if success return 0,
// otherwise return negative value (reversed -1, -2)
// int sled_hot_loader_main(void* ctx, int op) {
//     switch(op) {
//         case 0:  on_load(...);
//         case 2:  on_unload(...);
//         case 3:  on_close(...);
//     }
//     if (failure) {
//         return -3;
//     }
//     return 0;
// }
namespace sled {
// Windows return <base_name>.dll
// linux   return lib<base_name>.so
// macos   return lib<base_name>.dylib
std::string DynamicLibraryName(sled::string_view base_name);

class HotReloader {
public:
    using VersionType = unsigned int;

    // cr_mode defines how much we validate global state transfer between
    // instances. The default is CR_UNSAFE, you can choose another mode by
    // defining CR_HOST, ie.: #define CR_HOST CR_SAFEST

    enum class LoadMode {
        kSafest = 0, // validate address and size of the state section,
                     // if anything changes the load will rollback
        kSafe = 1,   // validate only the size of the state section,
                     // if it changes the load will rollback
        kUnsafe = 2, // don't validate anything but that the size of section fits
                     // may not be identical though
        kDisable = 3,// disable the auto state transfer
    };

    enum class Operation {
        kLoad   = 0,
        kStep   = 1,
        kUnload = 2,
        kClose  = 3,
    };

    enum class Failure {
        kOk                = 0,
        kSegmentationFault = 1,// SIGSEGV
        kIllegal           = 2,// SIGILL
        kAbort             = 3,// SIGBRT
        kMisalign          = 4,// SIGBUS
        kBounds            = 5,// ARRAY BOUNDS EXEEDED
        kStackOverflow     = 6,
        kStateInvalidated  = 7,
        kBadImage          = 8,// invalid  binary
        kInitialFailure    = 9,
        kOther             = 10,
        kUser              = 0x100,
    };

    HotReloader(sled::string_view bin_path);
    ~HotReloader();

    bool Initialize();
    /**
     * @brief 如果 Update失败，再次Update将会回滚
     * @force_reload: 如果当前库已经加载，是否reload当前库
     * @ status: 如果不为nullptr，返回当前加载状态
    **/
    sled::optional<Failure> UpdateOrError(bool force_reload = true);
    // bool Rollback();

    void set_userdata(void *data);
    sled::string_view name() const;
    VersionType version() const;
    VersionType last_working_version() const;
    VersionType next_version() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}// namespace sled

#endif// SLED_SYSTEM_HOT_LOADER_H

#include "sled/system/hot_reloader.h"
#include "sled/log/log.h"

#define CR_HOST CR_SAFEST
#define CR_MAIN_FUNC "sled_hot_loader_main"
#include "sled/system/detail/cr.h"

namespace sled {
namespace detail {
HotReloader::Failure
TranslateFailure(cr_failure failure)
{
    switch (failure) {
    case CR_NONE:
        return HotReloader::Failure::kOk;
    case CR_SEGFAULT:
        return HotReloader::Failure::kSegmentationFault;
    case CR_ILLEGAL:
        return HotReloader::Failure::kIllegal;
    case CR_ABORT:
        return HotReloader::Failure::kAbort;
    case CR_MISALIGN:
        return HotReloader::Failure::kMisalign;
    case CR_BOUNDS:
        return HotReloader::Failure::kBounds;
    case CR_STACKOVERFLOW:
        return HotReloader::Failure::kStackOverflow;
    case CR_STATE_INVALIDATED:
        return HotReloader::Failure::kStateInvalidated;
    case CR_BAD_IMAGE:
        return HotReloader::Failure::kBadImage;
    case CR_INITIAL_FAILURE:
        return HotReloader::Failure::kInitialFailure;
    case CR_OTHER:
        return HotReloader::Failure::kOther;
    default:
        return HotReloader::Failure::kOther;
    }
}
}// namespace detail

std::string
DynamicLibraryName(sled::string_view base_name)
{
#if defined(_WIN32)
    return sled::to_string(base_name) + ".dll";
#elif defined(__APPLE__)
    return "lib" + sled::to_string(base_name) + ".dylib";
#elif defined(__linux__)
    return "lib" + sled::to_string(base_name) + ".so";
#else
#error "Unsupported platform"
#endif
}

class HotReloader::Impl {
public:
    Impl(sled::string_view bin_path) : bin_path_(sled::to_string(bin_path)) {}

    ~Impl() { cr_plugin_close(ctx_); }

    bool Initialize()
    {
        if (cr_plugin_open(ctx_, bin_path_.c_str())) { return true; }
        LOGV("HotReloader", "Failed to load plugin: {}", bin_path_);
        return false;
    }

    sled::optional<Failure> Update(bool force_reload)
    {
        if (cr_plugin_update(ctx_, force_reload) == 0) { return sled::nullopt; }
        return detail::TranslateFailure(ctx_.failure);
    }

    void set_userdata(void *data) { ctx_.userdata = data; }

    sled::string_view name() const { return bin_path_; }

    VersionType version() const { return ctx_.version; }

    VersionType last_working_version() const { return ctx_.last_working_version; }

    VersionType next_version() const { return ctx_.next_version; }

private:
    std::string bin_path_;
    cr_plugin ctx_;
};

HotReloader::HotReloader(sled::string_view bin_path) : impl_(new Impl(bin_path)) {}

HotReloader::~HotReloader() {}

bool
HotReloader::Initialize()
{
    return impl_->Initialize();
}

sled::optional<HotReloader::Failure>
HotReloader::UpdateOrError(bool force_reload)
{
    return impl_->Update(force_reload);
}

void
HotReloader::set_userdata(void *data)
{
    impl_->set_userdata(data);
}

sled::string_view
HotReloader::name() const
{
    return impl_->name();
}

HotReloader::VersionType
HotReloader::version() const
{
    return impl_->version();
}

HotReloader::VersionType
HotReloader::last_working_version() const
{
    return impl_->last_working_version();
}

HotReloader::VersionType
HotReloader::next_version() const
{
    return impl_->next_version();
}

}// namespace sled

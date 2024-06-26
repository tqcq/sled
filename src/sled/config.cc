#include "sled/config.h"
#include "sled/log/log.h"
#include "sled/strings/utils.h"
#include <fstream>

namespace sled {
Config::Config() = default;

Config::Config(sled::string_view name) : Config(name, "") {}

Config::Config(sled::string_view name, sled::string_view path) : config_name_(name)
{
    config_paths_.emplace_back(sled::to_string(path));
}

std::string
Config::ToString() const
{
    toml::value root = toml_;
    // format(const basic_value<C, M, V>& v, std::size_t w = 80u,
    //        int fprec = std::numeric_limits<toml::floating>::max_digits10,
    //        bool no_comment = false, bool force_inline = false)
    return toml::format(root, 180u);
}

void
Config::AddConfigFullPath(sled::string_view path)
{
    config_full_paths_.emplace_back(sled::to_string(path));
}

void
Config::SetConfigName(sled::string_view name)
{
    config_name_ = sled::to_string(name);
}

void
Config::AddConfigPath(sled::string_view path)
{
    config_paths_.emplace_back(sled::to_string(path));
}

bool
Config::ReadInConfig()
{
    const static std::vector<std::string> extensions = {".toml"};
    auto load_config_from                            = [](sled::string_view full_path, toml::value &value) {
        const std::ifstream file(to_string(full_path));
        if (file.good()) {
            try {
                std::stringstream ss;
                ss << file.rdbuf();

                std::istringstream stream_data(ss.str(), std::ios_base::binary | std::ios_base::in);
                value = toml::parse(stream_data, "string");
                return true;
                // goto config_read_success;
            } catch (const std::exception &e) {
                LOGE("sled::Config",
                     "Failed to parse config file: {}, reason: {}",
                     sled::to_string(full_path),
                     e.what());
            } catch (...) {
                LOGD("sled::Config", "Failed to parse config file: {}", sled::to_string(full_path));
            }
        }
        return false;
    };

    for (const auto &full_path : config_full_paths_) {
        if (load_config_from(full_path, toml_)) { return true; }
    }

    for (const auto &path : config_paths_) {
        auto name = path + "/" + config_name_;
        for (const auto &ext : extensions) {
            auto full_path = name + ext;
            if (load_config_from(full_path, toml_)) { return true; }
        }
    }

    return false;
}

bool
Config::IsSet(sled::string_view key) const
{
    toml::value value;
    return GetNode(key, value);
}

bool
Config::IsArray(sled::string_view key) const
{
    toml::value value;
    if (!GetNode(key, value)) { return false; }
    return value.is_array();
}

bool
Config::GetBoolOr(sled::string_view key, const bool &def) const
{
    toml::value value;
    try {
        if (GetNode(key, value) && value.is_boolean()) { return value.as_boolean(); }
    } catch (...) {}
    return def;
}

int
Config::GetIntOr(sled::string_view key, const int &def) const
{
    toml::value value;
    try {
        if (GetNode(key, value)) {
            if (value.is_integer()) {
                return value.as_integer();
            } else if (value.is_boolean()) {
                return value.as_boolean() ? 1 : 0;
            } else if (value.is_floating()) {
                return static_cast<int>(value.as_floating());
            }
        }
    } catch (...) {}
    return def;
}

double
Config::GetDoubleOr(sled::string_view key, const double &def) const
{
    toml::value value;
    try {
        if (GetNode(key, value)) {
            if (value.is_floating()) {
                return value.as_floating();
            } else if (value.is_integer()) {
                return static_cast<double>(value.as_integer());
            }
        }
    } catch (...) {}
    return def;
}

std::string
Config::GetStringOr(sled::string_view key, sled::string_view def) const
{
    toml::value value;
    try {
        if (GetNode(key, value) && value.is_string()) { return value.as_string(); }
    } catch (...) {}
    return sled::to_string(def);
}

void
Config::SetDefault(sled::string_view key, const bool &value)
{
    default_values_.insert({sled::to_string(key), value});
}

void
Config::SetDefault(sled::string_view key, const char *value)
{
    SetDefault(key, std::string(value));
}

void
Config::SetDefault(sled::string_view key, const std::string &value)
{
    default_values_.insert({sled::to_string(key), value});
}

void
Config::SetDefault(sled::string_view key, sled::string_view value)
{
    SetDefault(key, std::string(value));
}

void
Config::SetDefault(sled::string_view key, const int &value)
{
    default_values_.insert({sled::to_string(key), value});
}

void
Config::SetDefault(sled::string_view key, const double &value)
{
    default_values_.insert({sled::to_string(key), value});
}

void
Config::SetValue(sled::string_view key, const bool &value)
{
    values_.insert({sled::to_string(key), value});
}

void
Config::SetValue(sled::string_view key, const char *value)
{
    SetValue(key, std::string(value));
}

void
Config::SetValue(sled::string_view key, const std::string &value)
{

    values_.insert({sled::to_string(key), value});
}

void
Config::SetValue(sled::string_view key, sled::string_view value)
{
    SetValue(key, std::string(value));
}

void
Config::SetValue(sled::string_view key, const int &value)
{
    values_.insert({sled::to_string(key), value});
}

void
Config::SetValue(sled::string_view key, const double &value)
{
    values_.insert({sled::to_string(key), value});
}

bool
Config::GetNode(sled::string_view key, toml::value &value) const
{
    // 1.优先使用用户设定的值
    if (GetValueNode(key, value)) { return true; }

    // 2. 然后从配置文件获取
    auto keys = StrSplit(sled::to_string(key), ".");
    auto cur  = toml_;
    for (const auto &k : keys) {
        try {
            auto next = toml::find(cur, k);
            cur       = next;
        } catch (...) {
            // 3. 最后使用默认值
            if (GetDefaultNode(key, value)) { return true; }
            return false;
        }
    }
    value = cur;
    return true;
}

bool
Config::AddDefaultNode(sled::string_view key, ValueType value)
{
    auto keys = StrSplit(sled::to_string(key), ".");
    if (keys.size() == 1) {
        auto first_key = keys[0];
        switch (value.index()) {
        case 0:
            toml_[first_key] = sled::get<bool>(value);
            break;
        case 1:
            toml_[first_key] = sled::get<std::string>(value);
            break;
        case 2:
            toml_[first_key] = sled::get<int>(value);
            break;
        case 3:
            toml_[first_key] = sled::get<double>(value);
            break;
        default:
            return false;
        }
        return true;
    }
    return false;
}

bool
Config::GetValueNode(sled::string_view key, toml::value &value) const
{

    auto iter = values_.find(sled::to_string(key));
    if (iter == values_.end()) { return false; }
    auto &default_value = iter->second;
    switch (default_value.index()) {
    case 0:
        value = sled::get<bool>(default_value);
        break;
    case 1:
        value = sled::get<std::string>(default_value);
        break;
    case 2:
        value = sled::get<int>(default_value);
        break;
    case 3:
        value = sled::get<double>(default_value);
        break;
    default:
        return false;
    }
    return true;
}

bool
Config::GetDefaultNode(sled::string_view key, toml::value &value) const
{
    auto iter = default_values_.find(sled::to_string(key));
    if (iter == default_values_.end()) { return false; }
    auto &default_value = iter->second;
    switch (default_value.index()) {
    case 0:
        value = sled::get<bool>(default_value);
        break;
    case 1:
        value = sled::get<std::string>(default_value);
        break;
    case 2:
        value = sled::get<int>(default_value);
        break;
    case 3:
        value = sled::get<double>(default_value);
        break;
    default:
        return false;
    }
    return true;
}
}// namespace sled

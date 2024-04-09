#ifndef SLED_CONFIG_H
#define SLED_CONFIG_H

#include "sled/nonstd/string_view.h"
#include "sled/variant.h"
#include "toml.hpp"

namespace sled {
class Config {
public:
    using ValueType = sled::variant<bool, std::string, int, double>;
    Config();
    Config(sled::string_view name);
    Config(sled::string_view name, sled::string_view path);
    Config(const Config &lhs)                = delete;
    Config(Config &&rhs) noexcept            = delete;
    Config &operator=(const Config &lhs)     = delete;
    Config &operator=(Config &&rhs) noexcept = delete;

    void SetConfigName(sled::string_view name);
    void AddConfigPath(sled::string_view path);

    bool ReadInConfig();

    bool IsSet(sled::string_view key) const;
    bool GetBoolOr(sled::string_view key, const bool &def = false) const;
    int GetIntOr(sled::string_view key, const int &def = 0) const;
    double GetDoubleOr(sled::string_view key, const double &def = 0.0) const;
    std::string GetStringOr(sled::string_view key, const sled::string_view def = "") const;

    void SetDefault(sled::string_view key, const bool &value);
    void SetDefault(sled::string_view key, const char *value);
    void SetDefault(sled::string_view key, const std::string &value);
    void SetDefault(sled::string_view key, sled::string_view value);
    void SetDefault(sled::string_view key, const int &value);
    void SetDefault(sled::string_view key, const double &value);

private:
    bool GetNode(sled::string_view key, toml::value &value) const;
    bool AddDefaultNode(sled::string_view, ValueType value);
    bool GetDefaultNode(sled::string_view key, toml::value &value) const;

    std::unordered_map<std::string, ValueType> default_values_;
    std::vector<std::string> config_paths_;
    std::string config_name_;
    toml::value toml_;
};
}// namespace sled
#endif// SLED_CONFIG_H

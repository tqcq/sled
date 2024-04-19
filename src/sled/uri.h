#pragma once
#ifndef SLED_URI_H
#define SLED_URI_H
#include "sled/status_or.h"
#include <map>
#include <string>

namespace sled {
namespace internal {
#define __SLED_URI_GETTER(type, name)                                                                                  \
    type &name() & { return name##_; }                                                                                 \
    type &&name() && { return std::move(name##_); }                                                                    \
    type const &name() const & { return name##_; }

#define __SLED_URI_SETTER(type, name)                                                                                  \
    void set_##name(type const &v) { name##_ = v; }                                                                    \
    void set_##name(type &&v) { name##_ = std::move(v); }

#define __SLED_URI_GETTER_AND_SETTER(type, name) __SLED_URI_GETTER(type, name) __SLED_URI_SETTER(type, name)

}// namespace internal

class URI {
public:
    // using ParamType = std::pair<std::string, std::string>;
    using ParamMap = std::map<std::string, std::string>;
    // http://xxx.com/index.html?field=value
    // static URI ParseAbsoluteURI(const std::string &uri_str);

    // http://xxx.com/index.html?field=value#download
    static sled::StatusOr<URI> ParseURI(const std::string &uri_str);

    // http://xxx.com/index.html
    // static URI ParseURIReference(const std::string &uri_str);

    URI() = default;
    SLED_DEPRECATED URI(const std::string &uri_str);

    // setter getter
    __SLED_URI_GETTER_AND_SETTER(std::string, scheme)
    __SLED_URI_GETTER_AND_SETTER(std::string, content)
    __SLED_URI_GETTER_AND_SETTER(std::string, username)
    __SLED_URI_GETTER_AND_SETTER(std::string, password)
    __SLED_URI_GETTER_AND_SETTER(std::string, host)
    __SLED_URI_GETTER_AND_SETTER(unsigned long, port)
    __SLED_URI_GETTER_AND_SETTER(std::string, path)
    // __SLED_URI_GETTER_AND_SETTER(std::string, query)
    __SLED_URI_GETTER(std::string, query)
    void set_query(std::string const &v);
    void set_query(std::string &&v);
    __SLED_URI_GETTER_AND_SETTER(std::string, anchor)

    __SLED_URI_GETTER_AND_SETTER(ParamMap, query_param)

    std::string href() const;
    std::string authority() const;
    std::string user_info() const;

private:
    std::string scheme_;
    std::string content_;
    std::string username_;
    std::string password_;
    std::string host_;
    unsigned long port_ = 0;
    std::string path_;
    std::string query_;
    std::string anchor_;
    ParamMap query_param_;

    bool has_authority_ = false;
};

#undef __SLED_URI_GETTER_AND_SETTER
#undef __SLED_URI_GETTER
#undef __SLED_URI_SETTER

}// namespace sled

#endif//  SLED_URI_H

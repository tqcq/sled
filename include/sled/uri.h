#pragma once
#include <map>
#ifndef SLED_URI_H
#define SLED_URI_H
#include <string>

namespace sled {
namespace internal {
#define __SLED_URI_GETTER_AND_SETTER(type, name)                                                                       \
    type &name() & { return name##_; }                                                                                 \
    type &&name() && { return std::move(name##_); }                                                                    \
    type const &name() const & { return name##_; }                                                                     \
    void set_##name(type const &v) { name##_ = v; }                                                                    \
    void set_##name(type &&v) { name##_ = std::move(v); }

}// namespace internal

class URI {
public:
    // using ParamType = std::pair<std::string, std::string>;
    using ParamMap = std::map<std::string, std::string>;
    // http://xxx.com/index.html?field=value
    // static URI ParseAbsoluteURI(const std::string &uri_str);

    // http://xxx.com/index.html?field=value#download
    static URI ParseURI(const std::string &uri_str);

    // http://xxx.com/index.html
    // static URI ParseURIReference(const std::string &uri_str);

    URI() = default;
    URI(const std::string &uri_str);

    // setter getter
    __SLED_URI_GETTER_AND_SETTER(std::string, scheme)
    __SLED_URI_GETTER_AND_SETTER(std::string, content)
    __SLED_URI_GETTER_AND_SETTER(std::string, username)
    __SLED_URI_GETTER_AND_SETTER(std::string, password)
    __SLED_URI_GETTER_AND_SETTER(std::string, host)
    __SLED_URI_GETTER_AND_SETTER(unsigned long, port)
    __SLED_URI_GETTER_AND_SETTER(std::string, path)
    __SLED_URI_GETTER_AND_SETTER(ParamMap, query)
    __SLED_URI_GETTER_AND_SETTER(std::string, anchor)

private:
    std::string scheme_;
    std::string content_;
    std::string username_;
    std::string password_;
    std::string host_;
    unsigned long port_;
    std::string path_;
    ParamMap query_;
    std::string anchor_;
};
}// namespace sled

#endif//  SLED_URI_H

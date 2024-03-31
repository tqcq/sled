#include "sled/uri.h"
#include "sled/log/log.h"
#include "sled/strings/utils.h"
#include <cctype>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace sled {
sled::StatusOr<URI>
URI::ParseURI(const std::string &uri_str)
{
    static const std::regex uri_regex(
        R"((([a-zA-Z][a-zA-Z0-9+.-]*):)?)"// scheme:
        R"(([^?#]*))"                     // authority and path
        R"((?:\?([^#]*))?)"               // ?query
        R"((?:#(.*))?)",
        std::regex::ECMAScript);
    static const std::regex authority_and_path_regex(R"(//([^/]*)(/.*)?)", std::regex::ECMAScript);
    static const std::regex authority_only_regex(R"(([^/?#]*))", std::regex::ECMAScript);
    static const std::regex authority_regex(
        R"((?:([^@:]*)(?::([^@]*))?@)?)"// user:pass@ or user:@ or :pass@
        R"((\[[^\]]*\]|[^\[:]*))"       // [::1] or 127.0.0.1 or domain
        R"((?::([0-9]*))?)"             // port
        ,
        std::regex::ECMAScript);

    URI uri;
    std::smatch match;
    if (!std::regex_match(uri_str, match, uri_regex)) {
        return sled::MakeStatusOr<URI>(sled::StatusCode::kInvalidArgument, "Invalid URI format");
    }
    uri.set_scheme(sled::ToLower(match[2]));

    int counter = 0;
    // for (auto res : match) { LOGD("match", "{}:{}", counter++, res); }

    std::smatch authority_and_path_match;
    std::string authority_and_path = match[3].str();
    if (!std::regex_match(authority_and_path, authority_and_path_match, authority_and_path_regex)) {
        /*
        std::smatch authority_only_match;
        if (std::regex_match(authority_and_path, authority_only_regex)
            && std::regex_match(authority_and_path, authority_only_match, authority_regex)) {
            // not find //
            uri.has_authority_ = false;
            // bob@example.com
            // example.com
            if (!authority_only_match[4].str().empty()) { uri.set_port(std::stoi(authority_only_match[4])); }
            uri.set_username(authority_only_match[1]);
            uri.set_password(authority_only_match[2]);
            uri.set_host(authority_only_match[3]);
            return uri;
        } else {
        */
        uri.set_path(match[3]);
        // }
    } else {
        // counter = 0;
        // for (auto res : authority_and_path_match) { LOGD("", "{}:{}", counter++, res); }
        std::string authority = authority_and_path_match[1];
        std::smatch authority_match;
        if (!std::regex_match(authority, authority_match, authority_regex)) {
            return sled::MakeStatusOr<URI>(sled::StatusCode::kInvalidArgument, "Invalid URI Authority");
        }
        // has //
        uri.has_authority_ = true;
        // counter = 0;
        // for (auto res : authority_match) { LOGD("", "{}:{}", counter++, res); }
        std::string path = authority_and_path_match[2];

        if (!authority_match[4].str().empty()) {
            try {
                uri.set_port(std::stoi(authority_match[4]));
            } catch (const std::out_of_range &e) {
                return sled::MakeStatusOr<URI>(sled::StatusCode::kInvalidArgument, "Invalid URI Port");
            }
        }
        uri.set_username(authority_match[1]);
        uri.set_password(authority_match[2]);
        uri.set_host(authority_match[3]);
        uri.set_path(authority_and_path_match[2]);
    }

    // LOGD("query", "{}", match[4]);
    uri.set_query(match[4]);
    uri.set_anchor(match[5]);

    return std::move(uri);
}

URI::URI(const std::string &uri_str)
{
    auto uri_or = ParseURI(uri_str);
    ASSERT(uri_or.ok(), "{}", uri_or.status());
    *this = uri_or.value();
}

std::map<std::string, std::string>
ParseQueryMap(const std::string &query)
{

    std::map<std::string, std::string> query_param_;
    auto item = sled::StrSplit(query, "&", /*ignore_empty=*/true);
    for (auto &i : item) {
        if (i[0] == '=') { continue; }

        auto kv = sled::StrSplit(i, "=", /*ignore_empty=*/true);
        if (kv.size() == 2) {
            query_param_[kv[0]] = kv[1];
        } else {
            query_param_[kv[0]] = "";
        }
    }
    return std::move(query_param_);
}

void
URI::set_query(std::string const &v)
{
    query_       = v;
    query_param_ = ParseQueryMap(query_);
}

void
URI::set_query(std::string &&v)
{
    query_       = std::move(v);
    query_param_ = ParseQueryMap(query_);
}

std::string
URI::href() const
{
    std::stringstream ss;
    if (!scheme().empty()) { ss << scheme() << ":"; }
    if (has_authority_) { ss << "//"; }
    if (!authority().empty()) { ss << authority(); }
    ss << path();
    if (!query().empty()) { ss << "?" << query(); }
    if (!anchor().empty()) { ss << "#" << anchor(); }
    return ss.str();
}

std::string
URI::authority() const
{
    std::stringstream ss;
    if (!username().empty()) { ss << username(); }
    if (!password().empty()) { ss << ":" << password(); }
    if (!username().empty() || !password().empty()) { ss << "@"; }
    ss << host();
    if (port() != 0) { ss << ":" << port(); }

    return ss.str();
}

std::string
URI::user_info() const
{
    if (password().empty()) { return username(); }
    if (username().empty()) { return ":" + password(); }
    return username() + ":" + password();
}

}// namespace sled

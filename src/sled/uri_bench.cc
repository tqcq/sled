#include <sled/log/log.h>
#include <sled/uri.h>

std::string
GenerateURI(int length)
{
    std::string uri = "http://host";
    // add host
    for (int i = 0; i < length; i++) { uri.append("host"); }
    for (int i = 0; i < length; i++) { uri.append("/path"); }
    if (length > 0) { uri.append("?"); }
    for (int i = 0; i < length; i++) {
        if (i) { uri.append("&"); }
        uri.append("key" + std::to_string(i) + "=value");
    }

    uri.append("#fragment");

    return uri;
}

void
ParseURI(picobench::state &s)
{
    // int length = 0;
    for (auto _ : s) {
        s.pause_timer();
        auto uri_str = GenerateURI(10);
        s.resume_timer();
        auto uri_or = sled::URI::ParseURI(uri_str);
        s.pause_timer();
        SLED_ASSERT(uri_or.ok() == true, "");
        SLED_ASSERT(uri_or.value().href() == uri_str, "{} != {}", uri_or.value().href(), uri_str);
    }
}

PICOBENCH(ParseURI);

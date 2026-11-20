#pragma once

#include "state.h"
#include <boost/url.hpp>
#include <boost/url/authority_view.hpp>
#include <string_view>

class HttpParser {
    std::string_view m_view;

    bool consume_char(const char c);

    bool consume_one_space();

  public:
    HttpParser(std::string_view view);

    HttpParserErrorCode parse_request_line(HttpState &state);
    HttpParserErrorCode parse_headers(HttpState &state);
};
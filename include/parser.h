#pragma once

#include "constants.h"
#include <boost/url.hpp>
#include <boost/url/authority_view.hpp>
#include <cstdint>
#include <string_view>

enum class HttpParserErrorCode : std::uint8_t {
    INVALID_METHOD,
    UNSUPPORTED_VERSION,
    BAD_REQUEST,
    CONTENT_TOO_LARGE,
    NOT_IMPLEMENTED,
    OK,
};

enum class RequestTargetForm : std::uint8_t {
    ORIGIN_FORM,
    ABSOLUTE_FORM,
    AUTHORITY_FORM,
    ASTERISK_FORM,
    NONE,
};

enum class ConnectionStatus : std::uint8_t {
    KEEP_ALIVE,
    CLOSE,
    NONE,
};

class HttpState {
  public:
    boost::urls::url_view uri;
    boost::urls::authority_view auth_view;
    std::string_view host;
    int content_length;
    ConnectionStatus conn_status;
    HttpMethod method;
    RequestTargetForm rt_form;
    HttpVersion version;

    HttpState();
};

class HttpParser {
    std::string_view m_view;

    bool consume_char(const char c);

    bool consume_one_space();

  public:
    HttpParser(std::string_view view);

    HttpParserErrorCode parse_request_line(HttpState &state);
    HttpParserErrorCode parse_headers(HttpState &state);
};
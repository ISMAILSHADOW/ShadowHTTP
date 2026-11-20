#pragma once

#include "constants.h"
#include <boost/url/url_view.hpp>
#include <cstdint>

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

    inline HttpState()
        : host(), content_length(-1), conn_status(ConnectionStatus::NONE), method(HttpMethod::NONE),
          rt_form(RequestTargetForm::NONE), version(HttpVersion::NONE) {}
};
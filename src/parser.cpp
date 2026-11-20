#include "parser.h"
#include "constants.h"
#include "log.h"
#include <boost/url.hpp>
#include <boost/url/grammar/alnum_chars.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/detail/charset.hpp>
#include <boost/url/grammar/lut_chars.hpp>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstring>
#include <string_view>

using namespace boost::urls;

constexpr grammar::lut_chars header_key_charset = grammar::lut_chars(grammar::alnum_chars) + "!#$%&'*+-.^_`|~";

constexpr grammar::lut_chars header_value_charset([](char ch) noexcept {
    return static_cast<unsigned char>(ch) <= 127 && ch != '\0' && ch != '\r' && ch != '\n';
});

HttpParser::HttpParser(std::string_view view) : m_view(view) {}

static bool handle_authority_form(HttpState &state, const std::string_view request_target) {
    state.rt_form = RequestTargetForm::AUTHORITY_FORM;
    auto r = parse_authority(request_target);
    if (!r) {
        LOG_DEBUG("Failed to parse authority form of request line.");
        return false;
    }
    state.auth_view = *r;
    if (state.auth_view.has_userinfo() || state.auth_view.port().empty()) return false;
    return true;
}

static bool handle_origin_form(HttpState &state, const std::string_view request_target) {
    state.rt_form = RequestTargetForm::ORIGIN_FORM;
    auto r = parse_origin_form(request_target);
    if (!r) {
        LOG_DEBUG("Failed to parse origin form of request line.");
        return false;
    }
    state.uri = *r;
    return true;
}

static bool handle_asterisk_form(HttpState &state) {
    state.rt_form = RequestTargetForm::ASTERISK_FORM;

    if (state.method != HttpMethod::OPTIONS) return false;
    return true;
}

static bool handle_absolute_form(HttpState &state, const std::string_view request_target) {
    state.rt_form = RequestTargetForm::ABSOLUTE_FORM;

    auto r = parse_absolute_uri(request_target);
    if (!r) {
        LOG_DEBUG("Failed to parse absolute form of request line.");
        return false;
    }
    state.uri = *r;
    if (state.uri.scheme_id() != scheme::http && state.uri.scheme_id() != scheme::https) return false;
    if (!state.uri.has_authority() || state.uri.encoded_host().empty()) return false;
    return true;
}

bool HttpParser::consume_char(const char c) {
    if (!m_view.empty() && m_view.front() == c) {
        m_view.remove_prefix(1);
        return true;
    }
    return false;
}

bool HttpParser::consume_one_space() { return consume_char(' '); }

HttpParserErrorCode HttpParser::parse_request_line(HttpState &state) {
    for (const MethodObject &m : HTTP_METHODS) {
        if (m_view.starts_with(m.name)) {
            state.method = m.method;
            m_view.remove_prefix(m.name.size());
            break;
        }
    }

    if (state.method == HttpMethod::NONE) return HttpParserErrorCode::INVALID_METHOD;

    if (!consume_one_space()) return HttpParserErrorCode::BAD_REQUEST;

    size_t pos = m_view.find_first_of(' ');
    if (pos == NO_POSITION || pos == 0) return HttpParserErrorCode::BAD_REQUEST;

    std::string_view request_target = m_view.substr(0, pos);
    if (request_target.size() > MAX_REQUEST_TARGET_SIZE) return HttpParserErrorCode::BAD_REQUEST;

    if (state.method == HttpMethod::CONNECT) {
        if (!handle_authority_form(state, request_target)) return HttpParserErrorCode::BAD_REQUEST;
    } else if (m_view.front() == '/') {
        if (!handle_origin_form(state, request_target)) return HttpParserErrorCode::BAD_REQUEST;
    } else if (request_target == "*") {
        if (!handle_asterisk_form(state)) return HttpParserErrorCode::BAD_REQUEST;
    } else {
        if (!handle_absolute_form(state, request_target)) return HttpParserErrorCode::BAD_REQUEST;
    }
    m_view.remove_prefix(pos);

    if (!consume_one_space()) return HttpParserErrorCode::BAD_REQUEST;

    if (m_view.starts_with("HTTP/1.1\r\n"))
        state.version = HttpVersion::HTTP_1_1;
    else if (m_view.starts_with("HTTP/1.0\r\n"))
        state.version = HttpVersion::HTTP_1_0;
    else
        return HttpParserErrorCode::BAD_REQUEST;

    m_view.remove_prefix(10); // "HTTP/1.x\r\n"

    return HttpParserErrorCode::OK;
}

HttpParserErrorCode HttpParser::parse_headers(HttpState &state) {
    size_t header_count = 0;

    while (m_view.size() > 2) {
        if (++header_count > MAX_HEADER_COUNT) return HttpParserErrorCode::BAD_REQUEST;

        size_t pos = m_view.find(':');
        if (pos == NO_POSITION || pos == 0) return HttpParserErrorCode::BAD_REQUEST;

        // We know \r\n exists since recv()ing from the socket
        // doesn't return until it finds the mark.
        size_t crlf_pos = m_view.find("\r\n");
        if (crlf_pos < pos) return HttpParserErrorCode::BAD_REQUEST;

        std::string_view key = m_view.substr(0, pos);
        std::string_view value = m_view.substr(pos + 1, crlf_pos - pos - 1);

        // For trimming spaces
        size_t val_start = value.find_first_not_of(" \t");
        if (val_start == NO_POSITION) {
            value = {};
        } else {
            size_t val_end = value.find_last_not_of(" \t");
            value = value.substr(val_start, val_end - val_start + 1);
        }

        if (header_key_charset.find_if_not(key.data(), key.data() + key.size()) != key.end()) {
            LOG_DEBUG("Invalid character used in header key: %s", key.data());
            return HttpParserErrorCode::BAD_REQUEST;
        }
        if (header_value_charset.find_if_not(value.data(), value.data() + value.size()) != value.end()) {
            LOG_DEBUG("Invalid character used in header value: %s", key.data());
            return HttpParserErrorCode::BAD_REQUEST;
        }

        if (grammar::ci_is_equal("content-length", key)) {
            auto r = grammar::parse(value, grammar::unsigned_rule<uint32_t>{});

            if (!r || *r > INT_MAX) return HttpParserErrorCode::BAD_REQUEST;
            if (*r > MAX_REQUEST_BODY_SIZE) return HttpParserErrorCode::CONTENT_TOO_LARGE;
            if (state.content_length != -1 && (int)*r != state.content_length) return HttpParserErrorCode::BAD_REQUEST;

            state.content_length = (int)*r;
        } else if (grammar::ci_is_equal("host", key)) {
            if (!state.host.empty()) return HttpParserErrorCode::BAD_REQUEST;
            state.host = value;
        } else if (state.conn_status == ConnectionStatus::NONE && grammar::ci_is_equal("connection", key)) {
            if (grammar::ci_is_equal("keep-alive", value)) {
                state.conn_status = ConnectionStatus::KEEP_ALIVE;
            } else {
                state.conn_status = ConnectionStatus::CLOSE;
            }
        } else if (grammar::ci_is_equal("transfer-encoding", key)) {
            return HttpParserErrorCode::NOT_IMPLEMENTED;
        }

        m_view.remove_prefix(crlf_pos + 2);
    }

    assert(m_view == "\r\n");
    m_view.remove_prefix(2);

    if (state.version == HttpVersion::HTTP_1_1 && state.host.empty()) {
        LOG_DEBUG("No host header was added.");
        return HttpParserErrorCode::BAD_REQUEST;
    }
    if (state.conn_status == ConnectionStatus::NONE) {
        state.conn_status =
            (state.version == HttpVersion::HTTP_1_1) ? ConnectionStatus::KEEP_ALIVE : ConnectionStatus::CLOSE;
    }

    return HttpParserErrorCode::OK;
}
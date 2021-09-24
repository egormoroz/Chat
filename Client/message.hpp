#pragma once

#include <cstdint>
#include <cstring>

const size_t MSG_BUFFER_SIZE = 1024;

enum class MsgKind {
    AUTH = 0,
    MSG,
    UIN,
    UOUT,
    ONLINE,
    N
};

struct Header {
    uint16_t len;
    uint8_t code;
};

const size_t HEADER_SIZE = sizeof(Header);
const size_t MAX_BODY_SIZE = MSG_BUFFER_SIZE - sizeof(Header);

class Message {
public:
    Message() {
        memset(m_data, 0, sizeof(m_data));
    }

    void set_body_len(uint16_t len) { m_header.len = len; }
    void set_kind(MsgKind k) { m_header.code = (uint8_t)k; }

    char *data() { return m_data; }
    const char *data() const { return m_data; }
    size_t length() const { return m_header.len + sizeof(Header); }

    char *body_data() { return &m_data[sizeof(Header)]; }
    const char *body_data() const { return &m_data[sizeof(Header)]; }
    size_t body_length() const { return m_header.len; }

    MsgKind kind() const { return (MsgKind)m_header.code; }

    bool sane_header() const {
        if (length() > MSG_BUFFER_SIZE
                || m_header.code >= (uint8_t)MsgKind::N)
            return false;
        return true;
    }

    bool decode_header() {
        memcpy(&m_header, m_data, sizeof(Header));
        if (sane_header())
            return true;

        memset(&m_header, 0, sizeof(Header));
        return false;
    }

    void encode_header() {
        memcpy(m_data, &m_header, sizeof(Header));
    }

    bool null_terminated() const {
        return body_length() > 0 && m_data[length() - 1] == 0;
    }

private:
    Header m_header;
    char m_data[MSG_BUFFER_SIZE];
};


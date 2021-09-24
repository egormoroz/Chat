#pragma once

#include <cassert>
#include "message.hpp"
#include <string>

const size_t MAX_USERNAME_SIZE = 32;

namespace parse {

class Auth {
public:
    Auth(const Message &src)
        : m_src(src) {}

    bool valid() const {
        size_t n = m_src.body_length();
        return n > 0 && n <= MAX_USERNAME_SIZE 
            && m_src.null_terminated();
    }
    
    const char *username() const {
        return m_src.body_data();
    }
private:
    const Message &m_src;
};

class Msg {
public:
    Msg(const Message &src)
        : m_src(src) {}

    bool valid() const {
        return m_src.null_terminated();
    }

    const char *text() const {
        return m_src.body_data();
    }

    size_t length() const {
        return m_src.body_length();
    }

private:
    const Message &m_src;
};

}

namespace build {
    static Message auth(const std::string &uname) {
        Message m;
        m.set_body_len(uname.length() + 1);
        m.set_kind(MsgKind::AUTH);
        m.encode_header();
        memcpy(m.body_data(), uname.c_str(), uname.length());
        return m;
    }

    static Message msg(const std::string &uname,
            const char *text, size_t tlen) 
    {
        Message m;
        m.set_body_len(uname.length() + tlen + 2);
        m.set_kind(MsgKind::MSG);
        m.encode_header();
        memcpy(m.body_data(), uname.c_str(), uname.length());
        memcpy(m.body_data() + uname.length() + 1, text, tlen);
        return m;
    }

    class UIOBuilder {
    public:
        UIOBuilder() = default;

        bool full() const {
            return (m_ucount + 1) * MAX_USERNAME_SIZE > MAX_BODY_SIZE;
        }

        void push_user(const std::string &name) {
            assert(name.length() < MAX_USERNAME_SIZE);
            assert(!full());
            memcpy(m_result.body_data() + m_ucount * MAX_USERNAME_SIZE, 
                name.c_str(), name.length());
             m_ucount++;
        }

        Message &get(MsgKind k) {
            m_result.set_body_len(m_ucount * MAX_USERNAME_SIZE);
            m_result.set_kind(k);
            m_result.encode_header();
            return m_result;
        }

        void reset() {
            m_result.clear();
            m_ucount = 0;
        }

    private:
        size_t m_ucount = 0;
        Message m_result;
    };
}


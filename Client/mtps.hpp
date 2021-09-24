#pragma once

#include "message.hpp"
#include <string>
#include <cassert>

const size_t MAX_USERNAME_SIZE = 32;
const size_t MAX_MSG_SIZE = MAX_BODY_SIZE - MAX_USERNAME_SIZE;

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

        bool empty() const {
            return m_src.body_length() < 2;
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
            : m_src(src), m_text(nullptr)
        {
            if (m_src.null_terminated() && m_src.body_length() > 4) {
                const char *end = m_src.body_data() + m_src.body_length() - 1;
                for (m_text = m_src.body_data(); *m_text; ++m_text);
                if (m_text != end) {
                    ++m_text;
                } else {
                    m_text = nullptr;
                }
            }
        }
        
        bool valid() const {
            return m_text != nullptr;
        }

        const char *sender() const {
            return m_src.body_data();
        }

        const char *text() const {
            return m_text;
        }

    private:
        const Message &m_src;
        const char *m_text;
    };

    class UserStatus {
    public:
        UserStatus(const Message &src) 
            : m_src(src) {}

        bool valid() const {
            if ((m_src.body_length() % MAX_USERNAME_SIZE) != 0
                    || m_src.body_length() == 0)
                return false;
            size_t n = m_src.body_length() / MAX_USERNAME_SIZE;
            auto data = m_src.body_data();
            for (size_t i = 0; i < n; ++i) {
                if (data[MAX_USERNAME_SIZE * (i + 1) - 1] != 0)
                    return false;
            }
            return true;
        }

        template<typename P>
        void for_each_user(P p) const {
            size_t n = m_src.body_length() / MAX_USERNAME_SIZE;
            auto data = m_src.body_data();
            for (size_t i = 0; i < n; ++i) {
                p(&data[MAX_USERNAME_SIZE * i]);
            }
        }

    private:
        const Message &m_src;
    };
}

namespace build {
    static Message auth(const std::string &uname) {
        assert(uname.length() < MAX_USERNAME_SIZE);
        Message m;
        m.set_body_len(uname.length() + 1);
        m.set_kind(MsgKind::AUTH);
        m.encode_header();
        memcpy(m.body_data(), uname.c_str(), uname.length());
        return m;
    }

    static Message msg(const char *text, size_t len) {
        assert(len < MAX_BODY_SIZE);
        Message m;
        m.set_body_len(len + 1);
        m.set_kind(MsgKind::MSG);
        m.encode_header();
        memcpy(m.body_data(), text, len);
        return m;
    }
}

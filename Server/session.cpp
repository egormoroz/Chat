#include "session.hpp"
#include "room.hpp"
#include "mtps.hpp"

using namespace boost;

Session::Session(tcp::socket socket, Room &room)
        : m_socket(std::move(socket)), m_room(room) {}

void Session::start() {
    do_read_header();
}

void Session::deliver(const Message &m) {
    bool write_in_progress = !m_write_msgs.empty();
    m_write_msgs.push_back(m);
    if (!write_in_progress) {
        do_write();
    }
}

const std::string &Session::name() const {
    return m_name;
}

void Session::do_read_header() {
    auto self = shared_from_this();
    asio::async_read(m_socket,
        asio::buffer(m_read_msg.data(), HEADER_SIZE),
        [this, self](system::error_code ec, size_t)
        {
            if (!ec && m_read_msg.decode_header()) {
                do_read_body();
            } else if (!m_name.empty()) {
                m_room.leave(shared_from_this());
            }
        });
}

void Session::do_read_body() {
    auto self = shared_from_this();
    asio::async_read(m_socket,
        asio::buffer(m_read_msg.body_data(), m_read_msg.body_length()),
        [this, self](system::error_code ec, size_t)
        {
            if (!ec) {
                handle_message();
                do_read_header();
            } else if (!m_name.empty()) {
                m_room.leave(shared_from_this());
            }
        });
}

void Session::do_write() {
    auto self = shared_from_this();
    asio::async_write(m_socket,
            asio::buffer(m_write_msgs.front().data(), 
                m_write_msgs.front().length()),
            [this, self] (system::error_code ec, size_t)
            {
                if (!ec) {
                    m_write_msgs.pop_front();
                    if (!m_write_msgs.empty()) {
                        do_write();
                    }
                } else if (!m_name.empty()) {
                    m_room.leave(shared_from_this());
                }
            });
}

void Session::handle_message() {
    if (m_name.empty()) {
        if (m_read_msg.kind() != MsgKind::AUTH)
            return;

        parse::Auth a(m_read_msg);
        if (!a.valid()) return;
        std::string name = a.username();
        if (m_room.available_name(name))
            m_name = std::move(name);

        deliver(build::auth(m_name));
        if (!m_name.empty()) {
            m_room.join(shared_from_this());
        }
    } else if (m_read_msg.kind() == MsgKind::MSG) {
        parse::Msg m(m_read_msg);
        if (!m.valid()) return;

        m_room.on_new_msg(m_name, m.text(), m.length());
    }
}



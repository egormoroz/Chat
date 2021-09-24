#pragma once

#include <boost/asio.hpp>
#include <deque>
#include "logger.hpp"
#include "mtps.hpp"

using std::cout;
using std::endl;
using namespace boost;
using boost::asio::ip::tcp;

class Client {
public:
    Client(asio::io_context &io_context, 
        const tcp::resolver::results_type &eps)
        : m_socket(io_context), m_io_context(io_context)
    {
        do_connect(eps);
    }

    void write(const Message &msg) {
        m_io_context.post([this, msg]() {
            bool write_in_progress = !m_write_msgs.empty();
            m_write_msgs.push_back(msg);
            if (!write_in_progress) {
                do_write();
            }
        });
    }

    void close() {
        m_io_context.post([this]() {
            m_socket.close();
        });
    }
    
private:
    void do_connect(const tcp::resolver::results_type &eps) {
        asio::async_connect(m_socket, eps, 
            [this](system::error_code ec, tcp::endpoint) {
                if (!ec) {
                    do_read_header();
                } else {
                    logger() << "failed to connect: " << ec.message() << endl;
                }
            });
    }

    void do_read_header() {
        asio::async_read(m_socket, 
            asio::buffer(m_read_msg.data(), HEADER_SIZE),
            [this](system::error_code ec, size_t) {
                if (!ec) {
                    if (m_read_msg.decode_header()) {
                        do_read_body();
                    } else {
                        logger() << "failed to decode header" << endl;
                        do_read_header();
                    }
                } else {
                    logger() << "lost connection" << endl;
                    m_socket.close();
                }
            });
    }

    void do_read_body() {
        asio::async_read(m_socket,
            asio::buffer(m_read_msg.body_data(), m_read_msg.body_length()),
            [this](system::error_code ec, size_t) {
                if (!ec) {
                    handle_message();
                    do_read_header();
                } else {
                    logger() << "lost connection" << endl;
                    m_socket.close();
                }
            });
    }

    void do_write() {
        const Message &m = m_write_msgs.front();
        asio::async_write(m_socket,
            asio::buffer(m.data(), m.length()),
            [this](system::error_code ec, size_t) {
                if (!ec) {
                    m_write_msgs.pop_front();
                    if (!m_write_msgs.empty())
                        do_write();
                } else {
                    logger() << "lost connection" << endl;
                    m_socket.close();
                }
            });
    }

    void handle_message() {
        switch (m_read_msg.kind()) {
        case MsgKind::AUTH: 
        {
            parse::Auth a(m_read_msg);
            if (!a.valid()) {
                logger() << "received invalid auth message" << endl;
                return;
            }
            if (a.empty()) {
                logger() << "failed to authorize (username is already in use)" << endl;
            } else {
                logger() << "authorized as " << a.username() << endl;
            }

            break;
        }
        case MsgKind::MSG:
        {
            parse::Msg m(m_read_msg);
            if (!m.valid()) {
                logger() << "received invalid chat message" << endl;
                return;
            }
               
            cout << m.sender() << ": " << m.text() << endl;
            break;
        }
        case MsgKind::UIN:
        case MsgKind::UOUT:
        {
            parse::UsersInOut uio(m_read_msg);
            if (!uio.valid()) {
                logger() << "received invalid users in/out message" << endl;
                return;
            }
            const char *status = m_read_msg.kind() == MsgKind::UIN ? 
                "online" : "offline";
            uio.for_each_user([status](const char *uname) {
                cout << uname << " is " << status << endl;
            });
            break;
        }
        default:
            logger() << "received invalid message" << endl;
            break;
        }
    }

    asio::io_context &m_io_context;
    tcp::socket m_socket;
    std::deque<Message> m_write_msgs;
    Message m_read_msg;
};

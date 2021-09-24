#pragma once

#include <boost/asio.hpp>
#include "room.hpp"
#include "session.hpp"

using namespace boost;
using asio::ip::tcp;

class Server {
public:
    Server(asio::io_context &io_context,
        const tcp::endpoint &ep) : m_acceptor(io_context, ep)
    {
        do_accept();
    }

private:
    void do_accept() {
        m_acceptor.async_accept(
            [this](system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), m_room)->start();
                }
                do_accept();
            });
    }

    tcp::acceptor m_acceptor;
    Room m_room;
};

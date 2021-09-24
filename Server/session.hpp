#pragma once

#include <string>
#include <memory>
#include <deque>
#include <boost/asio.hpp>
#include "message.hpp"

using boost::asio::ip::tcp;

class Room;

class Session 
    : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Room &room);

    void start();
    void deliver(const Message &m);

    const std::string &name() const;

private:
    void do_read_header();
    void do_read_body();

    void do_write();

    void handle_message();

private:
    tcp::socket m_socket;
    Room &m_room;
    std::deque<Message> m_write_msgs;
    Message m_read_msg;
    std::string m_name;
};


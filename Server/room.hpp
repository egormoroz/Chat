#pragma once

#include "session.hpp"
#include <set>

using Participant = std::shared_ptr<Session>;

class Room {
public:
    Room() = default;

    void join(Participant p);
    void leave(Participant p);

    bool available_name(const std::string &name) const;

    void on_new_msg(const std::string &sender,
            const char *text, size_t len);

    void deliver(const Message &m);

private:
    std::set<Participant> m_participants;
    const size_t MAX_RECENT_MSGS = 100;
    std::deque<Message> m_recent_msgs;
};


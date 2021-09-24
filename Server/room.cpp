#include "room.hpp"
#include "mtps.hpp"
#include <algorithm>
#include "logger.hpp"

void Room::join(Participant p) {
    build::UIOBuilder b;
    for (auto &i: m_participants) {
        if (b.full()) {
            p->deliver(b.get(MsgKind::UIN));
            b.reset();
        }
        b.push_user(i->name());
    }
    if (!m_participants.empty())
        p->deliver(b.get(MsgKind::UIN));
    for (auto &m: m_recent_msgs)
        p->deliver(m);

    b.reset();
    b.push_user(p->name());
    deliver(b.get(MsgKind::UIN));

    m_participants.insert(p);
    logger() << p->name() << " joined" << std::endl;
}

void Room::leave(Participant p) {
    m_participants.erase(p);
    build::UIOBuilder b;
    b.push_user(p->name());
    deliver(b.get(MsgKind::UOUT));
    logger() << p->name() << " left" << std::endl;
}

void Room::deliver(const Message &m) {
    m_recent_msgs.push_back(m);
    if (m_recent_msgs.size() > MAX_RECENT_MSGS)
        m_recent_msgs.pop_front();
    for (auto &p: m_participants)
        p->deliver(m);
}

bool Room::available_name(const std::string& name) const { 
    auto it = std::find_if(m_participants.begin(), 
            m_participants.end(), [&](const Participant &p) {
                return p->name() == name;
            });
    return it == m_participants.end();
}

void Room::on_new_msg(const std::string &sender,
            const char *text, size_t len) 
{
    logger() << sender << ": " << text << std::endl;
    deliver(build::msg(sender, text, len));
}


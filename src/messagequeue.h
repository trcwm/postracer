#pragma once

#include <deque>
#include <thread>
#include <mutex>

/** thread-safe message queue */
template<typename MessageType>
class MessageQueue
{
public:
    MessageQueue() = default;

    // remove copy and move constructor
    MessageQueue(const MessageQueue& queue) = delete;
    MessageQueue(MessageQueue&& queue) = delete;
    MessageQueue& operator=(const MessageQueue &) = delete;

    void push(MessageType &&msg) noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_messages.emplace_back(msg);
    }

    bool hasMessages() noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        return !m_messages.empty();
    }

    const MessageType& peek() noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_messages.front();
    }

    MessageType pop() noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        MessageType item = m_messages.front();
        m_messages.pop_front();
        return item;
    }

    void clear() noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_messages.clear();
    }

protected:
    std::mutex m_mutex;
    std::deque<MessageType> m_messages;
};

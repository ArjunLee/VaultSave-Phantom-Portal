#include "EventBus.hpp"

namespace vspp
{
    namespace core
    {

        EventBus &EventBus::get()
        {
            static EventBus instance;
            return instance;
        }

        void EventBus::subscribe(EventType type, EventHandler handler)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_handlers.emplace_back(type, handler);
        }

        void EventBus::publish(const Event &event)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto &[type, handler] : m_handlers)
            {
                if (type == event.type)
                {
                    handler(event);
                }
            }
        }

        void EventBus::reset()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_handlers.clear();
        }

    } // namespace core
} // namespace vspp

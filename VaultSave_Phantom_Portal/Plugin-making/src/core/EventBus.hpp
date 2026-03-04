#pragma once

#include <functional>
#include <vector>
#include <mutex>
#include <variant>
#include <string>

namespace vspp
{
    namespace core
    {

        enum class EventType
        {
            SavePosition,
            Teleport,
            SlotChanged,
            LanguageChanged
        };

        struct Event
        {
            EventType type;
            std::variant<int, std::string> data;
        };

        using EventHandler = std::function<void(const Event &)>;

        /**
         * @brief Central Event Bus for decoupling system communication.
         */
        class EventBus
        {
        public:
            /**
             * @brief Get the singleton instance.
             * @return EventBus& Reference to the singleton instance.
             */
            static EventBus &get();

            /**
             * @brief Subscribe to an event type.
             * @param type The event type to listen for.
             * @param handler The callback function to execute when the event is published.
             */
            void subscribe(EventType type, EventHandler handler);

            /**
             * @brief Publish an event to all subscribers.
             * @param event The event object containing type and data.
             */
            void publish(const Event &event);

            /**
             * @brief Reset the EventBus, clearing all subscribers.
             * Used primarily for testing cleanup.
             */
            void reset();

        private:
            EventBus() = default;
            ~EventBus() = default;

            std::vector<std::pair<EventType, EventHandler>> m_handlers;
            std::mutex m_mutex;
        };

    } // namespace core
} // namespace vspp

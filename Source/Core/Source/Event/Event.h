#pragma once

#include <functional>
#include <unordered_set>

namespace DX
{
    // -------------------------------------------------------
    // Event usage example: Class Foo signals a resize event to everybody connected.
    // 
    // using ResizeEvent = Event<std::function<void(int width, int height)>>;
    // 
    // class Foo
    // {
    // public:
    //     void RegisterResizeEvent(ResizeEvent::Handler& handler) { handler.Connect(m_resizeEvent); }
    //     void UnregisterResizeEvent(ResizeEvent::Handler& handler) { handler.Disconnect(m_resizeEvent); }
    // 
    // private:
    //     void DoSomething()
    //     {
    //         // ...
    // 
    //         // Signal event to all handlers connected to the event.
    //         m_resizeEvent.Signal(width, height);
    //     }
    // 
    //     ResizeEvent m_resizeEvent;
    // };
    //
    // Any other code can define an event handler to register to the event and receive the callback
    // when the event is signaled.
    // 
    // ResizeEvent::Handler newHandler(
    //     [](int width, int height) {
    //         // Do something with the new size.
    //     });
    // 
    // foo.RegisterResizeEvent(newHandler);
    // -------------------------------------------------------

    template<typename CallbackType>
    class Event;

    template<typename CallbackType>
    class EventHandler
    {
    public:
        EventHandler() = default;
        EventHandler(CallbackType callback) 
            : m_callback(std::move(callback))
        {
        }

        void SetCallback(CallbackType callback)
        {
            m_callback = std::move(callback);
        }

        void Connect(Event<CallbackType>& event)
        {
            event.AddHandler(this);
        }

        void Disconnect(Event<CallbackType>& event)
        {
            event.RemoveHandler(this);
        }

    private:
        friend class Event<CallbackType>;

        CallbackType m_callback;
    };

    template<typename CallbackType>
    class Event
    {
    public:
        using Handler = EventHandler<CallbackType>;

        Event() = default;

        // Arguments must match the CallbackType arguments
        void Signal(auto&&... args)
        {
            for (auto* handler : m_handlers)
            {
                handler->m_callback(std::forward<decltype(args)>(args)...);
            }
        }

    protected:
        friend class Handler;

        void AddHandler(Handler* handler)
        {
            m_handlers.insert(handler);
        }

        void RemoveHandler(Handler* handler)
        {
            m_handlers.erase(handler);
        }

    private:
        std::unordered_set<Handler*> m_handlers;
    };
} // namespace DX

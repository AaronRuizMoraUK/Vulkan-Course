#pragma once

#include <memory>

namespace DX
{
    template<typename T>
    class Singleton
    {
    public:
        static T& Get()
        {
            if (!Instance)
            {
                Instance.reset(new T());
            }
            return *Instance;
        }

        static void Destroy()
        {
            Instance.reset();
        }

        virtual ~Singleton() = default;

        // Delete copy constructor and assignment operator to prevent copying
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() = default;

    private:
        static inline std::unique_ptr<T> Instance;
    };
} // namespace DX


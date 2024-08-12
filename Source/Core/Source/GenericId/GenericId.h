#pragma once

#include <cstdint>
#include <compare>
#include <xhash>

namespace DX
{
    // GenericId is a unique identifier for a type.
    //
    // Example of sow to use GenericId:
    // 
    // using DeviceId = DX::GenericId<struct DeviceIdTag>;
    // using WindowId = DX::GenericId<struct WindowIdTag>;
    // 
    // DeviceId deviceId;       // Invalid by default
    // deviceId = DeviceId{1};  // Valid
    // 
    // WindowId windowId(356); // Valid
    //
    // DeviceId are WindowId are strongly typed and therefore it won't
    // compile if they get mixed up.
    // 
    // deviceId = windowId; // Compilation error!

    template<typename Tag>
    class GenericId
    {
    public:
        GenericId() = default;

        explicit GenericId(const uint64_t value)
            : m_value(value)
        {
        }

        bool IsValid() const
        {
            return m_value > 0;
        }

        uint64_t GetValue() const
        {
            return m_value;
        }

        auto operator<=>(const GenericId& id) const = default;

    private:
        uint64_t m_value = 0;
    };
} // namespace DX

namespace std
{
    // Enables GenericId<Tag> to be keys in hashed data structures.
    template<typename Tag>
    struct hash<DX::GenericId<Tag>>
    {
        size_t operator()(const DX::GenericId<Tag>& id) const
        {
            return hash<uint64_t>{}(id.GetValue());
        }
    };
}

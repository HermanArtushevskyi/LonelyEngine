#ifndef LONELYENGINE_QUEUEFAMILYINDICES_H
#define LONELYENGINE_QUEUEFAMILYINDICES_H
#include <cstdint>
#include <optional>

namespace HelloTriangle
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
}

#endif //LONELYENGINE_QUEUEFAMILYINDICES_H
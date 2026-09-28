
#ifndef LONELYENGINE_UNIFORMBUFFEROBJECT_H
#define LONELYENGINE_UNIFORMBUFFEROBJECT_H
#include <glm/detail/type_mat2x2.hpp>

namespace HelloTriangle
{
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
} // HelloTriangle

#endif //LONELYENGINE_UNIFORMBUFFEROBJECT_H

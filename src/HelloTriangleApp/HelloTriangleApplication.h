#ifndef LONELYENGINE_HELLOTRIANGLEAPPLICATION_H
#define LONELYENGINE_HELLOTRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Vertex.h"
#include "vulkan/vulkan_raii.hpp"

namespace HelloTriangle
{
    #ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
    #else
    static constexpr bool enableValidationLayers = true;
    #endif

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                          void *pUserData)
    {
        std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

        return vk::False;
    }

    class HelloTriangleApplication
    {
        public:
            void run();
            bool framebufferResized = false;

        private:
            const int WIDTH = 1920;
            const int HEIGHT = 1080;
            const std::vector<char const *> VALIDATION_LAYERS = {
                "VK_LAYER_KHRONOS_validation"
            };

            const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS = {
                vk::KHRSwapchainExtensionName
            };

            const std::vector<Vertex> vertices = {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
            };

            const std::vector<uint16_t> indices = {
                0, 1, 2, 2, 3, 0
            };

            GLFWwindow *window = nullptr;
            uint32_t queueIndex = ~0;
            vk::raii::Context context;
            vk::raii::Instance instance = nullptr;
            vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
            vk::raii::PhysicalDevice physicalDevice = nullptr;
            vk::raii::Device device = nullptr;
            vk::raii::Queue graphicsQueue = nullptr;
            vk::raii::SurfaceKHR surface = nullptr;
            vk::raii::SwapchainKHR swapchain = nullptr;
            std::vector<vk::Image> swapchainImages;
            vk::SurfaceFormatKHR swapchainFormat;
            vk::Extent2D swapchainExtent;
            std::vector<vk::raii::ImageView> swapchainImageViews;
            vk::raii::PipelineLayout pipelineLayout = nullptr;
            vk::raii::Pipeline graphicsPipeline = nullptr;
            vk::raii::CommandPool commandPool = nullptr;
            std::vector<vk::raii::CommandBuffer> commandBuffers;
            std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
            std::vector<vk::raii::Semaphore> renderingCompleteSemaphores;
            std::vector<vk::raii::Fence> drawFences;
            vk::raii::Buffer vertexBuffer = nullptr;
            vk::raii::DeviceMemory vertexBufferMemory = nullptr;
            vk::raii::Buffer       indexBuffer        = nullptr;
            vk::raii::DeviceMemory indexBufferMemory  = nullptr;
            uint32_t frameIndex = 0;

            void initWindow();

            void initVulkan();

            void setupDebugMessenger();

            void pickPhysicalDevice();

            void createLogicalDevice();

            void createSurface();

            void createImageViews();

            void createVertexBuffer();

            void createIndexBuffer();

            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

            void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);

            uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

            void createGraphicsPipeline();

            void createCommandPool();

            void createCommandBuffers();

            void recordCommandBuffer(uint32_t imageIndex);

            void recreateSwapChain();

            void cleanupSwapChain();

            void transition_image_layout(
                uint32_t imageIndex,
                vk::ImageLayout old_layout,
                vk::ImageLayout new_layout,
                vk::AccessFlags2 src_access_mask,
                vk::AccessFlags2 dst_access_mask,
                vk::PipelineStageFlags2 src_stage_mask,
                vk::PipelineStageFlags2 dst_stage_mask);

            [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char> &code) const;

            uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &capabilities);

            void createSwapChain();

            bool isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice);

            void validateLayers();

            std::vector<char const *> getRequiredLayers();

            std::vector<const char *> getRequiredExtensions();

            void checkThatRequiredExtensionsArePresent();

            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);

            vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentations);

            vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);

            void createInstance();

            void createSyncObjects();

            void mainLoop();

            void drawFrame();

            void cleanup();
    };
} // HelloTriangle

#endif //LONELYENGINE_HELLOTRIANGLEAPPLICATION_H

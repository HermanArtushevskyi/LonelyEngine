#ifndef LONELYENGINE_HELLOTRIANGLEAPPLICATION_H
#define LONELYENGINE_HELLOTRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <iostream>
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_raii.hpp"

namespace HelloTriangle
{
    #ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
    #else
    static constexpr bool enableValidationLayers = true;
    #endif

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                          vk::DebugUtilsMessageTypeFlagsEXT              type,
                                          const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                          void *                                         pUserData)
    {
        std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

        return vk::False;
    }

    class HelloTriangleApplication
    {
        public:
            void run();

        private:
            const int WIDTH = 1920;
            const int HEIGHT = 1080;
            const std::vector<char const*> VALIDATION_LAYERS = {
                "VK_LAYER_KHRONOS_validation"
            };

            const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
                vk::KHRSwapchainExtensionName};

            GLFWwindow* window = nullptr;
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

            void initWindow();

            void initVulkan();

            void setupDebugMessenger();

            void pickPhysicalDevice();

            void createLogicalDevice();

            void createSurface();

            void createImageViews();

            void createGraphicsPipeline();

            uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR & capabilities);

            void createSwapChain();

            bool isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice);

            void validateLayers();

            std::vector<char const*> getRequiredLayers();

            std::vector<const char*> getRequiredExtensions();

            void checkThatRequiredExtensionsArePresent();

            vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);

            vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentations);

            vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);

            void createInstance();

            void mainLoop();

            void cleanup();
    };
} // HelloTriangle

#endif //LONELYENGINE_HELLOTRIANGLEAPPLICATION_H
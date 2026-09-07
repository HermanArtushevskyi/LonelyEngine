#ifndef LONELYENGINE_HELLOTRIANGLEAPPLICATION_H
#define LONELYENGINE_HELLOTRIANGLEAPPLICATION_H

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include "QueueFamilyIndices.h"

namespace HelloTriangle
{
    class HelloTriangleApplication
    {
        public:
            void run();

        private:
            #ifdef NDEBUG
            const bool enableValidationLayers = false;
            #else
            const bool enableValidationLayers = true;
            #endif
            const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
            };

            const int WIDTH = 1920;
            const int HEIGHT = 1080;


            GLFWwindow* window;
            VkInstance instance;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue;
            VkQueue presentQueue;
            VkSurfaceKHR surface;
            VkDebugUtilsMessengerEXT debugMessenger;

            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

            void initWindow();

            void initVulkan();

            void createInstance();

            void setupDebugMessanger();

            void mainLoop();

            void cleanup();

            bool checkValidationSupport();

            void pickPhysicalDevice();

            bool isDeviceSuitable(VkPhysicalDevice device);

            void createLogicalDevice();

            void createSurface();

            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

            std::vector<const char*> getRequiredExtensions();

            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    };
} // HelloTriangle

#endif //LONELYENGINE_HELLOTRIANGLEAPPLICATION_H
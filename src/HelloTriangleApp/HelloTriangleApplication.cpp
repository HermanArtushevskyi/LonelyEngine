#include "HelloTriangleApplication.h"

#include <vulkan/vulkan_raii.hpp>

namespace HelloTriangle
{
    void HelloTriangleApplication::run()
    {
        this->initWindow();
        this->initVulkan();
        this->mainLoop();
        this->cleanup();
    }

    void HelloTriangleApplication::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "VULKAN", nullptr, nullptr);
    }

    void HelloTriangleApplication::initVulkan()
    {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createGraphicsPipeline();
    }

    void HelloTriangleApplication::createInstance()
    {
        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Hello Triangle!",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = vk::ApiVersion14
        };

        checkThatRequiredExtensionsArePresent();

        auto requiredLayers = getRequiredLayers();
        auto requiredExtensions = getRequiredExtensions();

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
        };

        instance = vk::raii::Instance(context, createInfo);
    }

    void HelloTriangleApplication::setupDebugMessenger()
    {
        if (enableValidationLayers == false) return;
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                           );

        vk::DebugUtilsMessageTypeFlagsEXT typeFlags(
                                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                                                   );

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .messageSeverity = severityFlags,
            .messageType = typeFlags,
            .pfnUserCallback = &debugCallback
        };
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo);
    }

    void HelloTriangleApplication::createSurface()
    {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
        {
            throw std::runtime_error("Could not create glfw surface!");
        }
        surface = vk::raii::SurfaceKHR(instance, _surface);
    }

    void HelloTriangleApplication::createImageViews()
    {
        assert(swapchainImages.empty());

        vk::ImageViewCreateInfo imageCreateInfo{
            .viewType = vk::ImageViewType::e2D,
            .format = swapchainFormat.format,
            .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        };

        for (auto &image : swapchainImages)
        {
            imageCreateInfo.image = image;
            swapchainImageViews.emplace_back(device, imageCreateInfo);
        }
    }

    void HelloTriangleApplication::createGraphicsPipeline()
    {
    }

    void HelloTriangleApplication::pickPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();

        if (physicalDevices.empty()) throw std::runtime_error("Could not find physical devices");

        for (auto device: physicalDevices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == nullptr) throw std::runtime_error("Could not find a suitable device");
    }

    void HelloTriangleApplication::createLogicalDevice()
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        uint32_t queueIndex = ~0;
        for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
        {
            if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
            {
                queueIndex = qfpIndex;
                break;
            }
        }

        if (queueIndex == ~0)
        {
            throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
        }

        vk::StructureChain<vk::PhysicalDeviceFeatures2,
                           vk::PhysicalDeviceVulkan11Features,
                           vk::PhysicalDeviceVulkan13Features,
                           vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
                featureChain = {
                    {},
                    {.shaderDrawParameters = true},
                    {.dynamicRendering = true},
                    {.extendedDynamicState = true}
                };
        float queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
            .queueFamilyIndex = queueIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size()),
            .ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data()
        };

        device = vk::raii::Device(physicalDevice, deviceCreateInfo);
        graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
    }

    void HelloTriangleApplication::createSwapChain()
    {
        vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        swapchainExtent = chooseSwapExtent(capabilities);
        uint32_t minImageCount = ChooseSwapMinImageCount(capabilities);

        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
        swapchainFormat = chooseSwapSurfaceFormat(availableFormats);

        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR( *surface );
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

        vk::SwapchainCreateInfoKHR swapChainCreateStruct{
            .surface = *surface,
            .minImageCount = minImageCount,
            .imageFormat = swapchainFormat.format,
            .imageColorSpace = swapchainFormat.colorSpace,
            .imageExtent = swapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentMode,
            .clipped = true,
            .oldSwapchain = nullptr
        };

        swapchain = vk::raii::SwapchainKHR(device, swapChainCreateStruct);
        swapchainImages = swapchain.getImages();
    }

    uint32_t HelloTriangleApplication::ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &capabilities)
    {
        auto minImageCount = std::max(3u, capabilities.minImageCount);
        if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < minImageCount))
        {
            minImageCount = capabilities.maxImageCount;
        }
        return minImageCount;
    }

    bool HelloTriangleApplication::isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice)
    {
        auto deviceProperties = physicalDevice.getProperties();
        auto deviceFeatures = physicalDevice.getFeatures();
        bool supportVK13 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        bool supportGraphics = std::ranges::any_of(
                                                   queueFamilies,
                                                   [](auto const &qfp)
                                                   {
                                                       return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
                                                   }
                                                  );
        std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions =
                std::ranges::all_of(requiredDeviceExtension,
                                    [&availableDeviceExtensions](auto const &requiredDeviceExtension)
                                    {
                                        return std::ranges::any_of(availableDeviceExtensions,
                                                                   [requiredDeviceExtension](
                                                               auto const &availableDeviceExtension)
                                                                   {
                                                                       return strcmp(availableDeviceExtension.
                                                                           extensionName,
                                                                           requiredDeviceExtension) == 0;
                                                                   });
                                    });

        auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                             vk::PhysicalDeviceVulkan11Features,
                                                             vk::PhysicalDeviceVulkan13Features,
                                                             vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters
                &&
                features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;


        return supportVK13 && supportGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    }

    std::vector<const char *> HelloTriangleApplication::getRequiredLayers()
    {
        std::vector<char const *> requiredLayers;
        if (enableValidationLayers)
        {
            requiredLayers.assign(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());
        }

        auto layerProperties = context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIterator = std::ranges::find_if(
                                                             requiredLayers,
                                                             [&layerProperties](auto const &requiredLayer)
                                                             {
                                                                 return std::ranges::none_of(
                                                                  layerProperties,
                                                                  [requiredLayer](auto const &layerProperties)
                                                                  {
                                                                      return strcmp(layerProperties.layerName,
                                                                          requiredLayer) == 0;
                                                                  });
                                                             });

        if (unsupportedLayerIterator != requiredLayers.end())
        {
            throw std::runtime_error("Required layers not supported: " + std::string(*unsupportedLayerIterator));
        }
        return requiredLayers;
    }

    void HelloTriangleApplication::checkThatRequiredExtensionsArePresent()
    {
        auto requiredExtensions = getRequiredExtensions();
        auto extensionProperties = context.enumerateInstanceExtensionProperties();
        auto unsupportedPropertyIt =
                std::ranges::find_if(requiredExtensions,
                                     [&extensionProperties](auto const &requiredExtension)
                                     {
                                         return std::ranges::none_of(extensionProperties,
                                                                     [requiredExtension](auto const &extensionProperty)
                                                                     {
                                                                         return strcmp(extensionProperty.extensionName,
                                                                             requiredExtension) == 0;
                                                                     });
                                     });
        if (unsupportedPropertyIt != requiredExtensions.end())
        {
            throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
        }
    }

    vk::SurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(
        std::vector<vk::SurfaceFormatKHR> const &availableFormats)
    {
        assert(availableFormats.empty() == false);
        const auto formatIt = std::ranges::find_if(
                                                   availableFormats,
                                                   [](const auto &format)
                                                   {
                                                       return format.format == vk::Format::eB8G8R8A8Srgb && format.
                                                               colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                                                   });
        return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
    }

    vk::PresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(
        std::vector<vk::PresentModeKHR> const &availablePresentations)
    {
        assert(std::ranges::any_of(availablePresentations, [](auto presentMode) { return presentMode == vk::
                   PresentModeKHR::eFifo; }));
        return std::ranges::any_of(availablePresentations,
                                   [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; })
                   ? vk::PresentModeKHR::eMailbox
                   : vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D HelloTriangleApplication::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        return {
            std::clamp<uint32_t>(width,
                capabilities.minImageExtent.width, capabilities.maxImageExtent.width),

            std::clamp<uint32_t>(height,
                capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    void HelloTriangleApplication::validateLayers()
    {
        std::vector<char const *> requiredLayers;
        if (enableValidationLayers)
        {
            requiredLayers.assign(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());
        }

        auto layerProperties = context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIterator = std::ranges::find_if(
                                                             requiredLayers,
                                                             [&layerProperties](auto const &requiredLayer)
                                                             {
                                                                 return std::ranges::none_of(
                                                                  layerProperties,
                                                                  [requiredLayer](auto const &layerProperties)
                                                                  {
                                                                      return strcmp(layerProperties.layerName,
                                                                          requiredLayer) == 0;
                                                                  });
                                                             });

        if (unsupportedLayerIterator != requiredLayers.end())
        {
            throw std::runtime_error("Required layers not supported: " + std::string(*unsupportedLayerIterator));
        }
    }

    std::vector<const char *> HelloTriangleApplication::getRequiredExtensions()
    {
        uint32_t glfwExtensionsCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

        if (enableValidationLayers)
        {
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        return extensions;
    }

    void HelloTriangleApplication::mainLoop()
    {
        while (glfwWindowShouldClose(window) == false)
        {
            glfwPollEvents();
        }
    }

    void HelloTriangleApplication::cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
} // HelloTriangle

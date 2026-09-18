#include "HelloTriangleApplication.h"

#include <fstream>
#include <vulkan/vulkan_raii.hpp>

namespace HelloTriangle
{
    static std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        std::vector<char> buffer(file.tellg());

        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

        file.close();

        return buffer;
    }

    void HelloTriangleApplication::run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
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
        createImageViews();
        createGraphicsPipeline();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
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

    void HelloTriangleApplication::createSyncObjects()
    {
        assert(presentCompleteSemaphores.empty() && renderingCompleteSemaphores.empty() && drawFences.empty());

        for (size_t i = 0; i < swapchainImages.size(); i++)
        {
            renderingCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
            drawFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
        }
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
        assert(swapchainImageViews.empty());

        vk::ImageViewCreateInfo imageCreateInfo{
            .viewType = vk::ImageViewType::e2D,
            .format = swapchainFormat.format,
            .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        };

        for (auto &image: swapchainImages)
        {
            imageCreateInfo.image = image;
            swapchainImageViews.emplace_back(device, imageCreateInfo);
        }
    }

    void HelloTriangleApplication::createGraphicsPipeline()
    {
        auto shaderCode = readFile("../../shaders/slang.spv");
        vk::raii::ShaderModule shaderModule = createShaderModule(shaderCode);
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"
        };
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"
        };
        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
            .topology = vk::PrimitiveTopology::eTriangleList
        };

        vk::Viewport viewport{
            0.0f,
            0.0f,
            static_cast<float>(swapchainExtent.width),
            static_cast<float>(swapchainExtent.height),
            0.0f,
            1.0f
        };
        vk::Rect2D scissors{vk::Offset2D{0, 0}, swapchainExtent};

        vk::PipelineViewportStateCreateInfo viewportState{
            .viewportCount = 1, .pViewports = &viewport,
            .scissorCount = 1, .pScissors = &scissors
        };

        vk::PipelineRasterizationStateCreateInfo rasterizer{
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = vk::False,
            .lineWidth = 1.0f
        };

        vk::PipelineMultisampleStateCreateInfo multisampling{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False
        };

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = vk::False,
            .colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment
        };

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};
        pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swapchainFormat.format
        };

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicState{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()
        };


        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
            {
                .stageCount = 2,
                .pStages = shaderStages,
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssemblyInfo,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicState,
                .layout = pipelineLayout,
                .renderPass = nullptr
            },
            {.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapchainFormat.format}
        };

        graphicsPipeline = vk::raii::Pipeline(device, nullptr,
                                              pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
    }

    void HelloTriangleApplication::createCommandPool()
    {
        vk::CommandPoolCreateInfo poolInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueIndex
        };

        commandPool = vk::raii::CommandPool(device, poolInfo);
    }

    void HelloTriangleApplication::createCommandBuffers()
    {
        vk::CommandBufferAllocateInfo allocInfo
        {
            .commandPool = commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };

        commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
    }

    void HelloTriangleApplication::recordCommandBuffer(uint32_t imageIndex)
    {
        auto &commandBuffer = commandBuffers[frameIndex];
        commandBuffer.begin({});
        transition_image_layout(imageIndex,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eColorAttachmentOptimal,
                                {},
                                vk::AccessFlagBits2::eColorAttachmentWrite,
                                vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        vk::ClearColorValue clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo =
        {
            .imageView = swapchainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clearValue
        };

        vk::RenderingInfo renderingInfo = {
            .renderArea           = {.offset = {0, 0}, .extent = swapchainExtent},
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &attachmentInfo};

        commandBuffer.beginRendering(renderingInfo);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
        commandBuffer.setViewport(0,
            vk::Viewport(0.0f, 0.0f,
                static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height),
                0.0f, 1.0f));
        commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRendering();
        transition_image_layout(
            imageIndex,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );
        commandBuffer.end();
    }

    void HelloTriangleApplication::transition_image_layout(
        uint32_t imageIndex,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask)
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = src_stage_mask,
            .srcAccessMask = src_access_mask,
            .dstStageMask = dst_stage_mask,
            .dstAccessMask = dst_access_mask,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = swapchainImages[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vk::DependencyInfo dependency_info = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };
        commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
    }

    vk::raii::ShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char> &code) const
    {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t *>(code.data())
        };
        vk::raii::ShaderModule shaderModule{device, createInfo};
        return shaderModule;
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
        queueIndex = ~0;
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
                    {.synchronization2 = true, .dynamicRendering = true},
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

        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
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
            drawFrame();
        }

        device.waitIdle();
    }

    void HelloTriangleApplication::drawFrame()
    {
        auto fenceResult = device.waitForFences(*drawFences[frameIndex], vk::True, UINT64_MAX);

        if (fenceResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to wait for fence");
        }

        device.resetFences(*drawFences[frameIndex]);

        auto [result, imageIndex] = swapchain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
        commandBuffers[frameIndex].reset();
        recordCommandBuffer(imageIndex);
        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
        const vk::SubmitInfo   submitInfo{.waitSemaphoreCount   = 1,
                                          .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
                                          .pWaitDstStageMask    = &waitDestinationStageMask,
                                          .commandBufferCount   = 1,
                                          .pCommandBuffers      = &*commandBuffers[frameIndex],
                                          .signalSemaphoreCount = 1,
                                          .pSignalSemaphores    = &*renderingCompleteSemaphores[imageIndex]};
        graphicsQueue.submit(submitInfo, *drawFences[frameIndex]);
        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderingCompleteSemaphores[imageIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &*swapchain,
            .pImageIndices      = &imageIndex};
        result = graphicsQueue.presentKHR(presentInfoKHR);

        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void HelloTriangleApplication::cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
} // HelloTriangle

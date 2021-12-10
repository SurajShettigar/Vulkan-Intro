#include "vulkan_renderer.h"

static void onFrameBufferResized(GLFWwindow *window, int width, int height)
{
    VulkanRenderer *renderer = reinterpret_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
    renderer->frameBufferResized = true;
}

const vector<const char *> VulkanRenderer::VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

bool VulkanRenderer::checkValidationLayersSupport()
{
    uint32_t layerCount = 0;

    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (layerCount == 0)
        return false;

    vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for (const char *reqLayer : VALIDATION_LAYERS)
    {
        bool isLayerFound = false;
        for (const VkLayerProperties &l : layers)
        {
            if (strcmp(l.layerName, reqLayer) == 0)
            {
                isLayerFound = true;
                break;
            }
        }

        if (!isLayerFound)
            return false;
    }

    return true;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = VkDebugUtilsMessengerCreateInfoEXT{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.messageSeverity = /*  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanRenderer::createDebugMessenger() throw()
{
    if (!isValidationLayersEnabled)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                          "vkCreateDebugUtilsMessengerEXT");

    if (func == nullptr)
        throw RendererException("Failed to create debug messenger");
    else
    {
        func(instance, &createInfo, nullptr, &debugMessenger);
    }
}

void VulkanRenderer::destoryDebugMessenger() throw()
{
    if (!isValidationLayersEnabled)
        return;

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                           "vkDestroyDebugUtilsMessengerEXT");

    if (func == nullptr)
        throw RendererException("Failed to destroy debug messenger");
    else
    {
        func(instance, debugMessenger, nullptr);
    }
}

vector<const char *> VulkanRenderer::getRequiredExtensions()
{
    uint32_t extenstionCount = 0;

    const char **extensions = glfwGetRequiredInstanceExtensions(&extenstionCount);

    vector<const char *> reqExtensions(extensions, extensions + extenstionCount);

    if (isValidationLayersEnabled)
        reqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return reqExtensions;
}

bool VulkanRenderer::hasRequiredExtenstions(const vector<const char *> &requiredExtensions)
{
    uint32_t vExtenstionCount = 0;

    vkEnumerateInstanceExtensionProperties(nullptr, &vExtenstionCount, nullptr);

    vector<VkExtensionProperties> vExtenstions(vExtenstionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vExtenstionCount, vExtenstions.data());

    for (const char *e : requiredExtensions)
    {
        bool hasExtension = false;
        for (VkExtensionProperties p : vExtenstions)
        {
            if (strcmp(e, p.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension)
            return false;
    }
    return true;
}

void VulkanRenderer::createVulkanInstance() throw()
{
    // Application Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Intro";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Vulkan create instance info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get required extensions
    vector<const char *> reqExtensions = getRequiredExtensions();
    if (!hasRequiredExtenstions(reqExtensions))
        throw RendererException("Current device does not have the required extensions");

    createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
    createInfo.ppEnabledExtensionNames = reqExtensions.data();

    // Set vulkan layers
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (isValidationLayersEnabled && !checkValidationLayersSupport())
        throw RendererException("Requested validation layers are not available");

    if (isValidationLayersEnabled)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    // Create Vulkan instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VkResult::VK_SUCCESS)
        throw("Failed to create a Vulkan Instance");
}

void VulkanRenderer::createSurface() throw()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw RendererException("Failed to create window surface");
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(const VkPhysicalDevice &device)
{
    QueueFamilyIndices indices;
    uint32_t qFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qFamilyCount, nullptr);
    vector<VkQueueFamilyProperties> qFamilyProps(qFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qFamilyCount, qFamilyProps.data());

    int i = 0;
    for (const VkQueueFamilyProperties &f : qFamilyProps)
    {
        if (f.queueCount > 0 && f.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 isPresentationSupported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &isPresentationSupported);
        if (f.queueCount > 0 && isPresentationSupported)
            indices.presentationFamily = i;

        if (indices.isValid())
            break;
        ++i;
    }
    return indices;
}

bool VulkanRenderer::isDeviceExtensionSupported(const VkPhysicalDevice &device)
{
    uint32_t extensionsCount = 0;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    if (extensionsCount == 0)
        return false;

    vector<VkExtensionProperties> extensions(extensionsCount);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

    for (const char *reqExt : DEVICE_EXTENSIONS)
    {
        bool isFound = false;

        for (VkExtensionProperties devExt : extensions)
        {
            if (strcmp(reqExt, devExt.extensionName) == 0)
            {
                isFound = true;
                break;
            }
        }

        if (!isFound)
            return false;
    }

    return true;
}

SwapchainDetails VulkanRenderer::getSwapchainDetails(const VkPhysicalDevice &device)
{
    SwapchainDetails swapchainDetails;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainDetails.surfaceCapabilities);

    uint32_t formatCount = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        swapchainDetails.fromats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             surface,
                                             &formatCount,
                                             swapchainDetails.fromats.data());
    }

    uint32_t presModCount = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presModCount, nullptr);
    if (presModCount != 0)
    {
        swapchainDetails.presentationModes.resize(presModCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                  surface,
                                                  &presModCount,
                                                  swapchainDetails.presentationModes.data());
    }

    return swapchainDetails;
}

bool VulkanRenderer::isDeviceSuitable(const VkPhysicalDevice &device)
{
    VkPhysicalDeviceProperties dProps;
    VkPhysicalDeviceFeatures dFeatures;

    vkGetPhysicalDeviceProperties(device, &dProps);
    vkGetPhysicalDeviceFeatures(device, &dFeatures);

    QueueFamilyIndices indices = getQueueFamilies(device);

    bool extSupported = isDeviceExtensionSupported(device);

    bool swapchainValid;
    if (extSupported)
    {
        SwapchainDetails swapchainDetails = getSwapchainDetails(device);
        swapchainValid = !swapchainDetails.fromats.empty() && !swapchainDetails.presentationModes.empty();
    }

    return indices.isValid() && extSupported && swapchainValid;
}

void VulkanRenderer::getPhysicalDevice() throw()
{
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw RendererException("No GPU found which supports Vulkan");

    vector<VkPhysicalDevice> devices(deviceCount);

    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    bool suitableDeviceFound = false;
    for (const VkPhysicalDevice &d : devices)
    {
        if (isDeviceSuitable(d))
        {
            suitableDeviceFound = true;
            mainDevice.physicalDevice = d;
            break;
        }
    }

    if (!suitableDeviceFound)
        throw RendererException("No GPU found which has the required extensions");
}

void VulkanRenderer::createLogicalDevice() throw()
{
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    std::set<int> queueFamiliyIndices = {indices.graphicsFamily, indices.presentationFamily};
    vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (int i : queueFamiliyIndices)
    {
        VkDeviceQueueCreateInfo queueInfo = VkDeviceQueueCreateInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext = nullptr;
        queueInfo.queueFamilyIndex = i;
        queueInfo.queueCount = 1;
        float queuePriority = 1.0;
        queueInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo = VkDeviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;

    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    VkPhysicalDeviceFeatures features = VkPhysicalDeviceFeatures{};
    deviceCreateInfo.pEnabledFeatures = &features;

    VkResult result = vkCreateDevice(mainDevice.physicalDevice,
                                     &deviceCreateInfo,
                                     nullptr,
                                     &mainDevice.logicalDevice);

    if (result != VK_SUCCESS)
        throw RendererException("Failed to create logical device");

    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const vector<VkSurfaceFormatKHR> &formats)
{
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_R8G8B8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    for (const VkSurfaceFormatKHR &f : formats)
    {
        if (f.format == VK_FORMAT_R8G8B8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    }

    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const vector<VkPresentModeKHR> &presentationModes)
{

    for (const VkPresentModeKHR &p : presentationModes)
    {
        if (p == VK_PRESENT_MODE_MAILBOX_KHR)
            return p;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilites)
{
    if (surfaceCapabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return surfaceCapabilites.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D newExtent{};
        newExtent.width = static_cast<uint32_t>(width);
        newExtent.height = static_cast<uint32_t>(height);

        newExtent.width = std::clamp(newExtent.width,
                                     surfaceCapabilites.minImageExtent.width,
                                     surfaceCapabilites.maxImageExtent.width);
        newExtent.height = std::clamp(newExtent.height,
                                      surfaceCapabilites.minImageExtent.height,
                                      surfaceCapabilites.maxImageExtent.height);
    }
}

VkImageView VulkanRenderer::createImageView(VkImage image,
                                            VkFormat format,
                                            VkImageAspectFlags aspectFlags) throw()
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;

    VkResult result = vkCreateImageView(mainDevice.logicalDevice, &createInfo, nullptr, &imageView);

    if (result != VK_SUCCESS)
        throw RendererException("Could not create image view");

    return imageView;
}

void VulkanRenderer::createSwapchain() throw()
{
    SwapchainDetails swapchainDetails = getSwapchainDetails(mainDevice.physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapchainDetails.fromats);
    VkPresentModeKHR presentationMode = chooseBestPresentationMode(swapchainDetails.presentationModes);
    VkExtent2D extent = chooseSwapExtent(swapchainDetails.surfaceCapabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;

    createInfo.surface = surface;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.presentMode = presentationMode;
    createInfo.imageExtent = extent;

    uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;
    if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 && swapchainDetails.surfaceCapabilities.maxImageCount < imageCount)
        imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;

    createInfo.minImageCount = imageCount;

    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapchainDetails.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;

    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        uint32_t qIndices[]{static_cast<uint32_t>(indices.graphicsFamily),
                            static_cast<uint32_t>(indices.presentationFamily)};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = qIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &createInfo, nullptr, &swapchain);

    if (result != VK_SUCCESS)
        throw RendererException("Failed to create swapchain");

    swapchainFormat = surfaceFormat.format;
    swapchainExtent = extent;

    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapchainImageCount, nullptr);
    vector<VkImage> images(swapchainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapchainImageCount, images.data());
    for (VkImage i : images)
    {
        SwapchainImage swapchainImage{};
        swapchainImage.image = i;

        swapchainImage.imageView = createImageView(i, swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        swapchainImages.push_back(swapchainImage);
    }
}

VkShaderModule VulkanRenderer::createShaderModule(const vector<char> &code) throw()
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(mainDevice.logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw RendererException("Failed to create shader module.");

    return shaderModule;
}

void VulkanRenderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw RendererException("Failed to create render pass!");
}

void VulkanRenderer::createGraphicsPipeline()
{
    // Shader Creation
    vector<char> vertexShaderCode = readBinFile(VERTEX_SHADER_PATH);
    vector<char> fragShaderCode = readBinFile(FRAGMENT_SHADER_PATH);

    VkShaderModule vertexShader = createShaderModule(vertexShaderCode);
    VkShaderModule fragShader = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShader;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.pNext = nullptr;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    // Input Assmebly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.pNext = nullptr;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo{};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.pNext = nullptr;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.scissorCount = 1;
    viewportCreateInfo.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.pNext = nullptr;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo{};
    multiSamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSamplingCreateInfo.pNext = nullptr;
    multiSamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multiSamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multiSamplingCreateInfo.minSampleShading = 1.0f;
    multiSamplingCreateInfo.pSampleMask = nullptr;
    multiSamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multiSamplingCreateInfo.alphaToOneEnable = VK_FALSE;

    // Depth Testing

    // Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.pNext = nullptr;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendCreateInfo.blendConstants[0] = 0.0f;
    colorBlendCreateInfo.blendConstants[1] = 0.0f;
    colorBlendCreateInfo.blendConstants[2] = 0.0f;
    colorBlendCreateInfo.blendConstants[3] = 0.0f;

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw RendererException("Failed to create a pipeline layout!");

    // Final Pipeline Assembly
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    // Shader/Programmable stage
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    // Fixed-function stage
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multiSamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    // Layout + Render Pass
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    // Reference to other base pipelines
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(mainDevice.logicalDevice,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineCreateInfo,
                                  nullptr,
                                  &graphicsPipeline) != VK_SUCCESS)
        throw RendererException("Failed to create graphics pipeline!");

    vkDestroyShaderModule(mainDevice.logicalDevice, vertexShader, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, fragShader, nullptr);
}

void VulkanRenderer::createFramebuffers()
{
    swapchainFramebuffers.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VkImageView attachments[] = {
            swapchainImages[i].imageView};

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.renderPass = renderPass;
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;
        createInfo.layers = 1;

        if (vkCreateFramebuffer(mainDevice.logicalDevice, &createInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
            throw RendererException("Failed to create framebuffer for swapchain image: " + i);
    }
}

void VulkanRenderer::createCommandPool()
{
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.queueFamilyIndex = static_cast<uint32_t>(indices.graphicsFamily);
    createInfo.flags = 0;

    if (vkCreateCommandPool(mainDevice.logicalDevice, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw RendererException("Failed to create a command pool.");
}

void VulkanRenderer::createCommandBuffers()
{
    commandBuffers.resize(swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(mainDevice.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw RendererException("Failed to allocate command buffers.");

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS)
            throw RendererException("Failed to begin recording command buffer for index: " + i);

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.framebuffer = swapchainFramebuffers[i];
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw RendererException("Failed to record command buffer.");
    }
}

void VulkanRenderer::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(mainDevice.logicalDevice,
                              &semaphoreCreateInfo,
                              nullptr,
                              &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mainDevice.logicalDevice,
                              &semaphoreCreateInfo,
                              nullptr,
                              &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mainDevice.logicalDevice,
                          &fenceCreateInfo,
                          nullptr,
                          &inFlightFences[i]) != VK_SUCCESS)
            throw RendererException("Failed to create sync objects.");
    }
}

void VulkanRenderer::drawFrame()
{
    vkWaitForFences(mainDevice.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mainDevice.logicalDevice,
                                            swapchain,
                                            UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw RendererException("Failed to acquire swapchain image.");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(mainDevice.logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    vkResetFences(mainDevice.logicalDevice, 1, &inFlightFences[currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        throw RendererException("Failed to submit draw command buffer.");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentationQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized)
    {
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image.");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::cleanSwapchain()
{
    for (VkFramebuffer f : swapchainFramebuffers)
        vkDestroyFramebuffer(mainDevice.logicalDevice, f, nullptr);
    vkFreeCommandBuffers(mainDevice.logicalDevice,
                         commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);
    for (SwapchainImage s : swapchainImages)
        vkDestroyImageView(mainDevice.logicalDevice, s.imageView, nullptr);
    swapchainImages.clear();
    vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);
}

void VulkanRenderer::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mainDevice.logicalDevice);

    cleanSwapchain();

    createSwapchain();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
}

void VulkanRenderer::start() throw()
{
    // throw RendererException("Exception Test");
    if (glfwInit() == GLFW_FALSE)
    {
        throw RendererException("Failed to initialize Window");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(winWidth, winHeight, winName, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, onFrameBufferResized);

    createVulkanInstance();
    createDebugMessenger();
    createSurface();
    getPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanRenderer::update()
{
}

void VulkanRenderer::render()
{
    drawFrame();
}

void VulkanRenderer::destroy()
{
    cleanSwapchain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(mainDevice.logicalDevice, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(mainDevice.logicalDevice, commandPool, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    destoryDebugMessenger();
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

int VulkanRenderer::init(int width, int height, const char *name)
{
    winWidth = width;
    winHeight = height;
    winName = name;

    try
    {
        start();
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            update();
            render();
        }
        vkDeviceWaitIdle(mainDevice.logicalDevice);
        destroy();
        return 0;
    }
    catch (const RendererException &e)
    {
        cerr << e.what() << endl;
        return -1;
    }
}
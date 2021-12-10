#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "renderer.h"
#include "vulkan_utilities.h"

struct VulkanDevice
{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
};

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentationFamily = -1;

    bool isValid()
    {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};

class VulkanRenderer : Renderer
{
private:
    static const int MAX_FRAMES_IN_FLIGHT = 2;

    GLFWwindow *window;

    VkInstance instance;
    VulkanDevice mainDevice;

    VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    VkQueue presentationQueue;

    VkSwapchainKHR swapchain;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    vector<SwapchainImage> swapchainImages;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    vector<VkFramebuffer> swapchainFramebuffers;

    VkCommandPool commandPool;
    vector<VkCommandBuffer> commandBuffers;

    vector<VkSemaphore> imageAvailableSemaphores;
    vector<VkSemaphore> renderFinishedSemaphores;
    vector<VkFence> inFlightFences;
    vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

#ifdef NDEBUG
    static const bool isValidationLayersEnabled = false;
#else
    static const bool isValidationLayersEnabled = true;
#endif
    static const vector<const char *> VALIDATION_LAYERS;
    bool checkValidationLayersSupport();

    VkDebugUtilsMessengerEXT debugMessenger;
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void createDebugMessenger() throw();
    void destoryDebugMessenger() throw();

    vector<const char *> getRequiredExtensions();
    bool hasRequiredExtenstions(const vector<const char *> &requiredExtensions);
    void createVulkanInstance() throw();

    void createSurface() throw();

    QueueFamilyIndices getQueueFamilies(const VkPhysicalDevice &device);
    bool isDeviceExtensionSupported(const VkPhysicalDevice &device);
    SwapchainDetails getSwapchainDetails(const VkPhysicalDevice &device);
    bool isDeviceSuitable(const VkPhysicalDevice &device);
    void getPhysicalDevice() throw();

    void createLogicalDevice() throw();

    VkSurfaceFormatKHR chooseBestSurfaceFormat(const vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR chooseBestPresentationMode(const vector<VkPresentModeKHR> &presentationModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilites);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) throw();
    void createSwapchain() throw();

    VkShaderModule createShaderModule(const vector<char> &code) throw();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();

    void createCommandPool();
    void createCommandBuffers();
    
    void createSyncObjects();
    void drawFrame();

    void cleanSwapchain();
    void recreateSwapchain();

protected:
    void start() throw() override;
    void update() override;
    void render() override;
    void destroy() override;

public:
    bool frameBufferResized;
    
    VulkanRenderer() = default;
    int init(int width, int height, const char *name) override;
};

#endif
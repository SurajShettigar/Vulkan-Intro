#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <string.h>

#include <iostream>

#include "../utils/file_utils.h"

using std::vector;
using std::cerr;
using std::cout;
using std::endl;


static const std::string VERTEX_SHADER_PATH = "./shaders/shader.vert.spv";
static const std::string FRAGMENT_SHADER_PATH = "./shaders/shader.frag.spv";

const vector<const char*> DEVICE_EXTENSIONS {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

struct SwapchainDetails
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vector<VkSurfaceFormatKHR> fromats;
    vector<VkPresentModeKHR> presentationModes;
};

struct SwapchainImage
{
    VkImage image;
    VkImageView imageView;
};
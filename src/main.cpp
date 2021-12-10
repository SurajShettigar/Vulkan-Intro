#include "./engine/vulkan_renderer.h"

// #include <iostream>
// #include <vulkan/vulkan.hpp>

// using std::cout;
// using std::endl;

int main(int argc, char **argv)
{
    VulkanRenderer renderer;
    int status = renderer.init(1280, 720, "Window");
    char* input;
    std::cout << "Press any key to continue";
    std::cin >> input;

    return status;

    // uint32_t vExtenstionCount = 0;
    // vkEnumerateInstanceExtensionProperties(nullptr, &vExtenstionCount, nullptr);
    // std::vector<VkExtensionProperties> vExtenstions(vExtenstionCount);
    // vkEnumerateInstanceExtensionProperties(nullptr, &vExtenstionCount, vExtenstions.data());

    // for (const VkExtensionProperties &e : vExtenstions)
    //     cout << "Supported Extensions: " << e.extensionName << endl;
    // return 0;
    
    // uint32_t layerCount = 0;

    // vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    // std::vector<VkLayerProperties> layers = std::vector<VkLayerProperties>(layerCount);
    // vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    // for (const VkLayerProperties &l : layers)
    //     cout << "Supported Layers: " << l.layerName << endl;
    // return 0;
}
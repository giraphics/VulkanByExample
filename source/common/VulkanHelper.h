#pragma once
#include <vulkan/vulkan.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
using namespace std;

// A structure to store physical device information
struct PhysicalDeviceInfo
{
    VkPhysicalDeviceProperties              prop = {};
    VkPhysicalDeviceFeatures                features = {};
    VkPhysicalDeviceMemoryProperties        memProp = {};
    std::vector<VkQueueFamilyProperties>    familyPropList;
    int                                     graphicsFamilyIndex = -1;
    int                                     presentFamilyIndex = -1;
    std::vector<VkExtensionProperties>      extensionList;
    VkSurfaceCapabilitiesKHR                capabilities = {};
    std::vector<VkSurfaceFormatKHR>         formatList;
    std::vector<VkPresentModeKHR>           presentModeList;
};

struct LayerProperties
{
	VkLayerProperties						properties;
	std::vector<VkExtensionProperties>		extensions;
};

class VulkanHelper
{
public:
	VulkanHelper();
	virtual ~VulkanHelper();


protected:
	// General helper functions
	void LogError(string text);

	// Layer and Extension helper functions
	VkResult GetInstanceLayerExtensionProperties();
	VkResult GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu);
	VkResult GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice gpu = nullptr);

	// Create Swap chain helper functions
	VkSurfaceFormatKHR SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D& windowDim);

	// Shader helper funcitons
	VkShaderModule CreateShader(VkDevice device, const std::string& filename);

	static bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex);
private:
	// Layer property list containing Layers and respective extensions
	std::vector<LayerProperties> m_LayerPropertyList;
};
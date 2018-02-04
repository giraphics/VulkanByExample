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

	// Create Swap chain helper functions
	static VkSurfaceFormatKHR SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	static VkExtent2D SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D& windowDim);

	// Layer and Extension helper functions
	static VkResult GetInstanceLayerExtensionProperties();
	static VkResult GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu);
	static VkResult GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice gpu = nullptr);

	// General helper functions
	static void LogError(string text);

	// Shader helper funcitons
	static VkShaderModule CreateShader(VkDevice device, const std::string& filename);

	static bool MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex);

	static void CreateCommandPool(const VkDevice& device, VkCommandPool& cmdPool, const PhysicalDeviceInfo& deviceInfo, const VkCommandPoolCreateInfo* commandBufferInfo = NULL);
	static void AllocateCommandBuffer(const VkDevice device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo = NULL);
	static void BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo = NULL);
	static void EndCommandBuffer(VkCommandBuffer cmdBuf);
	static void SubmitCommandBuffer(const VkQueue& queue, const VkCommandBuffer cmdBufList, const VkSubmitInfo* submitInfo = NULL, const VkFence& fence = VK_NULL_HANDLE);
	static void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& commandBuffer);

	static VkResult createBuffer(const VkDevice logicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties deviceMemProp, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory); // Please refrain the use of this createBuffer, still under experiment
	static VkResult createBuffer(const VkDevice device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties deviceMemProp, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr); // Please refrain the use of this createBuffer, still under experiment
	static void CreateBuffer(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, const void * vertexData, uint32_t dataSize, uint32_t dataStride, VkBuffer *buffer, VkDeviceMemory* memory); // Please use this Create Buffer currently begin used

public:
	// Layer property list containing Layers and respective extensions
	static std::vector<LayerProperties> m_LayerPropertyList;
};
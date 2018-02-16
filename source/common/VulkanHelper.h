#pragma once
#include <vulkan/vulkan.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <iomanip>

using namespace std;

#define GETSET(type, var) \
	protected: type m_##var; \
	public: type Get##var() { return m_##var; } \
	void Set##var(type val) { m_##var = val; } \
	type& GetRef##var() { return m_##var; } \
	void Set##var(type& val) { m_##var = val; }

#define UNIMPLEMENTED_INTEFACE { printf("\n Attempting to use an unimplemented default interface: %s\n", __FUNCTION__); assert(0); }

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

struct VulkanBuffer
{
	VkBuffer				m_Buffer;			// Buffer resource object
	uint64_t				m_DataSize;			// Actual data size request for, use m_MemRqrmnt.size for actual backing size
	VkDeviceMemory			m_Memory;			// Buffer resourece object's allocated device memory
	VkMemoryRequirements	m_MemRqrmnt;		// Memory requirement for the allocation buffer, useful in mapping/unmapping
	VkMemoryPropertyFlags	m_MemoryFlags;		// Memory properties flags
};

struct VulkanImage
{
	VkImage					m_Image;
	VkDeviceMemory			m_Memory;
	VkMemoryRequirements	m_MemRqrmnt;	// Memory requirement for the allocation buffer, useful in mapping/unmapping
};

class VulkanHelper
{
public:
	VulkanHelper();
	~VulkanHelper();

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
	static void CreateBuffer(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, const void * vertexData, uint32_t dataSize, VulkanBuffer& p_VulkanBuffer, VkBufferCreateInfo* p_pBufInfo = NULL); // Please use this Create Buffer currently begin used

    // Chapter 3
    static void CreateImage(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, VkMemoryPropertyFlags imageMemProp, VkImageCreateInfo* pImageInfo, VkImage* pTextureImage, VkDeviceMemory* pTextureImageMemory);
    static VkImageView CreateImageView(const VkDevice device, VkImage image, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    static void UpdateMemory(const VkDevice device, VkDeviceMemory deviceMem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, const void* pData);
    //@todo Remove this method once SetImageLayout is generalzied to handle all layout transition
    static bool SetImageLayoutEx(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandBuffer& commandBuffer);

	void* map(uint32_t memFlags, const size_t size, const size_t offset);

public:
	// Layer property list containing Layers and respective extensions
	static std::vector<LayerProperties> m_LayerPropertyList;
};
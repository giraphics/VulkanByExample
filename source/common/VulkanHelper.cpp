#include "VulkanHelper.h"

std::vector<LayerProperties> VulkanHelper::m_LayerPropertyList = {};
VulkanHelper::VulkanHelper()
{
}

VulkanHelper::~VulkanHelper()
{
}

// Helper method to log error messages
void VulkanHelper::LogError(string text)
{
	string outputText;
	outputText = "Error: " + text;
	cout << outputText;
}

VkResult VulkanHelper::GetInstanceLayerExtensionProperties()
{
	uint32_t						instanceLayerCount;		// Stores number of layers supported by instance
	std::vector<VkLayerProperties>	layerProperties;		// Vector to store layer properties
	VkResult						result;					// Variable to check Vulkan API result status

	m_LayerPropertyList.clear();

	// Query all the layers
	result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);
	layerProperties.resize(instanceLayerCount);
	result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());

	// Query all the extensions for each layer and store it.
	std::cout << "\nInstanced Layers" << std::endl;
	std::cout << "===================" << std::endl;
	for (auto globalLayerProp : layerProperties) {
		std::cout << "\n" << globalLayerProp.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.layerName << "\n";

		LayerProperties layerProps;
		layerProps.properties = globalLayerProp;

		// Get Instance level extensions for corresponding layer properties
		result = GetExtensionProperties(layerProps);
		if (result) continue;

		m_LayerPropertyList.push_back(layerProps);
		// Print extension name for each instance layer
		for (auto j : layerProps.extensions) {
			std::cout << "\t\t|\n\t\t|---[Layer Extension]--> " << j.extensionName << "\n";
		}
	}
	return result;
}

VkResult VulkanHelper::GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu)
{
	std::cout << "Device extensions" << std::endl;
	std::cout << "===================" << std::endl;
	VkResult result;
	std::vector<LayerProperties> instanceLayerProp = m_LayerPropertyList;
	for (auto globalLayerProp : instanceLayerProp) {
		LayerProperties layerProps;
		layerProps.properties = globalLayerProp.properties;

		if (result = GetExtensionProperties(layerProps, gpu))
			continue;

		std::cout << "\n" << globalLayerProp.properties.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.properties.layerName << "\n";
		m_LayerPropertyList.push_back(layerProps);

		if (layerProps.extensions.size()) {
			for (auto j : layerProps.extensions) {
				std::cout << "\t\t|\n\t\t|---[Device Extesion]--> " << j.extensionName << "\n";
			}
		}
		else {
			std::cout << "\t\t|\n\t\t|---[Device Extesion]--> No extension found \n";
		}
	}
	return result;
}

// This function retrieves extension and its properties at instance 
// and device level. Pass a valid physical device
// pointer to retrieve device level extensions, otherwise
// use NULL to retrieve extension specific to instance level.
VkResult VulkanHelper::GetExtensionProperties(LayerProperties &layerProps, VkPhysicalDevice gpu)
{
	uint32_t	extensionCount;								 // Stores number of extension per layer
	VkResult	result;										 // Variable to check Vulkan API result status
	char*		layerName = layerProps.properties.layerName; // Name of the layer 

	do {
		// Get the total number of extension in this layer
		if (gpu)
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, NULL);
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, NULL);

		if (result || extensionCount == 0) continue;

		layerProps.extensions.resize(extensionCount);

		// Gather all extension properties 
		if (gpu)
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, layerProps.extensions.data());
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.extensions.data());
	} while (result == VK_INCOMPLETE);

	return result;
}

// Returns the best surface format to create the swap chain
VkSurfaceFormatKHR VulkanHelper::SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Choose the first format as default
	VkSurfaceFormatKHR bestFormat = availableFormats[0];

	// When no formats is available then choose the following format
	if (availableFormats.size() == 1 && bestFormat.format == VK_FORMAT_UNDEFINED)
	{
		bestFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		bestFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	else
	{
		for (unsigned int i = 0; i < availableFormats.size(); i++)
		{
			// Choose a preferred format
			if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
				availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				bestFormat = availableFormats[i];
				break;
			}
		}
	}

	return bestFormat;
}

// Returns the best present mode to create the swap chain
VkPresentModeKHR VulkanHelper::SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	// By default set to VK_PRESENT_MODE_FIFO_KHR where the presentation engine 
	// waits for the next vertical blanking period to update the current image
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (uint32_t i = 0; i < availablePresentModes.size(); i++)
	{
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			// Set the mode to VK_PRESENT_MODE_MAILBOX_KHR where the presentation engine 
			// waits for the next vertical blanking period to update the current image.
			// The image waiting to be displayed may get overwritten.
			// This mode do not cause image tearing
			bestMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}

		if (availablePresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			// Set the mode to VK_PRESENT_MODE_IMMEDIATE_KHR where the presentation engine 
			// will not wait for display vertical blank interval
			// This mode may cause image tearing
			bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	return bestMode;
}

// Returns the best swap chain extent to create the swap chain
VkExtent2D VulkanHelper::SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D& windowDim)
{
	VkExtent2D swapChainExtent = capabilities.currentExtent;

	// If either the width or height is -1 then set to swap chain extent to window dimension
	// otherwise choose the current extent from device capabilities
	if (capabilities.currentExtent.width == (uint32_t)-1 ||
	capabilities.currentExtent.height == (uint32_t)-1)
	{
		swapChainExtent.width = windowDim.width;
		swapChainExtent.height = windowDim.height;
	}

	return (swapChainExtent);
}

VkShaderModule VulkanHelper::CreateShader(VkDevice device, const std::string& filename)
{
	VkShaderModule shaderModule = 0;
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (file.is_open())
	{
		// Read the shader file
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> shaderCode(fileSize);

		file.seekg(0);
		file.read(shaderCode.data(), fileSize);
		file.close();

		// Now create the shader module
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);

		if (result != VK_SUCCESS)
		{
			LogError("Failed to create shader!");
			assert(false);
		}
	}
	else
	{
		LogError("Failed to open file!");
		assert(false);
	}

	return shaderModule;
}

bool VulkanHelper::MemoryTypeFromProperties(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, 
											VkFlags requirementsMask, uint32_t *typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

void VulkanHelper::CreateCommandPool(const VkDevice& device, VkCommandPool& cmdPool, const PhysicalDeviceInfo& deviceInfo, const VkCommandPoolCreateInfo* commandPoolInfo)
{
	VkResult vkResult;
	if (commandPoolInfo)
	{
		vkResult = vkCreateCommandPool(device, commandPoolInfo, nullptr, &cmdPool);
		assert(!vkResult);
	}
	else
	{
		// Create the command buffer pool object
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = 0;
		poolInfo.queueFamilyIndex = deviceInfo.graphicsFamilyIndex;
		vkResult = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
		assert(!vkResult);
	}
}

void VulkanHelper::AllocateCommandBuffer(const VkDevice device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
	// Dependency on the intialize SwapChain Extensions and initialize CommandPool
	VkResult result;

	// If command information is available use it as it is.
	if (commandBufferInfo) {
		result = vkAllocateCommandBuffers(device, commandBufferInfo, cmdBuf);
		assert(!result);
		return;
	}

	// Default implementation, create the command buffer
	// allocation info and use the supplied parameter into it
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.pNext = NULL;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);;

	result = vkAllocateCommandBuffers(device, &cmdInfo, cmdBuf);
	assert(!result);
}

void VulkanHelper::BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
	// Dependency on  the initialieCommandBuffer()
	VkResult  result;
	// If the user has specified the custom command buffer use it
	if (inCmdBufInfo) {
		result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
		assert(result == VK_SUCCESS);
		return;
	}

	// Otherwise, use the default implementation.
	VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
	cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmdBufInheritInfo.pNext = NULL;
	cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
	cmdBufInheritInfo.subpass = 0;
	cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
	cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
	cmdBufInheritInfo.queryFlags = 0;
	cmdBufInheritInfo.pipelineStatistics = 0;

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;
	cmdBufInfo.flags = 0;
	cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;

	result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

	assert(result == VK_SUCCESS);
}

void VulkanHelper::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkResult  result;
	result = vkEndCommandBuffer(commandBuffer);
	assert(result == VK_SUCCESS);
}

void VulkanHelper::SubmitCommandBuffer(const VkQueue& queue, const VkCommandBuffer commandBuffer, const VkSubmitInfo* inSubmitInfo, const VkFence& fence)
{
	VkResult result;

	// If Subimt information is avialable use it as it is, this assumes that 
	// the commands are already specified in the structure, hence ignore command buffer 
	if (inSubmitInfo) {
		result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
		assert(!result);

		result = vkQueueWaitIdle(queue);
		assert(!result);
		return;
	}

	// Otherwise, create the submit information with specified buffer commands
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = (uint32_t)sizeof(commandBuffer) / sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;

	result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	assert(!result);

	result = vkQueueWaitIdle(queue);
	assert(!result);
}

void VulkanHelper::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkAccessFlagBits srcAccessMask, const VkCommandBuffer& commandBuffer)
{
	// Dependency on commandBuffer
	assert(commandBuffer != VK_NULL_HANDLE);

	VkImageMemoryBarrier imgMemoryBarrier = {};
	imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemoryBarrier.pNext = NULL;
	imgMemoryBarrier.srcAccessMask = srcAccessMask;
	imgMemoryBarrier.dstAccessMask = 0;
	imgMemoryBarrier.oldLayout = oldImageLayout;
	imgMemoryBarrier.newLayout = newImageLayout;
	imgMemoryBarrier.image = image;
	imgMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imgMemoryBarrier.subresourceRange.levelCount = 1;
	imgMemoryBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	switch (newImageLayout)
	{
		// Ensure that anything that was copying from this image has completed
		// An image in this layout can only be used as the destination operand of the commands
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

		// Ensure any Copy or CPU writes to image are flushed
		// An image in this layout can only be used as a read-only shader resource
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

		// An image in this layout can only be used as a framebuffer color attachment
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

		// An image in this layout can only be used as a framebuffer depth/stencil attachment
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
}

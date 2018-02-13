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
    assert(0);
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
    std::cout << "\n" << std::left << std::setw(40) << "Instance Layer Name" << " | " << std::setw(40) << "Instance Layer Description" << " | " << "Available Extensions\n";
    std::cout << "-------------------------------------------------------------------------------------------------------------------" << std::endl;
	for (auto globalLayerProp : layerProperties) {
		LayerProperties layerProps;
		layerProps.properties = globalLayerProp;

		// Get Instance level extensions for corresponding layer properties
		result = GetExtensionProperties(layerProps);
		if (result) continue;

		m_LayerPropertyList.push_back(layerProps);

        // Get extension name for each instance layer
        string extensions = "[ ";
        if (layerProps.extensions.size()) {
            for (auto j : layerProps.extensions) {
                extensions += j.extensionName;
                extensions += " ";
            }
        }
        else {
            extensions = "None";
        }

        extensions += " ]";

        // Print Instance Layer info
        std::cout << "\n" << std::left << std::setw(40) << globalLayerProp.layerName << " | " << std::setw(40) << globalLayerProp.description << " | " << extensions;
	}
    std::cout << "\n-------------------------------------------------------------------------------------------------------------------" << std::endl;
	return result;
}

VkResult VulkanHelper::GetDeviceLayerExtensionProperties(VkPhysicalDevice gpu)
{
	std::cout << "\n\nDevice layer extensions" << std::endl;
	std::cout << "==========================" << std::endl;
	VkResult result;
	std::vector<LayerProperties> instanceLayerProp = m_LayerPropertyList;

    std::cout << "\n" << std::left << std::setw(40) << "Device Layer Name" << " | " << std::setw(40) << "Device Layer Description" << " | " << "Available Extensions\n";
    std::cout << "-------------------------------------------------------------------------------------------------------------------" << std::endl;

	for (auto globalLayerProp : instanceLayerProp) {
		LayerProperties layerProps;
		layerProps.properties = globalLayerProp.properties;

		if (result = GetExtensionProperties(layerProps, gpu))
			continue;

        m_LayerPropertyList.push_back(layerProps);

        string extensions = "[ ";
		if (layerProps.extensions.size()) {
			for (auto j : layerProps.extensions) {
                extensions += j.extensionName;
                extensions += " ";
			}
		}
		else {
            extensions += "None";
		}
        extensions += " ]";

        // Print Device Layer info
        std::cout << "\n" << std::left << std::setw(40) << globalLayerProp.properties.layerName << " | " << std::setw(40) << globalLayerProp.properties.description << " | " << extensions;
	}
    std::cout << "\n-------------------------------------------------------------------------------------------------------------------" << std::endl;
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
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
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

	result = vkQueueWaitIdle(queue); // May be put this in a flag to wait for vkQueue idle
	assert(!result);
}

bool VulkanHelper::SetImageLayoutEx(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkCommandBuffer& commandBuffer)
{
    bool result = true;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        result = false;
        VulkanHelper::LogError("Unsupported layout transition!");
    }

    if (result)
    {
        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    return (result);
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

VkResult VulkanHelper::createBuffer(const VkDevice device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties deviceMemProp, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
{
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = size;
	bufferCreateInfo.flags = 0;

	VkResult result;
	result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer);

	vkGetBufferMemoryRequirements(device, *buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	MemoryTypeFromProperties(deviceMemProp, memReqs.memoryTypeBits, memoryPropertyFlags, &memAlloc.memoryTypeIndex);

	result = vkAllocateMemory(device, &memAlloc, nullptr, memory);
	if (data != nullptr)
	{
		void *mapped;
		result = vkMapMemory(device, *memory, 0, size, 0, &mapped);
		memcpy(mapped, data, size);
		vkUnmapMemory(device, *memory);
	}
	result = vkBindBufferMemory(device, *buffer, *memory, 0);

	return result;
}

VkResult VulkanHelper::createBuffer(const VkDevice logicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties deviceMemProp, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
{
	// Create the buffer handle
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = size;
	bufferCreateInfo.flags = 0;

	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult result;
	result = vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer);

	// Create the memory backing up the buffer handle
	VkMemoryRequirements memReqs;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = NULL;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;

	vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Find a memory type index that fits the properties of the buffer
	//memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
	MemoryTypeFromProperties(deviceMemProp, memReqs.memoryTypeBits, memoryPropertyFlags, &memAlloc.memoryTypeIndex);
	
	result = vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory);

	// If a pointer to the buffer data has been passed, map the buffer and copy over the data
	if (data != nullptr)
	{
		void *mapped;
		result = vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped);
		memcpy(mapped, data, size);
		vkUnmapMemory(logicalDevice, *memory);
	}

	// Attach the memory to the buffer object
	result = vkBindBufferMemory(logicalDevice, *buffer, *memory, 0);

	return result;
}

void VulkanHelper::CreateBuffer(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp, const void * vertexData, uint32_t dataSize, uint32_t dataStride, VkBuffer* buffer, VkDeviceMemory* memory)
{
	VkResult  result;
	bool  pass;

	// 1. Create the Buffer resource
	/*******************************/
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	result = vkCreateBuffer(device, &bufInfo, NULL, buffer);
	assert(result == VK_SUCCESS);

	// 2. Get memory specific requirements
	/**************************************************************/

	// 2a. Get the Buffer resource requirements
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(device, *buffer, &memRqrmnt);

	// 2b. Get the compatible type of memory
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	pass = VulkanHelper::MemoryTypeFromProperties(deviceMemProp, memRqrmnt.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass);

	// 3. Allocate the physical backing
	/******************************************************/
	result = vkAllocateMemory(device, &allocInfo, NULL, memory);
	assert(result == VK_SUCCESS);

	// 4. Copy data into buffer
	/**************************/
	// 4a. Map the physical device memory region to the host 
	uint8_t *pData;
	result = vkMapMemory(device, *memory, 0, memRqrmnt.size, 0, (void **)&pData);
	assert(result == VK_SUCCESS);

	// 4b. Copy the data in the mapped memory
	memcpy(pData, vertexData, dataSize);

	// 4c. Unmap the device memory
	vkUnmapMemory(device, *memory);

	// 5. Bind the allocated buffer resource to the device memory
	result = vkBindBufferMemory(device, *buffer, *memory, 0);
	assert(result == VK_SUCCESS);
}

void VulkanHelper::CreateImage(const VkDevice device, VkPhysicalDeviceMemoryProperties deviceMemProp,
    VkMemoryPropertyFlags imageMemProp, VkImageCreateInfo* pImageInfo, VkImage* pImage, VkDeviceMemory* pImageMemory)
{
    VkResult result = vkCreateImage(device, pImageInfo, nullptr, pImage);
    assert(result == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VulkanHelper::MemoryTypeFromProperties(deviceMemProp, memRequirements.memoryTypeBits,
        imageMemProp, &allocInfo.memoryTypeIndex);

    result = vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory);
    assert(result == VK_SUCCESS);

    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

void VulkanHelper::UpdateMemory(const VkDevice device, VkDeviceMemory deviceMem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, const void* pData)
{
    assert(pData);

    void* pHostMem = nullptr;
    vkMapMemory(device, deviceMem, offset, size, flags, &pHostMem);
    assert(pHostMem);

    if (pHostMem)
    {
        memcpy(pHostMem, pData, (size_t)size);
    }
    vkUnmapMemory(device, deviceMem);
}


VkImageView VulkanHelper::CreateImageView(const VkDevice device, VkImage image, VkImageViewType type /*= VK_IMAGE_VIEW_TYPE_2D*/, VkFormat format /*= VK_FORMAT_R8G8B8A8_UNORM*/)
{
    VkImageView imageView = nullptr;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = type; //  VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);

    assert(result == VK_SUCCESS);

    return imageView;
}

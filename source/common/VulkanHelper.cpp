#include "VulkanHelper.h"

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

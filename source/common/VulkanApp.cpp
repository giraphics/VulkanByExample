#include "VulkanApp.h"
#include <cstddef>

extern VulkanApp* g_pApp;

VulkanApp::VulkanApp()
{
    m_pWindow = nullptr;

    // Default application name
    SetApplicationName("Vulkan Application");
    // Default application window dimension
    SetWindowDimension(640, 480);

    // Vulkan specific objects
    m_hInstance = VK_NULL_HANDLE;
    m_hSurface = VK_NULL_HANDLE;
    m_hPhysicalDevice = VK_NULL_HANDLE;
    m_hDevice = VK_NULL_HANDLE;
    m_hGraphicsQueue = VK_NULL_HANDLE;
    m_hPresentQueue = VK_NULL_HANDLE;
    m_hSwapChain = VK_NULL_HANDLE;
    m_hSwapChainImageFormat = VK_FORMAT_UNDEFINED;
    m_hRenderPass = VK_NULL_HANDLE;
    m_hCommandPool = VK_NULL_HANDLE;
    m_hRenderReadySemaphore = VK_NULL_HANDLE;
    m_hPresentReadySemaphore = VK_NULL_HANDLE;

    memset(&m_swapChainExtent, 0, sizeof(m_swapChainExtent));

    m_hSwapChainImageViewList.clear();
    m_hSwapChainFramebufferList.clear();
    m_hSwapChainImageList.clear();
    m_hCommandBufferList.clear();
    
    m_activeSwapChainImageIndex = 0;

    // VK_KHR_SWAPCHAIN_EXTENSION_NAME extension support is required to
    // use the selected vulkan device to present rendered output to a window
    m_requiredDeviceExtensionList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

bool VulkanApp::Init()
{
    bool result = true;
    
    // Call the derived class to configure
    result = Configure();

    if (result)
    {
        // Create the main window
        result = CreateDisplayWindow();
    }

    if (result)
    {
        // Initialize Vulkan
        result = InitializeVulkan();
    }

    // Call the derived class to setup applications specific objects
    result = Setup();

    return (result);
}

// Helper method to log error messages
void VulkanApp::LogError(string text)
{
    string outputText;
    outputText = "Error: " + text;
    cout << outputText;
}

bool VulkanApp::Run()
{
    bool result = true;

    // Check if applicaiton window is closed
    while (!glfwWindowShouldClose(m_pWindow) && result)
    {
        // Call the derived class object to update 
        // application specific updates
        result = Update();

        // Call the base class or derived class object 
        // to render application specific content
        if (result) { result = Render(); }
        
        // Present the output to back buffer
        if (result) { result = Present(); }

        if (result)
        {
            // Swap front and back buffers
            glfwSwapBuffers(m_pWindow);

            // Poll and process events
            glfwPollEvents();
        }
    }

    return (result);
}

bool VulkanApp::CreateDisplayWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the application window
    m_pWindow = glfwCreateWindow(m_windowDim.width, m_windowDim.height, 
                                 m_appName.c_str(),nullptr,nullptr);

    return (m_pWindow ? true : false);
}

bool VulkanApp::InitializeVulkan()
{
    bool result = false;

    // Check if Vulkan loader is supported in GLFW
    result = (glfwVulkanSupported() == GLFW_TRUE);

    // Create Vulkan Instance
    if (result) { result = CreateVulkanInstance(); }
    // Create Vulkan Device
    if (result) { result = CreateVulkanDevice(); }
    // Create Swap chain
    if (result) { result = CreateSwapChain(); }
    // Create Render Pass
    if (result) { result = CreateRenderPass(); }
    // Create Frame buffer
    if (result) { result = CreateFramebuffers(); }
    // Create command buffers
    if (result) { result = CreateCommandBuffers(); }
    // Create semaphores
    if (result) { result = CreateSemaphores(); }

    return (result);
}


bool VulkanApp::CreateVulkanInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = m_appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VulkanApp";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;

    // Get the required vulkan instance extensions from GLFW
    const char** ppGlfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(ppGlfwExtensions[i]);
    }

    // Fill in the required createInfo structure
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Create the Vulkan Instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_hInstance);

    if (result == VK_SUCCESS)
    {
        // Create the Vulkan surface for the application window
        result = glfwCreateWindowSurface(m_hInstance, m_pWindow, nullptr, &m_hSurface);
    }

    if (result != VK_SUCCESS)
    {
        LogError("Error creating Vulkan Instance");
    }

    return (result == VK_SUCCESS);
}

bool VulkanApp::CreateVulkanDevice()
{
    // Step 1: Select the physical device suitable for Vulkan
    bool result = SelectPhysicalDevice();

    if (result)
    {
        // Step 2 & 3: Create the Vulkan device & get pointer 
        // to graphics & present queue
        result = CreateDeviceAndQueueObjects();
    }

    return (result);
}

// Returns detailed information for the given physical device handle
void VulkanApp::GetPhysicalDeviceInfo(VkPhysicalDevice device, PhysicalDeviceInfo* pDeviceInfo)
{
    vkGetPhysicalDeviceProperties(device, &pDeviceInfo->prop);
    vkGetPhysicalDeviceFeatures(device, &pDeviceInfo->features);
    vkGetPhysicalDeviceMemoryProperties(device, &pDeviceInfo->memProp);

    // Get count of queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    pDeviceInfo->familyPropList.resize(queueFamilyCount);

    // Get the list of queue family properties
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, pDeviceInfo->familyPropList.data());

    // Get the family queue index for graphics & present
    pDeviceInfo->graphicsFamilyIndex = -1;
    pDeviceInfo->presentFamilyIndex = -1;
    // For each queue family property in the list
    for (unsigned int i = 0; i<pDeviceInfo->familyPropList.size(); i++)
    {
        if (pDeviceInfo->familyPropList[i].queueCount > 0)
        {
            // Check if it supports graphics queue
            if (pDeviceInfo->familyPropList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // Device queue supports graphics queue
                // Store the graphics queue index
                pDeviceInfo->graphicsFamilyIndex = i;
            }


            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_hSurface, &presentSupport);

            // Check if it supports present queue
            if (presentSupport)
            {
                // Device queue family supports present
                // Store the present queue index
                pDeviceInfo->presentFamilyIndex = i;
            }

            // Check if device queue family supports both 
            // graphics & present queue
            if (pDeviceInfo->graphicsFamilyIndex >= 0 &&
                pDeviceInfo->presentFamilyIndex >= 0)
            {
                break;
            }
        }
    }

    // Get device extension count
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    pDeviceInfo->extensionList.resize(extensionCount);
    // Get device extensions
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, pDeviceInfo->extensionList.data());

    // Get surface capabilities of the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_hSurface, &pDeviceInfo->capabilities);

    // Get the surface format count
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_hSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        pDeviceInfo->formatList.resize(formatCount);
        // Get the surface formats
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_hSurface, &formatCount, pDeviceInfo->formatList.data());
    }

    // Get the surface present mode count
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_hSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        pDeviceInfo->presentModeList.resize(presentModeCount);
        // Get the surface present modes
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_hSurface, &presentModeCount, pDeviceInfo->presentModeList.data());
    }
}


// This method checks if the given device 
// a) Contains graphics and present queue families and
// b) Supports the extensions required and
// c) Supports swap chain requirements
// The method returns true when all 3 conditions are met
// or false otherwise
bool VulkanApp::IsDeviceSuitable(PhysicalDeviceInfo deviceInfo)
{
    bool result = false;

    // Step 1: Check if graphics & present queue families are
    // are supported
    if (deviceInfo.graphicsFamilyIndex >= 0 &&
        deviceInfo.presentFamilyIndex >= 0)
    {
        // Step 2: Check if required device extensions are supported
        std::set<std::string> requiredExtensions(m_requiredDeviceExtensionList.begin(), m_requiredDeviceExtensionList.end());

        uint32_t deviceExtensionCount = static_cast<uint32_t>(deviceInfo.extensionList.size());

        for (uint32_t i = 0; i < deviceExtensionCount && !result; i++)
        {
            requiredExtensions.erase(deviceInfo.extensionList[i].extensionName);
            // Exit loop when all required extensions are found
            result = requiredExtensions.empty();
        }

        if (result)
        {
            // Step 3: Check if swap chain requirements are met
            result = (!deviceInfo.formatList.empty()) && (!deviceInfo.presentModeList.empty());
        }
    }

    return (result);
}

// This method finds the appropriate physical device (GPU)
// suitable for rendering using Vulkan. The method returns 
// true on success, false otherwise
bool VulkanApp::SelectPhysicalDevice()
{
    bool result = true;
    uint32_t deviceCount = 0;

    // Get the number of GPUs that supports Vulkan
    vkEnumeratePhysicalDevices(m_hInstance, &deviceCount, nullptr);

    if (deviceCount >= 0)
    {
        std::vector<VkPhysicalDevice> deviceList(deviceCount);

        // Get the GPU devices that are accessible to a Vulkan
        vkEnumeratePhysicalDevices(m_hInstance, &deviceCount, deviceList.data());

        // Select the device that is suitable for rendering
        for (uint32_t i = 0; i<deviceCount; i++)
        {
            // Get all required device information to select the device
            PhysicalDeviceInfo deviceInfo = {};
            GetPhysicalDeviceInfo(deviceList[i], &deviceInfo);

            // Check if this device meets our application criteria
            result = IsDeviceSuitable(deviceInfo);

            if (result)
            {
                // Store the pointer to the selected physical device (GPU)
                m_hPhysicalDevice = deviceList[i];
                m_physicalDeviceInfo = deviceInfo;
                break;
            }
        }

        if (m_hPhysicalDevice == VK_NULL_HANDLE)
        {
            LogError("Could not find a suitable GPU");
            result = false;
        }
    }
    else
    {
        LogError("Could not find a GPU that supports Vulkan");
        result = false;
    }

    return (result);
}

bool VulkanApp::CreateDeviceAndQueueObjects()
{
    bool result = true;

    // Fill-in the queue info structures. Note that when the graphics family queue index
    // is different from the present family index we need to pass in 2 seperate
    // queue create info structures
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = { m_physicalDeviceInfo.graphicsFamilyIndex,
        m_physicalDeviceInfo.presentFamilyIndex };

    float queuePriority[] = { 1.0f };
    for (std::set<int>::iterator it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); ++it)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = *it;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Now fill-in the VkDeviceCreateInfo structure
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredDeviceExtensionList.size());
    createInfo.ppEnabledExtensionNames = m_requiredDeviceExtensionList.data();
    createInfo.enabledLayerCount = 0;

    // Create the Vulkan device
    if (vkCreateDevice(m_hPhysicalDevice, &createInfo, nullptr, &m_hDevice) == VK_SUCCESS)
    {
        // Get the pointer to the graphics queue and present queue
        // This is used to submit commands to the GPU
        vkGetDeviceQueue(m_hDevice, m_physicalDeviceInfo.graphicsFamilyIndex, 0, &m_hGraphicsQueue);
        vkGetDeviceQueue(m_hDevice, m_physicalDeviceInfo.presentFamilyIndex, 0, &m_hPresentQueue);
    }
    else
    {
        LogError("Vulkan Device creation failed");
        result = false;
    }

    return (result);
}

bool VulkanApp::CreateSwapChain()
{
    bool result = true;

    VkSurfaceFormatKHR surfaceFormat = SelectBestSurfaceFormat(m_physicalDeviceInfo.formatList);
    VkPresentModeKHR presentMode = SelectBestPresentMode(m_physicalDeviceInfo.presentModeList);
    VkExtent2D extent = SelectBestExtent(m_physicalDeviceInfo.capabilities);
    uint32_t imageCount = m_physicalDeviceInfo.capabilities.minImageCount + 1;

    // Validate the image count bounds
    if (m_physicalDeviceInfo.capabilities.maxImageCount > 0 &&
        imageCount > m_physicalDeviceInfo.capabilities.maxImageCount)
    {
        imageCount = m_physicalDeviceInfo.capabilities.maxImageCount;
    }

    // Fill in VkSwapchainCreateInfoKHR to create the swap chain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_hSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { (uint32_t)m_physicalDeviceInfo.graphicsFamilyIndex, (uint32_t)m_physicalDeviceInfo.presentFamilyIndex };

    if (m_physicalDeviceInfo.graphicsFamilyIndex != m_physicalDeviceInfo.presentFamilyIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = m_physicalDeviceInfo.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create the Swap chain
    if (vkCreateSwapchainKHR(m_hDevice, &createInfo, nullptr, &m_hSwapChain) != VK_SUCCESS)
    {
        LogError("vkCreateSwapchainKHR() failed!");
        result = false;
    }
    else
    {
        // Get the count of swap chain images
        vkGetSwapchainImagesKHR(m_hDevice, m_hSwapChain, &imageCount, nullptr);
        m_hSwapChainImageList.resize(imageCount);
        // Get the swap chain images
        vkGetSwapchainImagesKHR(m_hDevice, m_hSwapChain, &imageCount, m_hSwapChainImageList.data());

        m_hSwapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        m_hSwapChainImageViewList.resize(m_hSwapChainImageList.size());

        // For every image in the swap chain
        for (size_t i = 0; i < m_hSwapChainImageList.size(); i++)
        {
            // Fill in VkImageViewCreateInfo to create the image view
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_hSwapChainImageList[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_hSwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            // Create the swap chain image view
            if (vkCreateImageView(m_hDevice, &createInfo, nullptr, &m_hSwapChainImageViewList[i]) != VK_SUCCESS)
            {
                LogError("vkCreateImageView() failed!");
                result = false;
            }
        }
    }

    return (result);
}

// Returns the best surface format to create the swap chain
VkSurfaceFormatKHR VulkanApp::SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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
VkPresentModeKHR VulkanApp::SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
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
VkExtent2D VulkanApp::SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    VkExtent2D swapChainExtent = capabilities.currentExtent;

    // If either the width or height is -1 then set to swap chain extent to window dimension
    // otherwise choose the current extent from device capabilities
/*    if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max() ||
        capabilities.currentExtent.height == std::numeric_limits<uint32_t>::max())*/
    {
        swapChainExtent.width =  m_windowDim.width;
        swapChainExtent.height = m_windowDim.height;
    }

    return (swapChainExtent);
}

bool VulkanApp::CreateRenderPass()
{
    bool result = true;

    // Fill in the color attachment
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_hSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Fill in the color attachment reference
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Fill in the sub pass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Fill in the sub pass dependency
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Now fill in the render pass info with all the above details
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Create the render pass
    if (vkCreateRenderPass(m_hDevice, &renderPassInfo, nullptr, &m_hRenderPass) != VK_SUCCESS)
    {
        LogError("vkCreateRenderPass() failed to create render pass!");
        result = false;
    }

    return (result);
}

VkShaderModule VulkanApp::CreateShader(const std::string& filename)
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

        VkResult result = vkCreateShaderModule(m_hDevice, &createInfo, nullptr, &shaderModule);

        if (result != VK_SUCCESS)
        {
            LogError("Failed to create shader!");
        }
    }
    else
    {
        LogError("Failed to open file!");
    }

    return shaderModule;
}

bool VulkanApp::CreateFramebuffers()
{
    bool result = true;

    // Resize the list based on swap chain image view count
    m_hSwapChainFramebufferList.resize(m_hSwapChainImageViewList.size());

    // For each item in swap chain image view list
    for (size_t i = 0; i < m_hSwapChainImageViewList.size(); i++)
    {
        // Setup VkFramebufferCreateInfo to create frame buffer object
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_hRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_hSwapChainImageViewList[i];
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        // Create frame buffer object
        VkResult vkResult = vkCreateFramebuffer(m_hDevice, &framebufferInfo, nullptr, &m_hSwapChainFramebufferList[i]);
        if (vkResult != VK_SUCCESS)
        {
            LogError("vkCreateFramebuffer() failed!");
            result = false;
        }
    }

    return (result);
}

bool VulkanApp::CreateCommandBuffers()
{
    bool result = true;

    // Create the command buffer pool object
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.queueFamilyIndex = m_physicalDeviceInfo.graphicsFamilyIndex;

    VkResult vkResult = vkCreateCommandPool(m_hDevice, &poolInfo, nullptr, &m_hCommandPool);

    if (vkResult == VK_SUCCESS)
    {
        m_hCommandBufferList.resize(m_hSwapChainFramebufferList.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_hCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_hCommandBufferList.size();

        vkResult = vkAllocateCommandBuffers(m_hDevice, &allocInfo, m_hCommandBufferList.data());
        if (vkResult != VK_SUCCESS)
        {
            LogError("vkAllocateCommandBuffers() failed!");
            result = false;
        }
    }
    else
    {
        LogError("vkCreateCommandPool() failed!");
        result = false;
    }

    return (result);
}

bool VulkanApp::CreateSemaphores()
{
    bool result = true;
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    // Create 2 semaphore objects
    // One to signal when the image is ready to render
    // Another one to signal when the image is ready to present
    if (vkCreateSemaphore(m_hDevice, &semaphoreInfo, nullptr, &m_hRenderReadySemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_hDevice, &semaphoreInfo, nullptr, &m_hPresentReadySemaphore) != VK_SUCCESS)
    {
        LogError("vkCreateSemaphore() failed");
        result = false;
    }
    return (result);
}


bool VulkanApp::Render()
{
    m_activeSwapChainImageIndex = 0;

	const uint64_t timeOut = 999;// std::numeric_limits<uint64_t>::max();
    vkAcquireNextImageKHR(m_hDevice, m_hSwapChain, timeOut, m_hRenderReadySemaphore, VK_NULL_HANDLE, &m_activeSwapChainImageIndex);

    VkPipelineStageFlags waitDstStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    // Setup VkSubmitInfo to submit graphics queue workload to GPU
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_hRenderReadySemaphore;
    submitInfo.pWaitDstStageMask = waitDstStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_hCommandBufferList[m_activeSwapChainImageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_hPresentReadySemaphore;

    VkResult vkResult = vkQueueSubmit(m_hGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    return (vkResult == VK_SUCCESS);
}

bool VulkanApp::Present()
{
    bool result;

    // Setup VkPresentInfoKHR to execute the present queue
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_hPresentReadySemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_hSwapChain;
    presentInfo.pImageIndices = &m_activeSwapChainImageIndex;

    result = vkQueuePresentKHR(m_hPresentQueue, &presentInfo);

    if (result == VK_SUCCESS)
    {
        // Wait for present queue to become idle
        result = vkQueueWaitIdle(m_hPresentQueue);
    }

    return (result == VK_SUCCESS);
}

void VulkanApp::CleanupVulkan()
{
    vkDestroySemaphore(m_hDevice, m_hPresentReadySemaphore, nullptr);
    vkDestroySemaphore(m_hDevice, m_hRenderReadySemaphore, nullptr);
    vkDestroyCommandPool(m_hDevice, m_hCommandPool, nullptr);

    for (size_t i = 0; i < m_hSwapChainFramebufferList.size(); i++)
    {
        vkDestroyFramebuffer(m_hDevice, m_hSwapChainFramebufferList[i], nullptr);
    }

    vkDestroyRenderPass(m_hDevice, m_hRenderPass, nullptr);

    for (size_t i = 0; i < m_hSwapChainImageViewList.size(); i++)
    {
        vkDestroyImageView(m_hDevice, m_hSwapChainImageViewList[i], nullptr);
    }

    vkDestroySwapchainKHR(m_hDevice, m_hSwapChain, nullptr);
    vkDestroyDevice(m_hDevice, nullptr);
    vkDestroySurfaceKHR(m_hInstance, m_hSurface, nullptr);
    vkDestroyInstance(m_hInstance, nullptr);
}

void VulkanApp::CoreCleanup()
{
    CleanupVulkan();
    glfwDestroyWindow(m_pWindow);
    m_pWindow = NULL;
    glfwTerminate();
}

bool VulkanApp::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    bool  result = true;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_hDevice, &bufferInfo, nullptr, &buffer) == VK_SUCCESS)
    {
        VkMemoryRequirements memRequirements = {};
        vkGetBufferMemoryRequirements(m_hDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = GetMemoryTypeIndex(properties, memRequirements.memoryTypeBits);

        if (vkAllocateMemory(m_hDevice, &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS)
        {
            vkBindBufferMemory(m_hDevice, buffer, bufferMemory, 0);
        }
        else
        {
            result = false;
            LogError("Failed to allocate buffer memory!");
        }
    }
    else
    {
        result = false;
        LogError("Failed to create buffer!");
    }
    
    return (result);
}

VkCommandBuffer VulkanApp::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_hCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_hDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApp::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_hGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_hGraphicsQueue);

    vkFreeCommandBuffers(m_hDevice, m_hCommandPool, 1, &commandBuffer);
}

uint32_t VulkanApp::GetMemoryTypeIndex(VkMemoryPropertyFlags properties, uint32_t memoryTypeBits)
{
    uint32_t memoryTypeIndex = -1;

    for (uint32_t i = 0; i < m_physicalDeviceInfo.memProp.memoryTypeCount && (memoryTypeIndex == -1); i++)
    {
        if ((memoryTypeBits & (1 << i)) && (m_physicalDeviceInfo.memProp.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
        }
    }

    if (memoryTypeIndex == -1)
    {
        LogError("Unable to find suitable memory type!");
    }

    return (memoryTypeIndex);
}

void VulkanApp::UpdateMemory(VkDeviceMemory deviceMem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, const void* pData)
{
    void* pHostMem;
    vkMapMemory(m_hDevice, deviceMem, offset, size, flags, &pHostMem);
    memcpy(pHostMem, pData, (size_t)size);
    vkUnmapMemory(m_hDevice, deviceMem);
}


bool VulkanApp::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    bool result = true;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_hDevice, &imageInfo, nullptr, &image) == VK_SUCCESS)
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_hDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = GetMemoryTypeIndex(properties, memRequirements.memoryTypeBits);

        if (vkAllocateMemory(m_hDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            LogError("vkAllocateMemory() failed!");
        }

        vkBindImageMemory(m_hDevice, image, imageMemory, 0);
    }
    else
    {
        LogError("vkCreateImage() failed!");
    }

    return (result);
}
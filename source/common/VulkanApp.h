#pragma once
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <QWindow>
#include <QTimer>

class Window;
using namespace std;
class Window : public QWindow
{
	Q_OBJECT

public:
	Window();
	~Window();

public slots:
	void Run();

protected:
	void resizeEvent(QResizeEvent* p_Event) override;

	// Refresh timer
	QTimer *renderTimer;
};
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

// Base class for Vulkan application
class VulkanApp
{
public:
    VulkanApp();
    virtual ~VulkanApp() {}

    // Core virtual methods used by derived classes
    virtual bool Configure() { return (true); }
    virtual bool Setup() { return (true); }
    virtual bool Update() { return (true); }
    virtual bool Render();
    virtual bool Present();
    virtual void Close() {}
    virtual void SetApplicationName(string name) { m_appName = name; }
    virtual void SetWindowDimension(int width, int height)
    {
        m_windowDim.width = width;
        m_windowDim.height = height;
    }

    // Methods used in main()
    bool Init();
    bool Run();
    void Cleanup()
    {
        Close();
        CoreCleanup();
    }

    // Helper methods
    VkDevice GetDevice() { return (m_hDevice); }
    void LogError(string text);

    uint32_t GetMemoryTypeIndex(VkMemoryPropertyFlags properties, uint32_t memoryTypeBits);
    bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    bool CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    void UpdateMemory(VkDeviceMemory deviceMem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, const void* pData);
	QWindow* GetApplicationWindow() const { return m_pWindow; }

private:
    bool CreateDisplayWindow();
    bool InitializeVulkan();
    void CleanupVulkan();
    void CoreCleanup();

    // Private helper methods
    bool CreateVulkanInstance();
    bool CreateVulkanDevice();
    bool CreateSwapChain();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreateCommandBuffers();
    bool CreateSemaphores();
    
    bool CreateDeviceAndQueueObjects();
    void GetPhysicalDeviceInfo(VkPhysicalDevice device, PhysicalDeviceInfo* pDeviceInfo);
    bool IsDeviceSuitable(PhysicalDeviceInfo deviceInfo);
    bool SelectPhysicalDevice();

    VkSurfaceFormatKHR SelectBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    VkExtent2D SelectBestExtent(const VkSurfaceCapabilitiesKHR& capabilities);

protected:
    Window*			m_pWindow;   // Application window object
	VkExtent2D      m_windowDim; // Application window dimension
    string          m_appName;   // Application name

    // Vulkan specific objects
    VkInstance      m_hInstance; // Vulkan instance object
    VkSurfaceKHR    m_hSurface;  // Vulkan window surface

    // Vulkan Device specific objects
    VkPhysicalDevice            m_hPhysicalDevice;
    PhysicalDeviceInfo          m_physicalDeviceInfo;
    VkDevice                    m_hDevice;
    std::vector<const char*>    m_requiredDeviceExtensionList;

    // Pointers to Graphics & Present queue
    VkQueue                     m_hGraphicsQueue;
    VkQueue                     m_hPresentQueue;

    // Swap chain specific objects
    VkSwapchainKHR              m_hSwapChain;
    std::vector<VkImage>        m_hSwapChainImageList;
    VkFormat                    m_hSwapChainImageFormat;
    VkExtent2D                  m_swapChainExtent;
    std::vector<VkImageView>    m_hSwapChainImageViewList;
    std::vector<VkFramebuffer>  m_hSwapChainFramebufferList;
    uint32_t                    m_activeSwapChainImageIndex;

    // Render Pass
    VkRenderPass                m_hRenderPass;

    // Command buffer related objects
    VkCommandPool                   m_hCommandPool;
    std::vector<VkCommandBuffer>    m_hCommandBufferList;
    VkSemaphore                     m_hRenderReadySemaphore;
    VkSemaphore                     m_hPresentReadySemaphore;

    VkShaderModule CreateShader(const std::string& filename);
};
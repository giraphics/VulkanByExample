#pragma once
#include "VulkanHelper.h"

#include <QWindow>
#include <QTimer>

class VulkanApp;

class Window : public QWindow
{
	Q_OBJECT

public:
	Window(VulkanApp* parent = NULL);
	~Window() { delete renderTimer; }

public slots:
	void Run();

private:
	QTimer* renderTimer;	// Refresh timer
	VulkanApp* m_VulkanApp; // Used to call run() by the timer
};

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

// Base class for Vulkan application
class VulkanApp
{
public:
    VulkanApp();
	virtual ~VulkanApp();

    void Initialize(); // Initialize the Vulkan application
    void Run();  // Render loop

	void SetWindowDimension(int width, int height);
	void EnableDepthBuffer(bool depthEnabled) { m_DepthEnabled = depthEnabled; }
	virtual void CreateCommandBuffers(); // Overide the default implementation as per application requirement
	
protected:
	// Core virtual methods used by derived classes
	virtual void Configure() = 0; // Application's user configuration prior to Setup()
	virtual void Setup() = 0;     // Set's up the drawing pipeline
	virtual void Update() = 0;    // Update data prior to Render() & Present() such as updating locals, uniforms etc.

	virtual bool Render();		  // Draw the primitive on surface
	virtual bool Present();		  // Swap the drawn surface on application window

private:
	// Initialization functions for Vulkan application
	void InitializeVulkan();
	void CreateVulkanInstance();
	void CreateSurface();
	
	// Device creation objects
	void CreateVulkanDeviceAndQueue();
	    // Helper functions for CreateVulkanDeviceAndQueue()
		void SelectPhysicalDevice();
		void GetPhysicalDeviceInfo(VkPhysicalDevice device, PhysicalDeviceInfo* pDeviceInfo);
		bool IsDeviceSuitable(PhysicalDeviceInfo deviceInfo);
	void CreateDeviceAndQueueObjects();

	void CreateSwapChain();
	void CreateDepthImage();

	void CreateSemaphores();

	virtual void CreateRenderPass();
	void CreateFramebuffers();

public:
	struct {
		VkFormat		m_Format;
		VkImage			m_Image;
		VkDeviceMemory	m_DeviceMemory;
		VkImageView		m_ImageView;
	}DepthImage;
	VkCommandBuffer		cmdBufferDepthImage;			// Command buffer for depth image layout

	// Application display window
    Window*		    m_pWindow;		// Display window object
	VkExtent2D		m_windowDim;	// Display window dimension
	string          m_appName;		// Display name
	bool			m_DepthEnabled; // Is depth buffer supported

    // Vulkan specific objects
    VkInstance      m_hInstance; // Vulkan instance object    
	VkSurfaceKHR    m_hSurface;  // Vulkan presentation surface

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
    VkFormat                    m_hSwapChainImageFormat;
    VkExtent2D                  m_swapChainExtent;
    std::vector<VkImageView>    m_hSwapChainImageViewList;
    uint32_t                    m_activeSwapChainImageIndex;

    // Render Pass
    VkRenderPass                m_hRenderPass;
	// Frame Buffer
	std::vector<VkFramebuffer>  m_hFramebuffers;

    // Command buffer related objects
    VkCommandPool                   m_hCommandPool;
    std::vector<VkCommandBuffer>    m_hCommandBufferList;
	
	// Presentation synchronization objects
	VkSemaphore                     m_hRenderReadySemaphore;
    VkSemaphore                     m_hPresentReadySemaphore;
};
#include "InstancingDemoApp.h"
#include "../../../common/VulkanHelper.h"
#include "Rect.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication qtApp(argc, argv);

    InstancingDemoApp* instanceDemo = new InstancingDemoApp();
    instanceDemo->SetWindowDimension(800, 600);
    instanceDemo->EnableDepthBuffer(true);
    instanceDemo->EnableWindowResize(true);
    instanceDemo->Initialize();
    qtApp.exec();

    delete instanceDemo;
    return 0;
}

InstancingDemoApp::InstancingDemoApp()
{
    VulkanHelper::GetInstanceLayerExtensionProperties();

    m_Scene = new Scene3D(this);

	RectangleModel* m_Cube = new RectangleModel(this, m_Scene, NULL, "Rectangle 1", SHAPE_RECTANGLE, RENDER_SCEHEME_MULTIDRAW);
    m_Cube->Rectangle(100, 100 , 100, 100);
    m_Cube->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
    m_Cube->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));

    m_Cube = new RectangleModel(this, m_Scene, m_Cube, "Rectangle 2", SHAPE_RECTANGLE, RENDER_SCEHEME_MULTIDRAW);
    m_Cube->Rectangle(100, 100, 50, 50);
    m_Cube->SetColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
    m_Cube->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
}

InstancingDemoApp::~InstancingDemoApp()
{
    delete m_Scene;
}

void InstancingDemoApp::Configure()
{
    SetApplicationName("Instancing Demo");
    SetWindowDimension(800, 600);

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

void InstancingDemoApp::Setup()
{
    // Note: We are overidding the default Create Command pool with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    // because we need to re-record the command buffer when the instance data size changes. 
    // This need to recreate the command buffer. 
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_physicalDeviceInfo.graphicsFamilyIndex;
    VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo, &poolInfo);

    static glm::mat4 Projection = glm::ortho(0.0f, static_cast<float>(m_windowDim.width), 0.0f, static_cast<float>(m_windowDim.height));
    m_Scene->SetProjection(&Projection);

    static glm::mat4 View;
    m_Scene->SetView(&View);

    m_Scene->SetUpProjection(); // For some reason the ViewMatrix is not working properly, this setupensure model matrix is set properly.

	m_Scene->Setup(); // Create the object's vertex buffer
}

void InstancingDemoApp::Update()
{
    static float phi = 0.0f;
    //glm::mat4 View = glm::lookAt(glm::vec3(500, 500, 500) * sin(phi += 0.01), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    //m_Scene->SetView(&View);
    static glm::mat4 View;
    m_Scene->SetView(&View);

	// TODO: Add dirty flag check to avoid constant update

	static int i = 0;
	i++;
	// This is a test check to see the perform
	if (i < 20)
	{
		printf("\n Update: %d......", i);
		m_Scene->Update();
	}
}

bool InstancingDemoApp::Render()
{
    // Important: Uncomment below line only if there are updates for model expected in the Model
    // 1. It has been observed that the re-recording of command buffer in case of non-instance drawing is expensive
    
    // m_Scene->Render(); //Read the note before uncommenting

    return VulkanApp::Render();
}

void InstancingDemoApp::ResizeWindow(int width, int height)
{
    VulkanApp::ResizeWindow(width, height);

	m_Scene->Resize(width, height);
}

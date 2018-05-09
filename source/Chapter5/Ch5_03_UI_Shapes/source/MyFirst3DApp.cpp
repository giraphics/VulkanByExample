#include "MyFirst3DApp.h"
#include "../../../common/VulkanHelper.h"
#include "Cube.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

int main(int argc, char **argv)
{
	QApplication qtApp(argc, argv);

	MyFirst3DApp* helloVulkanApp = new MyFirst3DApp(); // Create Vulkan app instance
	helloVulkanApp->SetWindowDimension(800, 600);    // Default application window dimension
	helloVulkanApp->EnableDepthBuffer(true);
	helloVulkanApp->Initialize();
	qtApp.exec();

	delete helloVulkanApp;
	return 0;
}

MyFirst3DApp::MyFirst3DApp()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
    m_CubeFactory = RectangleFactory::SingleTon(this);

	m_Scene = new Scene3D();
	
	m_Cube1 = new Model3D(m_Scene, NULL, "Node 1", SHAPE_CUBE);
	m_Cube1->Rectangle(100, 100, 100, 800);
	m_Cube1->SetColor(glm::vec4(0.2, 0.5, 0.50, 1.0));

	m_Cube2 = new Model3D(m_Scene, m_Cube1, "Node 2", SHAPE_CUBE);
	m_Cube2->Rectangle(0, 0, 100, 100);
	m_Cube2->SetColor(glm::vec4(0.6, 0.4, 0.20, 1.0));
//	m_Cube1->Rotate(2*3.14/4.0, 0.0, 0.0, 1.0);

	m_Cube1->Scale(0.5, 0.5, 0.5);
}

MyFirst3DApp::~MyFirst3DApp()
{
}

void MyFirst3DApp::Configure()
{
    SetApplicationName("My First 3D Application - Depth buffer");
    SetWindowDimension(800, 600);

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

void MyFirst3DApp::Setup()
{
	// Note: We are overidding the default Create Command pool with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	// because we need to re-record the command buffer when the instance data size changes. 
	// This need to recreate the command buffer. 
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_physicalDeviceInfo.graphicsFamilyIndex;
	VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo, &poolInfo);

//	static glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	static glm::mat4 Projection = glm::ortho(0.0, 800.0, 0.0, 600.0);
	m_Scene->SetProjection(&Projection);

//	static glm::mat4 View = glm::lookAt(glm::vec3(20, -20, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	static glm::mat4 View;
	//View = glm::translate(View, glm::vec3(0, 0, -100));
	m_Scene->SetView(&View);

	m_Scene->SetUpProjection(); // For some reason the ViewMatrix is not working properly, this setupensure model matrix is set properly.
	m_CubeFactory->Setup();
}

void MyFirst3DApp::Update()
{
	m_Scene->Update();
//	m_Cube1->Rotate(0.001, 0.0, 0.0, 1.0);
	//m_Cube1->Scale(0.5, 0.5, 0.5);
	m_CubeFactory->prepareInstanceData(m_Scene);
}
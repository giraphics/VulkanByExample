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
    m_CubeFactory = CubeFactory::SingleTon(this);

	m_Scene = new Scene3D();
    
	m_Cube1 = new Model3D(m_Scene, NULL, "Node 1", SHAPE_CUBE);
	m_Cube1->SetModel(m_CubeFactory->GetNewInstance(0)/*.m_Model*/);

	m_Cube2 = new Model3D(m_Scene, NULL, "Node 2", SHAPE_CUBE);
	m_Cube2->SetModel(m_CubeFactory->GetNewInstance(1)/*.m_Model*/);
}

MyFirst3DApp::~MyFirst3DApp()
{
	//delete m_Cube;
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
	static glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
//	m_CubeFactory->SetProjection(&Projection);
	m_Scene->SetProjection(&Projection);

//	static glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	static glm::mat4 View = glm::translate(glm::mat4(1), glm::vec3(0, 0, -15));
	//m_CubeFactory->SetView(&View);
	m_Scene->SetView(&View);

	VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    m_CubeFactory->instanceBufferNew.m_BufObj.m_MemoryFlags = memoryProperty;
    m_CubeFactory->instanceBufferNew.m_BufObj.m_DataSize = 2/*m_Scene->m_MatrixVector.size()*/ * sizeof(QMatrix4x4);

	VulkanHelper::CreateStagingBuffer(m_hDevice,
		m_physicalDeviceInfo.memProp,
		m_hCommandPool,
		m_hGraphicsQueue,
        m_CubeFactory->instanceBufferNew.m_BufObj,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		m_Scene->m_MatrixVector.data()); // Please use this Create Buffer currently begin used

	m_CubeFactory->Setup();
}

void MyFirst3DApp::Update()
{
	static float rot = 0;
    m_Cube1->Reset();
    m_Cube1->Rotate(rot += .1f, 0.0f, 0.0f, 1.0f);
    m_Cube1->Translate(5.0f, 0.0f, 0.0f);

    m_Cube2->Reset();

	m_Scene->Update();
	m_Scene->Sort();
	m_CubeFactory->Update();
	m_CubeFactory->prepareInstanceData(m_Scene);
}

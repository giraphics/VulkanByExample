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
	m_Cube2 = new Model3D(m_Scene, m_Cube1, "Node 2", SHAPE_CUBE);

	m_Cube2->Translate(4.0f, 0.0f, 0.0f);

//	m_Cube2->Translate(3.0f, 0.0f, 0.0f);
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
	m_Scene->SetUpProjection();

	VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    m_CubeFactory->m_InstanceBuffer.m_MemoryFlags = memoryProperty;
    m_CubeFactory->m_InstanceBuffer.m_DataSize = 2/*m_Scene->m_MatrixVector.size()*/ * sizeof(QMatrix4x4);

	VulkanHelper::CreateStagingBuffer(m_hDevice,
		m_physicalDeviceInfo.memProp,
		m_hCommandPool,
		m_hGraphicsQueue,
        m_CubeFactory->m_InstanceBuffer,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		m_Scene->m_MatrixVector.data()); // Please use this Create Buffer currently begin used

	m_CubeFactory->Setup();
}

void MyFirst3DApp::Update()
{
	static float rot = 0;
	m_Cube1->Rotate(rot = .01f, 0.0f, 1.0f, 0.0f);
	//	m_Scene->Update();
//	m_Cube1->Reset();
	m_Scene->UpdateNew();
//	m_CubeFactory->Update();
	m_CubeFactory->prepareInstanceData(m_Scene);
}

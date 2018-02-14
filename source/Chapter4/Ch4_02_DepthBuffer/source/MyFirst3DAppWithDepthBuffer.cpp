#include "MyFirst3DAppWithDepthBuffer.h"
#include "../../../common/VulkanHelper.h"
#include "Cube.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

int main(int argc, char **argv)
{
	QApplication qtApp(argc, argv);

	MyFirst3DAppWithDepthBuffer* helloVulkanApp = new MyFirst3DAppWithDepthBuffer(); // Create Vulkan app instance
	helloVulkanApp->EnableDepthBuffer(true);
	helloVulkanApp->Initialize();
	qtApp.exec();
	
	delete helloVulkanApp;
	return 0;
}

MyFirst3DAppWithDepthBuffer::MyFirst3DAppWithDepthBuffer()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
    m_Cube = new Cube(this);
}

MyFirst3DAppWithDepthBuffer::~MyFirst3DAppWithDepthBuffer()
{
    if (m_Cube) { delete m_Cube; }
}

void MyFirst3DAppWithDepthBuffer::Configure()
{
    SetApplicationName("My First 3D Application with Depth buffer");
    SetWindowDimension(800, 600);

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

void MyFirst3DAppWithDepthBuffer::Setup()
{
	m_Cube->Setup();
}

void MyFirst3DAppWithDepthBuffer::Update()
{
	m_Cube->Update();
}

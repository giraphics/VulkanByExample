#include "MyFirst3DAppWithDepthBuffer.h"
#include "../../../common/VulkanHelper.h"
#include "Cube.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

std::vector<const char *> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

// Allow the user to specify the Vulkan instance extensions
std::vector<const char *> instanceExtensionNames = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
};

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
}

void MyFirst3DAppWithDepthBuffer::Configure()
{
	m_appName = "My First 3D Application with Depth buffer";
}

void MyFirst3DAppWithDepthBuffer::Setup()
{
	m_pWindow->setTitle(QString(m_appName.c_str()));
	m_Cube->Setup();
}

void MyFirst3DAppWithDepthBuffer::Update()
{
	m_Cube->Update();
}

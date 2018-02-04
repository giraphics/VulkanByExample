#include "MyFirst3DApp.h"
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

	MyFirst3DApp* helloVulkanApp = new MyFirst3DApp(); // Create Vulkan app instance
	helloVulkanApp->EnableDepthBuffer(false);
	helloVulkanApp->Initialize();
	qtApp.exec();

	delete helloVulkanApp;
	return 0;
}

MyFirst3DApp::MyFirst3DApp()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
	m_Cube = new Cube(this);
}

MyFirst3DApp::~MyFirst3DApp()
{
}

void MyFirst3DApp::Configure()
{
	m_appName = "My First 3D Application";
}

void MyFirst3DApp::Setup()
{
	m_pWindow->setTitle(QString(m_appName.c_str()));
	m_Cube->Setup();
}

void MyFirst3DApp::Update()
{
	m_Cube->Update();
}
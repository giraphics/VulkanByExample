#include "MeshDemoApp.h"
#include "../../../common/VulkanHelper.h"

#include "SimpleMesh.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

MeshDemoApp::MeshDemoApp()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
	m_Cube = new SimpleMesh(this);
}

MeshDemoApp::~MeshDemoApp()
{
	delete m_Cube;
}

void MeshDemoApp::Configure()
{
	m_appName = "Simple Mesh Application";
}

void MeshDemoApp::Setup()
{
	m_pWindow->setTitle(QString(m_appName.c_str()));
	m_Cube->Setup();
}

void MeshDemoApp::Update()
{
	m_Cube->Update();
}

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

	MeshDemoApp* meshApp = new MeshDemoApp(); // Create Vulkan app instance
	meshApp->EnableDepthBuffer(true);
	meshApp->Initialize();
	qtApp.exec();
	
	delete meshApp;
	return 0;
}
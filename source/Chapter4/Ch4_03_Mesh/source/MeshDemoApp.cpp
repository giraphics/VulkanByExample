#include "MeshDemoApp.h"
#include "../../../common/VulkanHelper.h"

#include "SimpleMesh.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

MeshDemoApp::MeshDemoApp()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
	m_Cube = new Cube(this);
}

MeshDemoApp::~MeshDemoApp()
{
    if (m_Cube) { delete m_Cube; }
}

void MeshDemoApp::Configure()
{
    SetApplicationName("Simple Mesh Application");
    SetWindowDimension(800, 600);

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

void MeshDemoApp::Setup()
{
	m_Cube->Setup();
}

void MeshDemoApp::Update()
{
	m_Cube->Update();
}

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
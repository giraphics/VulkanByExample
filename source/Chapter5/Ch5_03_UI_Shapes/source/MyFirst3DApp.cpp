#include "MyFirst3DApp.h"
#include "../../../common/VulkanHelper.h"
#include "Cube.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>
extern bool isDirty;
int main(int argc, char **argv)
{
	QApplication qtApp(argc, argv);

	MyFirst3DApp* helloVulkanApp = new MyFirst3DApp(); // Create Vulkan app instance
	helloVulkanApp->EnableDepthBuffer(true);
	helloVulkanApp->Initialize();
	qtApp.exec();

	delete helloVulkanApp;
	return 0;
}

MyFirst3DApp::MyFirst3DApp()
{
	VulkanHelper::GetInstanceLayerExtensionProperties();
}

MyFirst3DApp::~MyFirst3DApp()
{
}

void MyFirst3DApp::ProgressBarFunc(Scene3D* m_Scene)
{
	//ProgressBar* m_Parent = new ProgressBar(m_Scene, NULL, "Node 1", SHAPE_CUBE);
}

void MyFirst3DApp::Grid(Scene3D* m_Scene)
{
	float parentCol = 20;
	float parentRow = 10;
	float parentColWidth = m_windowDim.width / parentCol;
	float parentColHeight = m_windowDim.height / parentRow;

	const float Col = 10;
	const float Row = 10;
	float colWidth = parentColWidth / Col;
	float colHeight = parentColHeight / Row;

	for (int i = 0; i < parentCol; i++)
	{
		for (int j = 0; j < parentRow; j++)
		{
			Model3D* m_Parent = new Model3D(m_Scene, NULL, "Node 1", SHAPE_CUBE);
			m_Parent->Rectangle((i * parentColWidth), (j * parentColHeight), parentColWidth - 2, parentColHeight);
			m_Parent->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
			m_Parent->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));

			for (int k = 0; k < Col; k++)
			{
				for (int l = 0; l < Row; l++)
				{
					m_Cube1 = new Model3D(m_Scene, m_Parent, "Node 1", SHAPE_CUBE);
					m_Cube1->Rectangle((k * colWidth), (l * colHeight), colWidth, colHeight);
					m_Cube1->SetColor(glm::vec4(0.2, 0.5, 0.50, 1.0));
					m_Cube1->SetDefaultColor(glm::vec4(0.2, 0.5, 0.50, 1.0));
				}
			}
		}
	}
}

void MyFirst3DApp::Configure()
{
    SetApplicationName("My First 3D Application - Depth buffer");
    SetWindowDimension(800*2, 600);

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Add Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	m_CubeFactory = RectangleFactory::SingleTon(this);

	m_Scene = new Scene3D();
//	ProgressBar(m_Scene);
	ProgressBar* m_Parent = new ProgressBar(m_Scene, NULL, "Node 1", SHAPE_CUBE);
//	Grid(m_Scene);
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

	// static glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	static glm::mat4 Projection = glm::ortho(0.0f, static_cast<float>(m_windowDim.width), 0.0f, static_cast<float>(m_windowDim.height));
	m_Scene->SetProjection(&Projection);

	// static glm::mat4 View = glm::lookAt(glm::vec3(20, -20, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	static glm::mat4 View;
	//View = glm::translate(View, glm::vec3(0, 0, -100));
	m_Scene->SetView(&View);

	m_Scene->SetUpProjection(); // For some reason the ViewMatrix is not working properly, this setupensure model matrix is set properly.
	m_CubeFactory->Setup();
	isDirty = true;
}

void MyFirst3DApp::Update()
{
	static int i = 0;
	if (i++ < 40)
	{
		isDirty = true;
	}

	if (!isDirty) return;

	m_Scene->Update();
	m_CubeFactory->prepareInstanceData(m_Scene);

	isDirty = false;
}

void MyFirst3DApp::mousePressEvent(QMouseEvent* p_Event)
{
	m_Scene->mousePressEvent(p_Event);
}

void MyFirst3DApp::mouseReleaseEvent(QMouseEvent* p_Event) 
{ 
	m_Scene->mouseReleaseEvent(p_Event);
}

void MyFirst3DApp::mouseMoveEvent(QMouseEvent* p_Event) 
{ 
	m_Scene->mouseMoveEvent(p_Event);
}

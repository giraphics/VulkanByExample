#include "InstancingDemoApp.h"
#include "../../../common/VulkanHelper.h"
#include "Rect.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication qtApp(argc, argv);

    InstancingDemoApp* instanceDemo = new InstancingDemoApp(); // Create Vulkan app instance
    instanceDemo->SetWindowDimension(800, 600);    // Default application window dimension
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

    m_CubeFactory = RectangleMultiDrawFactory::SingleTon(this);

    m_Scene = new Scene3D();

    m_Cube = m_CubeFactory->GetModel(this, m_Scene, NULL, "Rectangle 1", SHAPE_RECTANGLE);
    m_Cube->Rectangle(0, 0 , 10, 10);
    m_Cube->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
    m_Cube->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
}

InstancingDemoApp::~InstancingDemoApp()
{
    delete m_Cube;
}

void InstancingDemoApp::Grid(Scene3D* m_Scene)
{
    //float parentCol = 20;
    //float parentRow = 10;
    //float parentColWidth = m_windowDim.width / parentCol;
    //float parentColHeight = m_windowDim.height / parentRow;

    //const float Col = 10;
    //const float Row = 10;
    //float colWidth = parentColWidth / Col;
    //float colHeight = parentColHeight / Row;

    //for (int i = 0; i < parentCol; i++)
    //{
    //	for (int j = 0; j < parentRow; j++)
    //	{
    //		Model3D* m_Parent = new Model3D(m_Scene, NULL, "Node 1", SHAPE_CUBE);
    //		m_Parent->Rectangle((i * parentColWidth), (j * parentColHeight), parentColWidth - 2, parentColHeight);
    //		m_Parent->SetColor(glm::vec4(0.6, 0.2, 0.20, 1.0));
    //		m_Parent->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));

    //		for (int k = 0; k < Col; k++)
    //		{
    //			for (int l = 0; l < Row; l++)
    //			{
    //				m_Cube1 = new Model3D(m_Scene, m_Parent, "Node 1", SHAPE_CUBE);
    //				m_Cube1->Rectangle((k * colWidth), (l * colHeight), colWidth, colHeight);
    //				m_Cube1->SetColor(glm::vec4(0.2, 0.5, 0.50, 1.0));
    //				m_Cube1->SetDefaultColor(glm::vec4(0.2, 0.5, 0.50, 1.0));
    //			}
    //		}
    //	}
    //}
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
    //m_Scene = new Scene3D();
    //Grid(m_Scene);
}

void InstancingDemoApp::Setup()
{
	static glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	m_Scene->SetProjection(&Projection);

	static glm::mat4 View = glm::lookAt(glm::vec3(100, -200, 300), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_Scene->SetView(&View);

	m_Scene->Setup(); // Create the object's vertex buffer
}

void InstancingDemoApp::Update()
{
    static float phi = 0.0f;
    glm::mat4 View = glm::lookAt(glm::vec3(500, 500, 500) * sin(phi += 0.01), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    m_Scene->SetView(&View);
    //static float rot = 0;
    //m_Cube->Rotate(rot += .001f, 0.0f, 1.0f, 0.0f);
    //m_Cube->Update();

    m_CubeFactory->m_Transform = (*m_Scene->GetProjection()) * (*m_Scene->GetView());

    m_CubeFactory->Prepare(m_Scene);
    m_CubeFactory->Update();

    m_Scene->Update();
}

void InstancingDemoApp::ResizeWindow(int width, int height)
{
    VulkanApp::ResizeWindow(width, height);

    // Parminder: Need to think over how to recreate the pipeline for various factories
    m_CubeFactory->ResizeWindow(width, height);
}

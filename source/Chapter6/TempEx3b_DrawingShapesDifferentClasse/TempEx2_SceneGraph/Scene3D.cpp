#include "Scene3D.h"
#include "Model3D.h"

#include "../../common/VulkanApp.h" // Not a good design to include vulkan app here: Todo move AbstractApp 
#include <QMouseEvent> 
/*extern*/ bool isDirty;

Scene3D::Scene3D(AbstractApp* p_Application)
    : m_Application(p_Application)
    , m_Frame(0)
    , m_ScreenWidth(800)
    , m_ScreenHeight(600)
    , m_CurrentHoverItem(NULL)
{
}

Scene3D::~Scene3D()
{
    std::map<SHAPE, AbstractRenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        delete itSRST->second;

        ++itSRST;
    }

    foreach (DrawItem* currentModel, m_ModelList)
    {
        delete currentModel;
    }
}

void Scene3D::Setup(VkCommandBuffer& p_CommandBuffer)
{
    GatherFlatModelList(); // Assuming all nodes are added into the scenes by now

    foreach (DrawItem* currentModel, m_ModelList)
    {
        currentModel->Setup();
    }

    foreach(DrawItem* currentModel, m_FlatList)
    {
        AbstractRenderSchemeFactory* factory = GetFactory(currentModel); // Populate factories
        if (!factory) continue;

        factory->UpdateModelList(currentModel);
    }

    std::map<SHAPE, AbstractRenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_ModelFactories.insert(itSRST->second);

        itSRST++;
    }

	assert(m_ModelFactories.size());
	foreach(AbstractRenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Setup(p_CommandBuffer);
    }
}

void Scene3D::Update()
{
    foreach(AbstractRenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->SetRefProjectViewMatrix(*m_Transform.GetProjectionMatrix() * *m_Transform.GetViewMatrix());
    }

    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->Update();
    }

	foreach(AbstractRenderSchemeFactory* currentModelFactory, m_ModelFactories)
	{
		currentModelFactory->Update();
	}
}

void Scene3D::Render(VkCommandBuffer& p_CommandBuffer)
{
    foreach(AbstractRenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Render(p_CommandBuffer);
    }
}

void Scene3D::GatherFlatModelList()
{
    m_FlatList.clear();

    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->GatherFlatModelList();
    }
}

void Scene3D::AddModel(DrawItem* p_Model)
{
    if (p_Model && !p_Model->GetParent())
    {
        m_ModelList.push_back(p_Model);
    }
}

void Scene3D::RemoveModel(DrawItem* p_Model)
{
    while (true)
    {
        auto result = std::find(std::begin(m_ModelList), std::end(m_ModelList), p_Model);
        if (result == std::end(m_ModelList)) break;

        m_ModelList.erase(result);
    }
}

void Scene3D::Resize(VkCommandBuffer& p_CommandBuffer, int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;

    foreach(AbstractRenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->ResizeWindow(p_CommandBuffer);
    }
}

// Default implementation, extend this function as per requirement in your function
void Scene3D::SetUpProjection()
{
    m_Transform.SetMatrixMode(Transformation3D::PROJECTION_MATRIX);
    m_Transform.LoadIdentity();
    m_Transform.Ortho(0.0f, static_cast<float>(m_ScreenWidth), 0.0f, static_cast<float>(m_ScreenHeight), -1.0f, 1.0f);

    m_Transform.SetMatrixMode(Transformation3D::VIEW_MATRIX);
    m_Transform.LoadIdentity();

    m_Transform.SetMatrixMode(Transformation3D::MODEL_MATRIX);
    m_Transform.LoadIdentity();
}

void Scene3D::mousePressEvent(QMouseEvent* p_Event)
{
    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Scene3D::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Scene3D::mouseMoveEvent(QMouseEvent* p_Event)
{
    DrawItem* oldModelItem = GetCurrentHoverItem();
    for (int i = m_ModelList.size() - 1; i >= 0; i--)
    {
        DrawItem* item = m_ModelList.at(i);
        assert(item);

        if (item->mouseMoveEvent(p_Event))
        {
            DrawItem* currentModel = GetCurrentHoverItem();
            if (oldModelItem && oldModelItem != currentModel)
            {
                oldModelItem->SetColor(oldModelItem->GetDefaultColor());
            }

            currentModel->SetColor(glm::vec4(1.0, 1.0, 0.3, 1.0));
            isDirty = true;
            return;
        }
    }

    if (oldModelItem)
    {
        oldModelItem->SetColor(oldModelItem->GetDefaultColor());
    }
}

AbstractRenderSchemeFactory* Scene3D::GetFactory(DrawItem* p_Model)
{
    const SHAPE shapeType = p_Model->GetShapeType();
    if ((shapeType <= SHAPE_NONE) && (shapeType >= SHAPE_COUNT)) return NULL;

    std::map<SHAPE, AbstractRenderSchemeFactory*>::iterator it = m_ShapeRenderSchemeTypeMap.find(shapeType);
    if (it != m_ShapeRenderSchemeTypeMap.end())
    {
        return it->second;
    }

    AbstractRenderSchemeFactory* abstractFactory = p_Model->GetRenderSchemeFactory();
    if (abstractFactory)
    {
        (m_ShapeRenderSchemeTypeMap)[shapeType] = abstractFactory;
    }

    return abstractFactory;
}


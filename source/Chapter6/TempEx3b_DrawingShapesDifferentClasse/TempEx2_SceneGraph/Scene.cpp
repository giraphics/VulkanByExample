#include "Scene.h"
#include "DrawItem.h"
#include "../../common/VulkanApp.h"

Scene::Scene(AbstractApp* p_Application)
    : m_Application(p_Application)
    , m_Frame(0)
    , m_ScreenWidth(800)
    , m_ScreenHeight(600)
    , m_CurrentHoverItem(NULL)
{
}

Scene::~Scene()
{
    std::map<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

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

void Scene::Setup(VkCommandBuffer& p_CommandBuffer)
{
    GatherDrawItemsFlatList(); // Assuming all nodes are added into the scenes by now

    foreach (DrawItem* currentModel, m_ModelList)
    {
        currentModel->Setup();
    }

    foreach(DrawItem* currentModel, m_FlatList)
    {
        RenderSchemeFactory* factory = GetFactory(currentModel); // Populate factories
        if (!factory) continue;

        factory->UpdateModelList(currentModel);
    }

    std::map<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_ModelFactories.insert(itSRST->second);

        itSRST++;
    }

	assert(m_ModelFactories.size());
    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Setup(p_CommandBuffer);
    }
}

void Scene::Update()
{
    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->SetRefProjectViewMatrix(*m_Transform.GetProjectionMatrix() * *m_Transform.GetViewMatrix());
    }

    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->Update();
    }

    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Update();
    }
}

void Scene::Render(VkCommandBuffer& p_CommandBuffer)
{
    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Render(p_CommandBuffer);
    }
}

void Scene::GatherDrawItemsFlatList()
{
    m_FlatList.clear();

    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->GatherDrawItemsFlatList();
    }
}

void Scene::AddItem(DrawItem* p_Item)
{
    if (p_Item && !p_Item->GetParent())
    {
        m_ModelList.push_back(p_Item);
    }
}

void Scene::RemoveItem(DrawItem* p_Item)
{
    while (true)
    {
        auto result = std::find(std::begin(m_ModelList), std::end(m_ModelList), p_Item);
        if (result == std::end(m_ModelList)) break;

        m_ModelList.erase(result);
    }
}

void Scene::Resize(VkCommandBuffer& p_CommandBuffer, int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;

    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->ResizeWindow(p_CommandBuffer);
    }
}

// Default implementation, extend this function as per requirement in your function
void Scene::SetUpProjection()
{
    m_Transform.SetMatrixMode(Transformation::PROJECTION_MATRIX);
    m_Transform.LoadIdentity();
    m_Transform.Ortho(0.0f, static_cast<float>(m_ScreenWidth), 0.0f, static_cast<float>(m_ScreenHeight), -1.0f, 1.0f);

    m_Transform.SetMatrixMode(Transformation::VIEW_MATRIX);
    m_Transform.LoadIdentity();

    m_Transform.SetMatrixMode(Transformation::MODEL_MATRIX);
    m_Transform.LoadIdentity();
}

void Scene::mousePressEvent(QMouseEvent* p_Event)
{
    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Scene::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach (DrawItem* item, m_ModelList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Scene::mouseMoveEvent(QMouseEvent* p_Event)
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
            return;
        }
    }

    if (oldModelItem)
    {
        oldModelItem->SetColor(oldModelItem->GetDefaultColor());
    }
}

RenderSchemeFactory* Scene::GetFactory(DrawItem* p_Item)
{
    const SHAPE shapeType = p_Item->GetShapeType();
    if ((shapeType <= SHAPE_NONE) && (shapeType >= SHAPE_COUNT)) return NULL;

    std::map<SHAPE, RenderSchemeFactory*>::iterator it = m_ShapeRenderSchemeTypeMap.find(shapeType);
    if (it != m_ShapeRenderSchemeTypeMap.end())
    {
        return it->second;
    }

    RenderSchemeFactory* abstractFactory = p_Item->GetRenderSchemeFactory();
    if (abstractFactory)
    {
        (m_ShapeRenderSchemeTypeMap)[shapeType] = abstractFactory;
    }

    return abstractFactory;
}

void Scene::AppendToDrawItemsFlatList(DrawItem* p_Item)
{
    m_FlatList.push_back(p_Item);
}

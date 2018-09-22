#include "Scene.h"
#include "Node.h"

#include "../../common/VulkanApp.h" // Not a good design to include vulkan app here: Todo move AbstractApp 
#include <QMouseEvent> 

Scene::Scene(AbstractApp* p_Application)
    : m_Application(p_Application)
    , m_Frame(0)
    , m_ScreenWidth(800)
    , m_ScreenHeight(600)
    , m_CurrentHoverItem(NULL)
    , m_Projection(NULL)	// Not owned by Scene, double check this can be owned. TODO: PS
    , m_View(NULL)		// Not owned by Scene
    , m_DirtyType(SCENE_DIRTY_TYPE::ALL)
{
}

Scene::~Scene()
{
    RenderSchemeTypeMap* m_FactoryMap = NULL;
    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_FactoryMap = itSRST->second;
        std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->begin();
        if (it != m_FactoryMap->end())
        {
            delete it->second;
        }

        ++itSRST;
    }

    foreach (Node* currentModel, m_ModelList)
    {
        delete currentModel;
    }
}

void Scene::Setup()
{
    GatherFlatList(); // Assuming all nodes are added into the scenes by now

    foreach (Node* currentModel, m_ModelList)
    {
        currentModel->Setup();
    }

    foreach (Node* currentModel, m_FlatList)
    {
        RenderSchemeFactory* factory = GetFactory(currentModel); // Populate factories
        if (!factory) continue;

        factory->UpdateNodeList(currentModel);
    }

    ////////////////////////////////////////////////
    RenderSchemeTypeMap* m_FactoryMap = NULL;
    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
    if (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_FactoryMap = itSRST->second;
        std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->begin();
        if (it != m_FactoryMap->end())
        {
            m_ModelFactories.insert(it->second);
            it++;
        }

        itSRST++;
    }


    assert(m_ModelFactories.size());
    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Setup();
    }

    // Setup is the first time update() therefore update ALL
    m_DirtyType = SCENE_DIRTY_TYPE::ALL;
}

void Scene::Update()
{
    if (!IsDirty()) return;

    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
//        currentModelFactory->m_Transform = (*GetProjection()) * (*GetView());
        glm::mat4 transformation = (*GetProjection()) * (*GetView());
        currentModelFactory->SetRefProjectViewMatrix(transformation);
    }

    const SCENE_DIRTY_TYPE updateTransformType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::TRANSFORMATION));
    if (updateTransformType == SCENE_DIRTY_TYPE::TRANSFORMATION)
    {
        foreach (Node* item, m_ModelList)
        {
            assert(item);

            item->Update();
        }
    }

    SCENE_DIRTY_TYPE updateItemType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::ALL_ITEMS));
    if (updateItemType == SCENE_DIRTY_TYPE::ALL_ITEMS)
    {
        foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
        {
            currentModelFactory->Update();
        }
    }
    else
    {
        updateItemType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::DIRTY_ITEMS));
        if (updateItemType == SCENE_DIRTY_TYPE::DIRTY_ITEMS)
        {
            foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
            {
                currentModelFactory->UpdateDirty();
            }
        }
    }

    m_DirtyType = SCENE_DIRTY_TYPE::NONE;
}

void Scene::Render()
{
    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
    {
        currentModelFactory->Render();
    }
}

void Scene::GatherFlatList()
{
    m_FlatList.clear();

    foreach (Node* item, m_ModelList)
    {
        assert(item);

        item->GatherFlatList();
    }
}

void Scene::AddModel(Node* p_Model)
{
    if (p_Model && !p_Model->GetParent())
    {
        m_ModelList.push_back(p_Model);
    }
}


// While removing the model remove it from model list and flat list.
void Scene::RemoveModel(Node* p_Model)
{
    while (true)
    {
        auto result = std::find(std::begin(m_ModelList), std::end(m_ModelList), p_Model);
        if (result == std::end(m_ModelList)) break;

        m_ModelList.erase(result);
    }

    while (true)
    {
        auto result = std::find(std::begin(m_FlatList), std::end(m_FlatList), p_Model);
        if (result == std::end(m_FlatList)) break;

        m_ModelList.erase(result);
    }

    RenderSchemeFactory* factory = GetFactory(p_Model); // Populate factories
    if (!factory) return;

    factory->RemoveNodeList(p_Model);
}

void Scene::Resize(int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;

    foreach(RenderSchemeFactory* currentModelFactory, m_ModelFactories)
	{
		currentModelFactory->ResizeWindow(p_Width, p_Height);
	}
}

void Scene::SetUpProjection()
{
    m_Transform.SetMatrixMode(Transformation::PROJECTION_MATRIX);
    m_Transform.LoadIdentity();

    m_Transform.SetPerspective(45.0f, float(m_ScreenWidth)/m_ScreenHeight, 0.10f, 100.0f);

    m_Transform.SetMatrixMode(Transformation::VIEW_MATRIX);
    glm::vec3 eye(10.0, 10.0, 10.0);
    glm::vec3 center(0.0, 0.0, 0.0);
    glm::vec3 up(0.0, 1.0, 0.0);
    m_Transform.LoadIdentity();
    m_Transform.LookAt(&eye, &center, &up);

    m_Transform.SetMatrixMode(Transformation::MODEL_MATRIX);
    m_Transform.LoadIdentity();
}

void Scene::mousePressEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_ModelList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Scene::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_ModelList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Scene::mouseMoveEvent(QMouseEvent* p_Event)
{
    Node* oldModelItem = GetCurrentHoverItem();
    for (int i = m_ModelList.size() - 1; i >= 0; i--)
    {
        Node* item = m_ModelList.at(i);
        assert(item);

        if (item->mouseMoveEvent(p_Event))
        {
            Node* currentModel = GetCurrentHoverItem();
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

RenderSchemeFactory* Scene::GetFactory(Node* p_Model)
{
    const SHAPE shapeType = p_Model->GetShapeType();
    if ((shapeType <= SHAPE::SHAPE_NONE) && (shapeType >= SHAPE::SHAPE_COUNT)) return NULL;

    RenderSchemeTypeMap* m_FactoryMap = NULL;
    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.find(shapeType);
    if (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_FactoryMap = itSRST->second;
    }
    else
    {
        m_FactoryMap = new RenderSchemeTypeMap();
        m_ShapeRenderSchemeTypeMap[shapeType] = m_FactoryMap;
    }

    const RENDER_SCEHEME_TYPE renderSchemeType = p_Model->GetRenderSchemeType();
    std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->find(renderSchemeType);
    if (it != m_FactoryMap->end())
    {
        return it->second;
    }

    RenderSchemeFactory* abstractFactory = p_Model->GetRenderScemeFactory();
    if (abstractFactory)
    {
        (*m_FactoryMap)[renderSchemeType] = abstractFactory;
    }

    return abstractFactory;
}


#include "Scene.h"
#include "Node.h"

#include "../../common/VulkanApp.h" // Not a good design to include vulkan app here: Todo move AbstractApp 
#include <QMouseEvent> 

Scene::Scene(AbstractApp* p_Application, const QString& p_Name)
    : m_Application(p_Application)
    , m_Name(p_Name)
    , m_Frame(0)
    , m_ScreenWidth(800)
    , m_ScreenHeight(600)
    , m_CurrentHoverItem(NULL)
    , m_Projection(NULL)        // Not owned by Scene, double check this can be owned. TODO: PS
    , m_View(NULL)              // Not owned by Scene
    , m_DirtyType(SCENE_DIRTY_TYPE::ALL)
{
}

Scene::~Scene()
{
//    RenderSchemeTypeMap* m_FactoryMap = NULL;
//    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

//    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
//    {
//        m_FactoryMap = itSRST->second;
//        std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->begin();
//        if (it != m_FactoryMap->end())
//        {
//            delete it->second;
//        }

//        ++itSRST;
//    }
    std::map<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();

    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        delete itSRST->second;

        ++itSRST;
    }

    foreach (Node* currentNode, m_NodeList)
    {
        delete currentNode;
    }
}

void Scene::Setup()
{
    GatherFlatNodeList(); // Assuming all nodes are added into the scenes by now

    foreach (Node* currentModel, m_NodeList)
    {
        currentModel->Setup();
    }

    foreach (Node* currentModel, m_FlatList)
    {
        RenderSchemeFactory* factory = GetRenderSchemeFactory(currentModel); // Populate factories
        if (!factory) continue;

        factory->UpdateNodeList(currentModel);
    }

    ////////////////////////////////////////////////
//    RenderSchemeTypeMap* m_FactoryMap = NULL;
//    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
//    if (itSRST != m_ShapeRenderSchemeTypeMap.end())
//    {
//        m_FactoryMap = itSRST->second;
//        std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->begin();
//        if (it != m_FactoryMap->end())
//        {
//            m_RenderSchemeFactorySet.insert(it->second);
//            it++;
//        }

//        itSRST++;
//    }
    std::map<SHAPE, RenderSchemeFactory*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.begin();
    while (itSRST != m_ShapeRenderSchemeTypeMap.end())
    {
        m_RenderSchemeFactorySet.insert(itSRST->second);

        itSRST++;
    }


    assert(m_RenderSchemeFactorySet.size());
    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        currentModelFactory->Setup();
    }

    // Setup is the first time update() therefore update ALL
    m_DirtyType = SCENE_DIRTY_TYPE::ALL;
}

void Scene::Update()
{
    if (!IsDirty()) return;

    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
//        currentModelFactory->m_Transform = (*GetProjection()) * (*GetView());
        glm::mat4 transformation = (*GetProjection()) * (*GetView());
        currentModelFactory->SetRefProjectViewMatrix(transformation);
    }

    const SCENE_DIRTY_TYPE updateTransformType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::TRANSFORMATION));
    if (updateTransformType == SCENE_DIRTY_TYPE::TRANSFORMATION)
    {
        foreach (Node* item, m_NodeList)
        {
            assert(item);

            item->Update();
        }
    }

    SCENE_DIRTY_TYPE updateItemType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::ALL_ITEMS));
    if (updateItemType == SCENE_DIRTY_TYPE::ALL_ITEMS)
    {
        foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
        {
            currentModelFactory->Update();
        }
    }
    else
    {
        updateItemType = static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(SCENE_DIRTY_TYPE::DIRTY_ITEMS));
        if (updateItemType == SCENE_DIRTY_TYPE::DIRTY_ITEMS)
        {
            foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
            {
                currentModelFactory->UpdateDirty();
            }
        }
    }

    m_DirtyType = SCENE_DIRTY_TYPE::NONE;
}

void Scene::Render()
{
    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
    {
        currentModelFactory->Render();
    }
}

void Scene::GatherFlatNodeList()
{
    m_FlatList.clear();

    foreach (Node* item, m_NodeList)
    {
        assert(item);

        item->GatherFlatNodeList();
    }
}

void Scene::AddItem(Node* p_Item)
{
    if (p_Item && !p_Item->GetParent())
    {
        m_NodeList.push_back(p_Item);
    }
}


// While removing the model remove it from model list and flat list.
void Scene::RemoveItem(Node* p_Item)
{
    while (true)
    {
        auto result = std::find(std::begin(m_NodeList), std::end(m_NodeList), p_Item);
        if (result == std::end(m_NodeList)) break;

        m_NodeList.erase(result);
    }

    while (true)
    {
        auto result = std::find(std::begin(m_FlatList), std::end(m_FlatList), p_Item);
        if (result == std::end(m_FlatList)) break;

        m_NodeList.erase(result);
    }

    RenderSchemeFactory* factory = GetRenderSchemeFactory(p_Item); // Populate factories
    if (!factory) return;

    factory->RemoveNodeList(p_Item);
}

void Scene::Resize(int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;

    foreach(RenderSchemeFactory* currentModelFactory, m_RenderSchemeFactorySet)
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
    foreach (Node* item, m_NodeList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Scene::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach (Node* item, m_NodeList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Scene::mouseMoveEvent(QMouseEvent* p_Event)
{
    Node* oldModelItem = GetCurrentHoverItem();
    for (int i = m_NodeList.size() - 1; i >= 0; i--)
    {
        Node* item = m_NodeList.at(i);
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

RenderSchemeFactory* Scene::GetRenderSchemeFactory(Node* p_Item)
{
    const SHAPE shapeType = p_Item->GetShapeType();
    if ((shapeType <= SHAPE::SHAPE_NONE) && (shapeType >= SHAPE::SHAPE_COUNT)) return NULL;

//    RenderSchemeTypeMap* m_FactoryMap = NULL;
//    std::map<SHAPE, RenderSchemeTypeMap*>::iterator itSRST = m_ShapeRenderSchemeTypeMap.find(shapeType);
//    if (itSRST != m_ShapeRenderSchemeTypeMap.end())
//    {
//        m_FactoryMap = itSRST->second;
//    }
//    else
//    {
//        m_FactoryMap = new RenderSchemeTypeMap();
//        m_ShapeRenderSchemeTypeMap[shapeType] = m_FactoryMap;
//    }
    std::map<SHAPE, RenderSchemeFactory*>::iterator it = m_ShapeRenderSchemeTypeMap.find(shapeType);
    if (it != m_ShapeRenderSchemeTypeMap.end())
    {
        return it->second;
    }

//    const RENDER_SCEHEME_TYPE renderSchemeType = p_Item->GetRenderSchemeType();
//    std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*>::iterator it = m_FactoryMap->find(renderSchemeType);
//    if (it != m_FactoryMap->end())
//    {
//        return it->second;
//    }
//
//    RenderSchemeFactory* abstractFactory = p_Item->GetRenderSchemeFactory();
//    if (abstractFactory)
//    {
//        (*m_FactoryMap)[renderSchemeType] = abstractFactory;
//    }
    RenderSchemeFactory* renderSchemeFactoryItem = p_Item->GetRenderSchemeFactory();
    if (renderSchemeFactoryItem)
    {
        (m_ShapeRenderSchemeTypeMap)[shapeType] = renderSchemeFactoryItem;
    }

    return renderSchemeFactoryItem;
}

void Scene::AppendToFlatNodeList(Node *p_Item)
{
    m_FlatList.push_back(p_Item);
}


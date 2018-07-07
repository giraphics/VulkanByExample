#include "Model3D.h"
#include<QMouseEvent>
#include<glm/gtx/string_cast.hpp>

Model3D::Model3D(Scene3D *p_Scene, Model3D *p_Parent, const QString &p_Name, SHAPE p_ShapeType, RENDER_SCEHEME_TYPE p_RenderSchemeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_ShapeType(p_ShapeType)
	, m_RenderSchemeType(p_RenderSchemeType)
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddModel(this);
}

void Model3D::Setup()
{
    foreach(Model3D* currentModel, m_ChildList)
    {
        Model3D* model = /*dynamic_cast<Model3D*>*/(currentModel);

        if (!model) continue;

        model->Setup();
    }
}

void Model3D::mousePressEvent(QMouseEvent* p_Event)
{
    foreach(Model3D* item, m_ChildList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Model3D::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach(Model3D* item, m_ChildList)
    {
        assert(item);

        item->mouseReleaseEvent(p_Event);
    }
}

void Model3D::mouseMoveEvent(QMouseEvent* p_Event)
{
    foreach(Model3D* item, m_ChildList)
    {
        assert(item);

        item->mouseMoveEvent(p_Event);
    }
}

void Model3D::Update()
{
    m_Scene->PushMatrix();
    m_Scene->ApplyTransformation(m_Model);

    m_TransformedModel = *m_Scene->m_Transform.GetModelMatrix();

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->Update();
    }

    m_Scene->PopMatrix();
}

void Model3D::SetZOrder(float p_ZOrder)
{
    m_Position.z = p_ZOrder;

    Reset();
    Translate(m_Position.x, m_Position.y, m_Position.z);
}

void Model3D::GatherFlatModelList()
{
    if (!m_Scene) return;

    m_Scene->m_FlatList.push_back(this);

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->GatherFlatModelList();
    }
}

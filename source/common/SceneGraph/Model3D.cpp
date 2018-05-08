#include "Model3D.h"

Model3D::Model3D(Scene3D *p_Scene, Model3D *p_Parent, const QString &p_Name, SHAPE p_ShapeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_ShapeType(p_ShapeType)
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddModel(this);
}

void Model3D::Setup()
{
    foreach(Model3D* currentModel, m_ChildList)
    {
        Model3D* model = dynamic_cast<Model3D*>(currentModel);

        if (!model) continue;

        model->Setup();
    }
}

void Model3D::Update()
{
    m_Scene->PushMatrix();
    m_Scene->ApplyTransformation(m_Model);

    m_TransformedModel = *m_Scene->m_Transform.GetModelMatrix();
    m_Scene->m_FlatList.push_back(this);

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->Update();
    }

    m_Scene->PopMatrix();
}

void Model3D::Rectangle(float p_X, float p_Y, float p_Width, float p_Height)
{
	Translate(p_X, p_Y, 0);
	m_Dimension.x = p_Width;
	m_Dimension.y = p_Height;
	//	Scale(p_Width, p_Height, 1);
}

#include "Scene3D.h"
#include "Model3D.h"

Scene3D::Scene3D(const QString& p_Name)
      : m_Frame(0),
      m_ScreenWidth(800),
      m_ScreenHeight(600)
{
}

Scene3D::~Scene3D()
{
    foreach (Model3D* currentModel, m_ModelList)
    {
        delete currentModel;
    }
}

void Scene3D::Setup()
{
	foreach(Model3D* currentModel, m_ModelList)
	{
		currentModel->Setup();
	}
}

void Scene3D::Update()
{
    foreach (Model3D* currentModel, m_ModelList)
    {
        currentModel->Update();
    }
}

void Scene3D::UpdateNew()
{
	m_FlatList.clear();

    Q_FOREACH(Model3D* item, m_ModelList)
    {
        assert(item);

        item->UpdateNew();
    }
}

void Scene3D::AddModel(Model3D* p_Model)
{
    if (p_Model && !p_Model->GetParent())
    {
        m_ModelList.push_back(p_Model);
    }
}

void Scene3D::RemoveModel(Model3D* p_Model)
{
    m_ModelList.removeAll(p_Model);
}

void Scene3D::Resize(int p_Width, int p_Height)
{
    m_ScreenWidth = p_Width;
    m_ScreenHeight = p_Height;
}

void Scene3D::SetUpProjection()
{
    m_Transform.SetMatrixMode(Transformation3D::PROJECTION_MATRIX);
    m_Transform.LoadIdentity();

    m_Transform.SetPerspective(45.0f,/* float(m_ScreenWidth)/m_ScreenHeight*/800.0f / 600.0f, 0.10f, 100.0f);
	static glm::mat4 View = glm::translate(glm::mat4(1), glm::vec3(0, 0, -15));

    m_Transform.SetMatrixMode(Transformation3D::VIEW_MATRIX);
    m_Transform.LoadIdentity();
	m_Transform.Translate(0, 0, -15);

    m_Transform.SetMatrixMode(Transformation3D::MODEL_MATRIX);
    m_Transform.LoadIdentity();
}

#include "Model3D.h"
//
//UiFL3DRenderableItem::UiFL3DRenderableItem(UiFL3DScene* p_Scene, UiFL3DRenderableItem* p_Parent, const QString& p_Name)
//    : m_Scene(p_Scene),
//      QObject(p_Parent),
//      m_Center(),
//      m_IsVisible(true),
//      m_Program(0)
//{
//    assert(p_Scene);
//
//    p_Scene->AddModel(this);
//
//    setObjectName(p_Name);
//}
//
//void UiFL3DRenderableItem::Initialize()
//{
//    foreach (QObject* objectItem, children())
//    {
//        UiFL3DRenderableItem* model = dynamic_cast<UiFL3DRenderableItem*>(objectItem);
//
//        if (!model) continue;
//
//        model->Initialize();
//    }
//}
//
//void UiFL3DRenderableItem::Render()
//{
//    if (!m_IsVisible) return;
//
//    foreach (QObject* objectItem, children())
//    {
//        UiFL3DRenderableItem* model = dynamic_cast<UiFL3DRenderableItem*>(objectItem);
//
//        if (!model || !model->m_IsVisible) continue;
//
//        model->Render();
//    }
//}
//
//void UiFL3DRenderableItem::Rotate(float p_Angle, float p_X, float p_Y, float p_Z)
//{
//    m_Transformation.translate(m_Center);
//    m_Transformation.rotate(p_Angle, p_X, p_Y, p_Z);
//    m_Transformation.translate(-m_Center);
//}
//
//void UiFL3DRenderableItem::Translate(float p_X, float p_Y, float p_Z)
//{
//    m_Transformation.translate(QVector3D(p_X, p_Y, p_Z));
//}
//
//void UiFL3DRenderableItem::Scale(float p_X, float p_Y, float p_Z)
//{
//    m_Transformation.scale(QVector3D(p_X, p_Y, p_Z));
//}
//
//void UiFL3DRenderableItem::Reset()
//{
//    m_Transformation.setToIdentity();
//}
//
//void UiFL3DRenderableItem::ApplyTransformation()
//{
//    m_Scene->Transform().GetModelMatrix() *= m_Transformation;
//}

void Model3D::UpdateNew()
{
    //if (!m_IsVisible) return;

    m_Scene->PushMatrix();
    m_Scene->ApplyTransformation(m_Model);

	m_TransformedModel = *m_Scene->m_Transform.GetModelMatrix();
    //Paint();
	m_Scene->m_FlatList.push_back(this);

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->UpdateNew();
    }

    m_Scene->PopMatrix();
}

void Model3D::Update()
{
    //		if (!m_IsVisible) return;

    foreach(Model3D* currentModel, m_ChildList)
    {
        Model3D* model = dynamic_cast<Model3D*>(currentModel);

        if (!model) continue;

        model->Update();
    }
}

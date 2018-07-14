#pragma once

#include <QObject>
#include <QMatrix4x4>

#include "Transformation3D.h"
#include "../../../common/VulkanHelper.h"
#include "SGCommon.h"

class Model3D;
class Scene3D;
class QMouseEvent;
class AbstractModelFactory;
class AbstractApp;

class Scene3D
{
public:
    Scene3D(AbstractApp* p_Application = NULL);
    virtual ~Scene3D();

    void Setup(VkCommandBuffer& p_CommandBuffer);
    void Update();
    void Render(VkCommandBuffer& p_CommandBuffer);

    void AddModel(Model3D* p_Model);
    void RemoveModel(Model3D *p_Model);

    virtual void Resize(VkCommandBuffer& p_CommandBuffer, int p_Width, int p_Height);
    virtual void SetUpProjection();
    inline Transformation3D& Transform() { return m_Transform; }

    void PushMatrix() { m_Transform.PushMatrix(); }
    void PopMatrix() { m_Transform.PopMatrix(); }
    void ApplyTransformation(const glm::mat4& m_TransformationMatrix) { *m_Transform.GetModelMatrix() *= m_TransformationMatrix; }

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

    GETSET(Model3D*, CurrentHoverItem)  // Not owned by Scene
    GETSET(AbstractApp*, Application)

    AbstractModelFactory* GetFactory(Model3D* p_Model);

private:
    void GatherFlatModelList();

public:
    int m_ScreenHeight;
    int m_ScreenWidth;

    std::vector<Model3D*> m_ModelList;
    Transformation3D m_Transform;
    int m_Frame;

    std::vector<QMatrix4x4> m_MatrixVector;
    std::vector<Model3D*> m_FlatList;
    std::set<AbstractModelFactory*> m_ModelFactories;

    std::map<SHAPE, AbstractModelFactory*> m_ShapeRenderSchemeTypeMap;
};

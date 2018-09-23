#pragma once

#include <QObject>
#include <QMap>
#include "Transformation.h"
#include "../../../common/VulkanHelper.h"
#include "Common.h"

class Node;
class Scene;
class QMouseEvent;
class RenderSchemeFactory;
class AbstractApp;

class Scene
{
public:
    Scene(AbstractApp* p_Application = NULL, const QString& p_Name = QString());
    virtual ~Scene();

    void Setup();
    void Update();
    void Render();

    void AddItem(Node* p_Item);
    void RemoveItem(Node* p_Item);

    virtual void Resize(int p_Width, int p_Height);
    virtual void SetUpProjection();
    inline Transformation& Transform() { return m_Transform; }

    void PushMatrix() { m_Transform.PushMatrix(); }
    void PopMatrix() { m_Transform.PopMatrix(); }
    void ApplyTransformation(const glm::mat4& m_TransformationMatrix) { *m_Transform.GetModelMatrix() *= m_TransformationMatrix; }

    bool IsDirty() { return (m_DirtyType != SCENE_DIRTY_TYPE::NONE); }
    SCENE_DIRTY_TYPE GetDirtyType() { return m_DirtyType; }
    void SetDirtyType(SCENE_DIRTY_TYPE p_InvalidateType) { m_DirtyType = p_InvalidateType; }

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
    RenderSchemeFactory* GetRenderSchemeFactory(Node* p_Item);

public: // Parminder: Todo this public method should not be visible to the outside world. Covert recurvise gathering of node list to iterative
    void AppendToFlatNodeList(Node* p_Item);

private:
    void GatherFlatNodeList();

    std::vector<Node*>                      m_NodeList;
    std::vector<Node*>                      m_FlatList;
    std::set<RenderSchemeFactory*>          m_RenderSchemeFactorySet;
    std::map<SHAPE, RenderSchemeFactory*>   m_ShapeRenderSchemeTypeMap;

    GETSET(QString,                         Name)
    GETSET(Node*,                           CurrentHoverItem)  // Not owned by Scene
    GETSET(AbstractApp*,                    Application)
    GETSET(int,                             ScreenHeight);
    GETSET(int,                             ScreenWidth);
    GETSET(int,                             Frame);
    GETSET(Transformation,                  Transform);

private:
    SCENE_DIRTY_TYPE m_DirtyType;
};

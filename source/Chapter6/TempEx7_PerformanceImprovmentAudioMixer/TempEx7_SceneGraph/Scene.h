#pragma once

#include <QObject>
#include <QMatrix4x4>

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
    Scene(AbstractApp* p_Application = NULL);
    virtual ~Scene();

    void Setup();
    void Update();
    void Render();

    void AddModel(Node* p_Model);
    void RemoveModel(Node *p_Model);

    void Resize(int p_Width, int p_Height);
    void SetUpProjection();
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

    GETSET(glm::mat4*, Projection)	    // Not owned by Scene, double check this can be owned. TODO: PS
    GETSET(glm::mat4*, View)		    // Not owned by Scene
    GETSET(Node*, CurrentHoverItem)	// Not owned by Scene
    GETSET(AbstractApp*, Application)
    
    RenderSchemeFactory* GetFactory(Node* p_Model);

private:
    void GatherFlatList();

public:
    int m_ScreenHeight;
    int m_ScreenWidth;

    std::vector<Node*> m_ModelList;
    Transformation m_Transform;
    int m_Frame;

    std::vector<QMatrix4x4> m_MatrixVector;
    std::vector<Node*> m_FlatList;
    std::set<RenderSchemeFactory*> m_ModelFactories;

    typedef std::map<RENDER_SCEHEME_TYPE, RenderSchemeFactory*> RenderSchemeTypeMap;
    std::map<SHAPE, RenderSchemeTypeMap*> m_ShapeRenderSchemeTypeMap;

private:
    SCENE_DIRTY_TYPE m_DirtyType;
};

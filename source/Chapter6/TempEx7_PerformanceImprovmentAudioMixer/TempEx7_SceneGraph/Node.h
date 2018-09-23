#pragma once

#include "Scene.h"
#include "RenderSchemeFactory.h"
#include "../common/VulkanHelper.h"

class VulkanApp;

class Node
{
public:
    Node(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE, RENDER_SCEHEME_TYPE p_RenderSchemeType = RENDER_SCEHEME_TYPE::RENDER_SCEHEME_INSTANCED);
    ~Node();

    virtual void Setup();
    virtual void Update(Node* p_Item = NULL);
    virtual RenderSchemeFactory* GetRenderSchemeFactory() { return NULL; } // Custom model class do not need to implement it as they are made of basic model classes.

    void Rotate(float p_Angle, float p_X, float p_Y, float p_Z);
    void Translate(float p_X, float p_Y, float p_Z);
    void Scale(float p_X, float p_Y, float p_Z);
    void Reset() { m_ModelTransformation = glm::mat4(); } // Load identity
    void ResetPosition(); // Reset the tranformation to identity first, then set the position m_BoundedRegion.m_Position.x/y/z 
    void SetZOrder(float p_ZOrder);
    void SetPosition(float p_X, float p_Y);
    void SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);
    void ApplyTransformation();
    glm::mat4 GetAbsoluteTransformations() const;
    glm::mat4 GetParentsTransformation(Node* p_Parent) const;

    inline Node* GetParent() const;
    void GatherFlatNodeList();

    bool IsDirty() { return (m_DirtyType != DIRTY_TYPE::NONE); }
    DIRTY_TYPE GetDirtyType() { return m_DirtyType; }
    void SetDirtyType(DIRTY_TYPE p_InvalidateType);

    const glm::vec4& GetColor() const { return m_Color; }
    void SetColor(const glm::vec4& p_Color) { m_Color = p_Color; SetDirtyType(DIRTY_TYPE::ATTRIBUTES); }

    const BoundingRegion& GetBoundedRegion() const { return m_BoundedRegion; }
    void SetBoundedRegion(const BoundingRegion& p_BoundingRegion)
    {
        m_BoundedRegion.m_Position = p_BoundingRegion.m_Position;
        m_BoundedRegion.m_Dimension = p_BoundingRegion.m_Dimension;
        SetDirtyType(DIRTY_TYPE::ATTRIBUTES);
    }

    // Event Management
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual bool mouseMoveEvent(QMouseEvent* p_Event);
    virtual void mouseDoubleClickEvent(QMouseEvent* p_Event) UNIMPLEMENTED_INTEFACE

    // Key interaction: Dummy interface for now.
    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE

    // Application Window resizing
    virtual void ResizeWindow(int width, int height) {}

    GETSET(QString,                     Name)
    GETSET(SHAPE,                       ShapeType)
    GETSET(RENDER_SCEHEME_TYPE,         RenderSchemeType);
    GETSET(glm::vec4,                   DefaultColor)
    GETSET(glm::mat4,                   ModelTransformation)
    GETSET(glm::vec3,                   OriginOffset)
    GETSET(glm::mat4,                   AbsoluteTransformation)
    GETSET(Scene*,                      Scene)
    GETSET(Node*,                       Parent)
    GETSET(QList<Node*>,                ChildList)
    GETSET(bool,                        Visible)
    GETSET(unsigned int,                GpuMemOffset)  // TODO the data type should be unsigned long long to accomodate large offsets

    void QuerySupportedSchemes() { } // this function must tell the user what schemes are supported

protected:
    DIRTY_TYPE m_DirtyType;

    // Attributes Start
    glm::vec4 m_Color;
    BoundingRegion m_BoundedRegion;
    // Attributes End
};

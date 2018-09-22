#pragma once

#define M_PI (3.14)
#define M_PI_2 (3.14 * 2)

#include "Scene.h"
#include "RenderSchemeFactory.h"
#include "../common/VulkanHelper.h"
//#include "Common.h"

class VulkanApp;

class Node
{
public:
    Node(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE, RENDER_SCEHEME_TYPE p_RenderSchemeType = RENDER_SCEHEME_TYPE::RENDER_SCEHEME_INSTANCED);
    ~Node();
    virtual void Setup();
    void Update();
    virtual RenderSchemeFactory* GetRenderScemeFactory() { return NULL; } // Custom model class do not need to implement it as they are made of basic model classes.

	void Rotate(float p_Angle, float p_X, float p_Y, float p_Z) { m_Model = glm::rotate(m_Model, p_Angle, glm::vec3(p_X, p_Y, p_Z)); }
	void Translate(float p_X, float p_Y, float p_Z) { m_Model = glm::translate(m_Model, glm::vec3(p_X, p_Y, p_Z)); }
    void Scale(float p_X, float p_Y, float p_Z) { m_Model = glm::scale(m_Model, glm::vec3(p_X, p_Y, p_Z)); }
    void Reset() { m_ModelTransformation = glm::mat4(); }

    //void Rectangle(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);
    void SetZOrder(float p_ZOrder);
    void SetPosition(float p_X, float p_Y);
    void SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);

    bool IsDirty() { return (m_DirtyType != DIRTY_TYPE::NONE); }
    DIRTY_TYPE GetDirtyType() { return m_DirtyType; }
    void SetDirtyType(DIRTY_TYPE p_InvalidateType);

    const glm::vec4& GetColor() const { return m_Color; }
    void SetColor(const glm::vec4& p_Color) { m_Color = p_Color; SetDirtyType(DIRTY_TYPE::ATTRIBUTES); }

//    const glm::vec3& GetPosition() const { return m_Position; }
//    void SetPosition(const glm::vec3& p_Position) { m_Position = p_Position; SetDirtyType(DIRTY_TYPE::ATTRIBUTES); }

//    const glm::vec2& GetDimension() const { return m_Dimension; }
//    void SetDimension(const glm::vec2& p_Dimension) { m_Dimension = p_Dimension; SetDirtyType(DIRTY_TYPE::ATTRIBUTES); }

    const BoundingRegion& GetBoundedRegion() const { return m_BoundedRegion; }
    void SetBoundedRegion(const BoundingRegion& p_BoundingRegion)
    {
        m_BoundedRegion.m_Position = p_BoundingRegion.m_Position;
        m_BoundedRegion.m_Dimension = p_BoundingRegion.m_Dimension;
        SetDirtyType(DIRTY_TYPE::ATTRIBUTES);
    }


    GETSET(SHAPE, ShapeType);
    GETSET(RENDER_SCEHEME_TYPE, RenderSchemeType);
    GETSET(glm::vec4, DefaultColor)
    GETSET(glm::mat4, Model)            // Owned by drawable item
    GETSET(Scene*, Scene)
    GETSET(Node*, Parent)
    GETSET(glm::mat4, ModelTransformation) // Owned by drawable item
    GETSET(unsigned int, GpuMemOffset)  // TODO the data type should be unsigned long long to accomodate large offsets
    GETSET(bool, Visible)

    // Mouse interaction: Dummy interface for now.
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual bool mouseMoveEvent(QMouseEvent* p_Event);
    virtual void mouseDoubleClickEvent(QMouseEvent* p_Event) UNIMPLEMENTED_INTEFACE

    // Key interaction: Dummy interface for now.
    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE

    // Application Window resizing
    virtual void ResizeWindow(int width, int height) {}

    inline Node* GetParent() const { return m_Parent; }
    void ApplyTransformation() { *m_Scene->Transform().GetModelMatrix() *= m_Model; }
    glm::mat4 GetRelativeTransformations() const { return GetParentsTransformation(GetParent()) * m_Model; }
    glm::mat4 GetParentsTransformation(Node* p_Parent) const { return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_Model) : glm::mat4(); }

    void GatherFlatList();

    //AbstractModelFactory* m_AbstractFactory; // REMOVE ME

protected:
    DIRTY_TYPE m_DirtyType;

    // Attributes Start
    glm::vec4 m_Color;
//    glm::vec3 m_Position;
//    glm::vec2 m_Dimension;
    BoundingRegion m_BoundedRegion;
    // Attributes End

private:
    QList<Node*> m_ChildList;
};

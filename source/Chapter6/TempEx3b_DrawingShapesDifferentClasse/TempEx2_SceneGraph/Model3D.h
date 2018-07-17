#pragma once

#include "Scene3D.h"
#include "../common/VulkanHelper.h"
#include "SGCommon.h"

class VulkanApp;

class RenderSchemeFactory
{
public:
    RenderSchemeFactory() {}
    virtual ~RenderSchemeFactory() {}

    virtual void Setup(VkCommandBuffer& p_CommandBuffer) {}
    virtual void Update() {}
    virtual void Render(VkCommandBuffer& p_CommandBuffer) {}
    virtual void Prepare(Scene3D* p_Scene) {}
    virtual void UpdateModelList(DrawItem* p_Parent) {}
    virtual void ResizeWindow(VkCommandBuffer& p_CommandBuffer) {}

    GETSET(glm::mat4x4, ProjectViewMatrix);
    GETSET(VulkanApp*, VulkanApplication)

protected:
    QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;
};

struct BoundingRegion
{
    BoundingRegion(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0) // For 2D Bounding Box
    {
        m_Position.x = p_X;         m_Position.y = p_Y;        m_Position.z = p_ZOrder;
        m_Dimension.x = p_Width;    m_Dimension.y = p_Height;  m_Dimension.z = p_ZOrder;
    }
    
    BoundingRegion(float p_X, float p_Y, float p_Z, float p_Width, float p_Height, float p_Depth)
    {
        m_Position.x = p_X;         m_Position.y = p_X;        m_Position.z = p_Z;
        m_Dimension.x = p_Width;    m_Dimension.y = p_Height;  m_Dimension.z = p_Depth;
    }

    glm::vec3 m_Position;
    glm::vec3 m_Dimension;
};

class DrawItem
{
public:
    DrawItem(Scene3D* p_Scene, DrawItem* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE);

    virtual void Setup();
    virtual void Update(DrawItem* p_Item = NULL);
    virtual RenderSchemeFactory* GetRenderSchemeFactory() { return NULL; } // Custom model class do not need to implement it as they are made of basic model classes.

    void Rotate(float p_Angle, float p_X, float p_Y, float p_Z);
    void Translate(float p_X, float p_Y, float p_Z);
    void Scale(float p_X, float p_Y, float p_Z);

    void Reset() { m_Model = glm::mat4(); }
    void SetZOrder(float p_ZOrder);
    void SetPosition(float p_X, float p_Y);
    void SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);

    GETSET(QString,             Name)
    GETSET(SHAPE,               ShapeType);
    GETSET(BoundingRegion,      BoundedRegion)
    GETSET(glm::vec4,           Color)
    GETSET(glm::vec4,           DefaultColor)
    GETSET(glm::mat4,           Model)              // Owned by drawable item
    GETSET(Scene3D*,            Scene)
    GETSET(DrawItem*,           Parent)
    GETSET(glm::vec3,           OriginOffset)
    GETSET(glm::mat4,           TransformedModel)   // Owned by drawable item
    GETSET(QList<DrawItem*>,    ChildList)

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual bool mouseMoveEvent(QMouseEvent* p_Event);
    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE
    virtual void ResizeWindow(int width, int height) {}

    inline DrawItem* GetParent() const { return m_Parent; }
    void ApplyTransformation() { *m_Scene->Transform().GetModelMatrix() *= m_Model; }
    glm::mat4 GetRelativeTransformations() const { return GetParentsTransformation(GetParent()) * m_Model; }
    glm::mat4 GetParentsTransformation(DrawItem* p_Parent) const { return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_Model) : glm::mat4(); }

    void GatherDrawItemsFlatList(); // Rename: GatherFlatRenderItemList
};

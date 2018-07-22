#pragma once

#include "Scene.h"
#include "../common/VulkanHelper.h"

class VulkanApp;

class RenderSchemeFactory
{
public:
    RenderSchemeFactory() {}
    virtual ~RenderSchemeFactory() {}

    virtual void Setup() {}
    virtual void Update() {}
    virtual void Render(VkCommandBuffer& p_CommandBuffer) {}
    virtual void Prepare(Scene* p_Scene) {}
    virtual void UpdateNodeList(Node* p_Parent) {}
    virtual void ResizeWindow(VkCommandBuffer& p_CommandBuffer) {}

    GETSET(glm::mat4x4, ProjectViewMatrix);
    GETSET(VulkanApp*, VulkanApplication)

protected:
    QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;
};

class Node
{
public:
    Node(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE);

    virtual void Setup();
    virtual void Update(Node* p_Item = NULL);
    virtual RenderSchemeFactory* GetRenderSchemeFactory() { return NULL; } // Custom model class do not need to implement it as they are made of basic model classes.

    void Rotate(float p_Angle, float p_X, float p_Y, float p_Z);
    void Translate(float p_X, float p_Y, float p_Z);
    void Scale(float p_X, float p_Y, float p_Z);

    void Reset() { m_Model = glm::mat4(); }
    void SetZOrder(float p_ZOrder);
    void SetPosition(float p_X, float p_Y);
    void SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);

    inline Node* GetParent() const;
    void ApplyTransformation();
    glm::mat4 GetRelativeTransformations() const;
    glm::mat4 GetParentsTransformation(Node* p_Parent) const;
    void GatherFlatNodeList();

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);
    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE
    virtual void ResizeWindow(int width, int height) {}

    GETSET(QString,                     Name)
    GETSET(SHAPE,                       ShapeType);
    GETSET(BoundingRegion,              BoundedRegion)
    GETSET(glm::vec4,                   Color)
    GETSET(glm::vec4,                   DefaultColor)
    GETSET(glm::mat4,                   Model)              // Owned by drawable item
    GETSET(glm::vec3,                   OriginOffset)
    GETSET(glm::mat4,                   TransformedModel)   // Owned by drawable item
    GETSET(Scene*,                      Scene)
    GETSET(Node*,                       Parent)
    GETSET(QList<Node*>,                ChildList)
};

#pragma once

#include "Scene3D.h"
#include "../common/VulkanHelper.h"
#include "SGCommon.h"

class VulkanApp;

class AbstractModelFactory
{
public:
    AbstractModelFactory() {}
    virtual ~AbstractModelFactory() {}

    virtual void Setup(VkCommandBuffer& p_CommandBuffer) {}
    virtual void Update() {}
    virtual void Render(VkCommandBuffer& p_CommandBuffer) {}
    virtual void Prepare(Scene3D* p_Scene) {}
    virtual void UpdateModelList(Model3D* p_Parent) {}
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

class Model3D
{
public:
    Model3D(Scene3D* p_Scene, Model3D* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE);

    virtual void Setup();
    virtual void Update(Model3D* p_Item = NULL);

    virtual AbstractModelFactory* GetRenderScemeFactory() { return NULL; } // Custom model class do not need to implement it as they are made of basic model classes.

    virtual void Rotate(float p_Angle, float p_X, float p_Y, float p_Z) 
    { 
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, m_OriginOffset); }
        m_Model = glm::rotate(m_Model, p_Angle, glm::vec3(p_X, p_Y, p_Z));
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, -m_OriginOffset); }
    }
    
    void Translate(float p_X, float p_Y, float p_Z) 
    { 
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, m_OriginOffset); }
        m_Model = glm::translate(m_Model, glm::vec3(p_X, p_Y, p_Z));
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, -m_OriginOffset); }
    }

    void Scale(float p_X, float p_Y, float p_Z) 
    { 
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, m_OriginOffset); }
        m_Model = glm::scale(m_Model, glm::vec3(p_X, p_Y, p_Z));
        if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f)) { m_Model = glm::translate(m_Model, -m_OriginOffset); }
    }

    void Reset() { m_Model = glm::mat4(); }

    void SetZOrder(float p_ZOrder);
    void SetPosition(float p_X, float p_Y);

    GETSET(QString, Name)
    GETSET(SHAPE, ShapeType);
    GETSET(BoundingRegion, BoundedRegion)
    GETSET(glm::vec4, Color)
    GETSET(glm::vec4, DefaultColor)
    GETSET(glm::mat4, Model)		// Owned by drawable item
    GETSET(Scene3D*, Scene)
    GETSET(Model3D*, Parent)
    GETSET(glm::vec3, OriginOffset)
    GETSET(glm::mat4, TransformedModel)		// Owned by drawable item
    GETSET(QList<Model3D*>, ChildList)

    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual bool mouseMoveEvent(QMouseEvent* p_Event);

    // Key interaction: Dummy interface for now.
    virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE

    // Application Window resizing
    virtual void ResizeWindow(int width, int height) {}

    inline Model3D* GetParent() const { return m_Parent; }
    void ApplyTransformation() { *m_Scene->Transform().GetModelMatrix() *= m_Model; }
    glm::mat4 GetRelativeTransformations() const { return GetParentsTransformation(GetParent()) * m_Model; }
    glm::mat4 GetParentsTransformation(Model3D* p_Parent) const { return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_Model) : glm::mat4(); }

    void GatherFlatModelList(); // Rename: GatherFlatRenderItemList

protected:
public:
    void SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder = 0.0f);
};

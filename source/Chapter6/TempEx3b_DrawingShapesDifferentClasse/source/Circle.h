#pragma once
#include "../../../common/VulkanApp.h"
#include "Rect.h"

#include "../TempEx2_SceneGraph/Transformation3D.h"
#include "../TempEx2_SceneGraph/Scene3D.h"
#include "../TempEx2_SceneGraph/Model3D.h"
class CircleMultiDrawFactory;

class Circle : public RectangleModel
{
public:
    Circle(Scene3D* p_Scene, Model3D* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "") 
        : RectangleModel(p_Scene, p_Parent, p_BoundedRegion, p_Name)
    {
        SetShapeType(SHAPE::SHAPE_CIRCLE_MULTIDRAW);
    }

    Circle(Scene3D* p_Scene, Model3D* p_Parent, glm::vec2 m_Center, float radius, const QString& p_Name = "")
        : RectangleModel(p_Scene, p_Parent, BoundingRegion(m_Center.x - (radius * 0.5f), m_Center.y - (radius * 0.5f), radius, radius), p_Name)
    {
        SetShapeType(SHAPE::SHAPE_CIRCLE_MULTIDRAW);
    }

    virtual ~Circle() {}

    virtual void Setup();
    void CreateVertexBuffer();

    AbstractModelFactory* GetRenderScemeFactory();
};

struct CircleDescriptorSet : public RectangleDescriptorSet
{
    CircleDescriptorSet(VulkanApp* p_VulkanApplication)
        : RectangleDescriptorSet(p_VulkanApplication)
    {
    }
};

class CircleMultiDrawFactory : public AbstractModelFactory
{
public:
    CircleMultiDrawFactory(VulkanApp* p_VulkanApp);
    virtual ~CircleMultiDrawFactory();

public:
    virtual void Setup(VkCommandBuffer& p_CommandBuffer);
    virtual void Update();
    virtual void Render(VkCommandBuffer& p_CmdBuffer);

    void ResizeWindow(VkCommandBuffer& p_CommandBuffer);
    virtual void Prepare(Scene3D* p_Scene);

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateRectFillPipeline();
    void CreateRectOutlinePipeline();

    void createPushConstants();

    void CreateVertexBuffer();

    //void Render(VkCommandBuffer& p_CmdBuffer);

    virtual void UpdateModelList(Model3D* p_Item);

    QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    typedef std::vector<Model3D*> ModelVector;
    ModelVector m_PipelineTypeModelVector[PIPELINE_COUNT];

    std::shared_ptr<CircleDescriptorSet> CDS;
};
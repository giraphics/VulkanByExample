#pragma once
#include "../../../common/VulkanApp.h"
#include "Rect.h"

#include "../TempEx2_SceneGraph/Transformation3D.h"
#include "../TempEx2_SceneGraph/Scene3D.h"
#include "../TempEx2_SceneGraph/Model3D.h"
class CircleMultiDrawFactory;

class Circle : public Model3D
{
public:
    enum DRAW_TYPE
    {
        FILLED = 0,
        OUTLINE,
        ROUNDED,
        DRAW_TYPE_COUNT
    };

public:
    Circle(Scene3D* p_Scene, Model3D* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "") 
//        : RectangleModel(p_Scene, p_Parent, p_BoundedRegion, p_Name)
        : Model3D(p_Scene, p_Parent, p_BoundedRegion, p_Name, SHAPE_CIRCLE_MULTIDRAW)
        , m_DrawType(FILLED)
    {
        //SetShapeType(SHAPE::SHAPE_CIRCLE_MULTIDRAW);
    }

    Circle(Scene3D* p_Scene, Model3D* p_Parent, glm::vec2 m_Center, float radius, const QString& p_Name = "")
        : Model3D(p_Scene, p_Parent, BoundingRegion(m_Center.x - (radius * 0.5f), m_Center.y - (radius * 0.5f), radius, radius), p_Name, SHAPE_CIRCLE_MULTIDRAW)
        , m_DrawType(FILLED)
    {
    }

protected:
    virtual void Update(Model3D* p_Item = NULL)
    {
        Model3D::Update(p_Item);

        CreateCircleVertexBuffer();
    }

public:
    void UpdateMeAndMyChildren()
    {
        Update(this);
    }

    virtual ~Circle() {}

    GETSET(DRAW_TYPE, DrawType)

    AbstractModelFactory* GetRenderScemeFactory();

    virtual void Setup();
    void CreateCircleVertexBuffer();
    VulkanBuffer m_VertexBuffer;
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
    void CreateCircleFillPipeline();
    void CreateCircleOutlinePipeline();

    void createPushConstants();

    void CreateVertexLayoutBinding();

    virtual void UpdateModelList(Model3D* p_Item);


    enum CIRCLE_GRAPHICS_PIPELINES
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

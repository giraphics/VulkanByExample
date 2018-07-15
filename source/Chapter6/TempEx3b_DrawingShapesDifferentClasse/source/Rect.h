#pragma once
#include "../../../common/VulkanApp.h"

#include "../TempEx2_SceneGraph/Transformation3D.h"
#include "../TempEx2_SceneGraph/Scene3D.h"
#include "../TempEx2_SceneGraph/Model3D.h"

class RectangleModel : public Model3D
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
    RectangleModel(Scene3D* p_Scene, Model3D* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");
    virtual ~RectangleModel() {}

    GETSET(DRAW_TYPE, DrawType)
        
protected:
    virtual void Update(Model3D* p_Item = NULL)
    {
        Model3D::Update(p_Item);

        CreateRectVertexBuffer();
    }

public:
    void UpdateMeAndMyChildren()
    {
        Update(this);
    }

    AbstractModelFactory* GetRenderScemeFactory();

    virtual void Setup();
    void CreateRectVertexBuffer();
    VulkanBuffer m_VertexBuffer;
};

struct RectangleDescriptorSet
{
    struct UniformBufferObj {
        UniformBufferObj()
        {
            memset(this, 0, sizeof(UniformBufferObj));
        }

        VulkanBuffer					m_BufObj;
        VkDescriptorBufferInfo			m_DescriptorBufInfo;// Descriptor buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
        std::vector<VkMappedMemoryRange>m_MappedRange;		// Metadata of memory mapped objects
        uint8_t*						m_MappedMemory;  	// Host pointer containing the mapped device address which is used to write data into.
        size_t							m_DataSize;			// Data size.
    };

    RectangleDescriptorSet(VulkanApp* p_VulkanApplication)
    {
        m_VulkanApplication = p_VulkanApplication;

        UniformBuffer = new UniformBufferObj;

        CreateDescriptor();
    }

    ~RectangleDescriptorSet()
    {
        // Destroy descriptors
        for (int i = 0; i < descLayout.size(); i++) {
        	vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
        }
        descLayout.clear();

        vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
        vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);
    }

    void CreateUniformBuffer();
    void DestroyUniformBuffer();

    void CreateDescriptorSetLayout();
    void DestroyDescriptorLayout();

    UniformBufferObj* UniformBuffer;

    // List of all the VkDescriptorSetLayouts 
    std::vector<VkDescriptorSetLayout> descLayout;

    // List of all created VkDescriptorSet
    std::vector<VkDescriptorSet> descriptorSet;

private:
    void CreateDescriptor();

    // Creates the descriptor pool, this function depends on - 
    void CreateDescriptorPool();
    // Creates the descriptor sets using descriptor pool.
    // This function depend on the createDescriptorPool() and createUniformBuffer().
    void CreateDescriptorSet();

    // Decriptor pool object that will be used for allocating VkDescriptorSet object
    VkDescriptorPool descriptorPool;

    VulkanApp*		 m_VulkanApplication;
};

class RectangleMultiDrawFactory : public AbstractModelFactory
{
public:
    RectangleMultiDrawFactory(VulkanApp* p_VulkanApp);
    virtual ~RectangleMultiDrawFactory();

public:
    virtual void Setup(VkCommandBuffer& p_CommandBuffer);
    virtual void Update();
    virtual void Render(VkCommandBuffer& p_CmdBuffer);

    void ResizeWindow(VkCommandBuffer& p_CmdBuffer);
    virtual void Prepare(Scene3D* p_Scene);

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateRectFillPipeline();
    void CreateRectOutlinePipeline();

    void createPushConstants();

    void CreateVertexLayoutBinding();

    virtual void UpdateModelList(Model3D* p_Item);

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

    std::shared_ptr<RectangleDescriptorSet> CDS;
};

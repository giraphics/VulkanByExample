#pragma once
#include "../../../common/VulkanApp.h"

#include "../DrawingShapes_SceneGraph/Transformation.h"
#include "../DrawingShapes_SceneGraph/Scene.h"
#include "../DrawingShapes_SceneGraph/Node.h"

class Rectangl : public Node
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
    Rectangl(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "");
    virtual ~Rectangl() {}

    GETSET(DRAW_TYPE, DrawType)
        
protected:
    virtual void Update(Node* p_Item = NULL)
    {
        Node::Update(p_Item);

        CreateRectVertexBuffer();
    }

public:
    void UpdateMeAndMyChildren()
    {
        Update(this);
    }

    RenderSchemeFactory* GetRenderSchemeFactory();

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

        VulkanBuffer                    m_BufObj;
        VkDescriptorBufferInfo          m_DescriptorBufInfo;// Descriptor buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
        std::vector<VkMappedMemoryRange>m_MappedRange;      // Metadata of memory mapped objects
        uint8_t*                        m_MappedMemory;     // Host pointer containing the mapped device address which is used to write data into.
        size_t                          m_DataSize;         // Data size.
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
        for (int i = 0; i < descLayout.size(); i++)
        {
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

class RectangleMultiDrawScheme : public RenderSchemeFactory
{
public:
    RectangleMultiDrawScheme(VulkanApp* p_VulkanApp);
    virtual ~RectangleMultiDrawScheme();

public:
    virtual void Setup();
    virtual void Update();
    virtual void Render(VkCommandBuffer& p_CommandBuffer);

    void ResizeWindow(VkCommandBuffer& p_CommandBuffer);
    virtual void Prepare(Scene* p_Scene);

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateRectFillPipeline();
    void CreateRectOutlinePipeline();
    void CreateVertexLayoutBinding();

    void createPushConstants();

    virtual void UpdateNodeList(Node* p_Item);
    virtual void RemoveNodeList(Node* p_Model);

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    typedef std::vector<Node*> NodeVector;
    NodeVector m_PipelineTypeModelVector[PIPELINE_COUNT];

    std::shared_ptr<RectangleDescriptorSet> m_DescriptorSet;
};

class RectangleInstancingScheme : public RenderSchemeFactory
{
public:
    RectangleInstancingScheme(VulkanApp* p_VulkanApp);
    virtual ~RectangleInstancingScheme();

public:
    virtual void Setup();
    virtual void Update();
    virtual void UpdateDirty();
    virtual void Render() { RecordCommandBuffer(); }

    void ResizeWindow(int width, int height);

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
    void CreateRectFillPipeline();
    void CreateRectOutlinePipeline();

    void RecordCommandBuffer();
    void CreateVertexBuffer();

    void Render(VkCommandBuffer& p_CmdBuffer);

    virtual void UpdateNodeList(Node* p_Item);
    virtual void RemoveNodeList(Node* p_Item);

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    ////////////////////////////////////////////////////////////////
public:
    void PrepareInstanceData(RECTANGLE_GRAPHICS_PIPELINES p_Pipeline = PIPELINE_COUNT);
    void UpdateDirtyInstanceData();


    // Per-instance data block
    struct InstanceData {
        glm::mat4 m_Model;
        glm::vec4 m_Rect;
        glm::vec4 m_Color;
        uint m_BoolFlags; // [0] Visibility [1] Unused [2] Unused [3] Unused
    };

    VulkanBuffer m_VertexBuffer[PIPELINE_COUNT];
    int m_VertexCount[PIPELINE_COUNT];
    typedef std::vector<Node*> ModelVector;
    VulkanBuffer m_InstanceBuffer[PIPELINE_COUNT];
    ModelVector m_PipelineTypeModelVector[PIPELINE_COUNT];
    int m_OldInstanceDataSize[PIPELINE_COUNT];
};
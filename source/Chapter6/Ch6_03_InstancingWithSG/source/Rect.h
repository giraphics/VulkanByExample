#pragma once
#include "../../../common/VulkanApp.h"

#include "../../../common/SceneGraph/Transformation3D.h"
#include "../../../common/SceneGraph/Scene3D.h"
#include "../../../common/SceneGraph/Model3D.h"

struct Vertex
{
    glm::vec3 m_Position; // Vertex Position => x, y, z
    glm::vec3 m_Color;    // Color format => r, g, b
};

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
    RectangleModel(VulkanApp* p_VulkanApp/*REMOVE ME*/, Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE, RENDER_SCEHEME_TYPE p_RenderSchemeType = RENDER_SCEHEME_INSTANCED);
    virtual ~RectangleModel() {}
    GETSET(DRAW_TYPE, DrawType)

    virtual AbstractModelFactory* GetRenderScemeFactory();
};

struct CubeDescriptorSet
{
    struct  UniformBufferObj {
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

    CubeDescriptorSet(VulkanApp* p_VulkanApplication)
    {
        m_VulkanApplication = p_VulkanApplication;

        UniformBuffer = new UniformBufferObj;
    }
    ~CubeDescriptorSet()
    {
        //// Destroy descriptors
        //for (int i = 0; i < descLayout.size(); i++) {
        //	vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
        //}
        //descLayout.clear();

        //vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
        //vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);
    }

    void CreateUniformBuffer();
    void DestroyUniformBuffer();

    void CreateDescriptorSetLayout();
    void DestroyDescriptorLayout();

    void CreateDescriptor();

    // Creates the descriptor pool, this function depends on - 
    void CreateDescriptorPool();
    // Creates the descriptor sets using descriptor pool.
    // This function depend on the createDescriptorPool() and createUniformBuffer().
    void CreateDescriptorSet();

    // List of all the VkDescriptorSetLayouts 
    std::vector<VkDescriptorSetLayout> descLayout;

    // Decriptor pool object that will be used for allocating VkDescriptorSet object
    VkDescriptorPool descriptorPool;

    // List of all created VkDescriptorSet
    std::vector<VkDescriptorSet> descriptorSet;

    VulkanApp*		 m_VulkanApplication;
    UniformBufferObj* UniformBuffer;
};

class RectangleFactory : public AbstractModelFactory
{
public:
    RectangleFactory(VulkanApp* p_VulkanApp);
    virtual ~RectangleFactory();

public:
    void Setup();
    void Update();

private:
    void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
        void CreateRectFillPipeline();
        void CreateRectOutlinePipeline();
    void CreateVertexBuffer();

    void RecordCommandBuffer(); // made public
    virtual void UpdateModelList(Model3D* p_Item) 
    {
        //m_ModelList.push_back(p_Item);

        RectangleModel* rectangle = dynamic_cast<RectangleModel*>(p_Item);
        assert(rectangle);

        // Note: Based on the draw type push the model in respective pipelines
        // Keep the draw type loose couple with the pipeline type, 
        // they may be in one-to-one correspondence but that is not necessary.
        switch (rectangle->GetDrawType())
        {
            case RectangleModel::FILLED:
                m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
                break;

            case RectangleModel::OUTLINE:
                m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
                break;

            case RectangleModel::ROUNDED:
                // TODO
                break;

            default:
                break;
        }
    }

public:
    void PrepareInstanceData();

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

    std::vector<VkVertexInputBindingDescription>   m_VertexInputBinding[PIPELINE_COUNT];   // 0 for (position and color) 1 for ()
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttribute[PIPELINE_COUNT]; // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

    VulkanApp* m_VulkanApplication;

    QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;
    // Per-instance data block
    struct InstanceData {
        glm::mat4 m_Model;
        glm::vec4 m_Rect;
        glm::vec4 m_Color;
    };


    std::set<RECTANGLE_GRAPHICS_PIPELINES> m_ActivePipelines;
    
    typedef std::vector<Model3D*> ModelVector;

    VulkanBuffer m_VertexBuffer[PIPELINE_COUNT], m_InstanceBuffer[PIPELINE_COUNT];
    ModelVector m_PipelineTypeModelVector[PIPELINE_COUNT];
    int m_OldInstanceDataSize[PIPELINE_COUNT];
    int m_VertexCount[PIPELINE_COUNT];
};


//TODO: 
//a. VertexBuffer should be m_VertexBuffer[PIPELINE_COUNT]
//b. Address its distruction.
//c. Assign correct vertex buffer data as per pipeline rectFilledVertices, rectOutlineVertices
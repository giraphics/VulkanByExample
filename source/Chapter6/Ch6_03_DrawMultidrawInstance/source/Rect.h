#pragma once
#include "../../../common/VulkanApp.h"

#include "../SceneGraph/Transformation3D.h"
#include "../SceneGraph/Scene3D.h"
#include "../SceneGraph/Model3D.h"

struct Vertex
{
    glm::vec3 m_Position; // Vertex Position => x, y, z
    glm::vec3 m_Color;    // Color format => r, g, b
};

const float dimension = 10.0f;
static const Vertex rectVertices[] =
{
    { glm::vec3( dimension, -dimension, -dimension), glm::vec3(0.f, 0.f, 0.f) },
    { glm::vec3(-dimension, -dimension, -dimension), glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3( dimension,  dimension, -dimension), glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3( dimension,  dimension, -dimension), glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(-dimension, -dimension, -dimension), glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3(-dimension,  dimension, -dimension), glm::vec3(1.f, 1.f, 0.f) },
};

class RectangleModel : public Model3D
{
public:
    //enum DRAW_TYPE
    //{
    //    FILLED = 0,
    //    OUTLINE,
    //    ROUNDED,
    //    DRAW_TYPE_COUNT
    //};

public:
    RectangleModel(VulkanApp* p_VulkanApp, Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE, RENDER_SCEHEME_TYPE p_RenderSchemeType = RENDER_SCEHEME_MULTIDRAW);
    virtual ~RectangleModel() {}
    //GETSET(DRAW_TYPE, DrawType)

	AbstractModelFactory* GetRenderScemeFactory();

	virtual void Setup();

    void CreateVertexBuffer(const void * vertexData, uint32_t dataSize, uint32_t dataStride);
    void Render(VkCommandBuffer& p_CmdBuffer);

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
	}

    ~RectangleDescriptorSet()
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

class RectangleMultiDrawFactory : public AbstractModelFactory
{
public:
    RectangleMultiDrawFactory(VulkanApp* p_VulkanApp);
    virtual ~RectangleMultiDrawFactory();

	//RectangleModel* GetModel(VulkanApp* p_VulkanApp, Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE)
	//{
	//	m_ModelList.push_back(new RectangleModel(p_VulkanApp, p_Scene, p_Parent, p_Name, p_ShapeType)); // consider using shared ptr/smart pointers
	//	
	//	// Todo: Is it good to introduce another parameter as into RectangleModel
	//	RectangleModel* rectangleModel = static_cast<RectangleModel*>(m_ModelList.back());
	//	rectangleModel->m_AbstractFactory = this;

	//	return rectangleModel;
	//}

	virtual void Setup();
	void Update();
    void ResizeWindow(int width, int height);

	virtual void Prepare(Scene3D* p_Scene);

	// Parminder: Is it possible to remove the parameter from SingleTon, it looks wierd
    //static RectangleMultiDrawFactory* SingleTon(VulkanApp* p_VulkanApp = NULL) { return m_Singleton ? m_Singleton : (m_Singleton = new RectangleMultiDrawFactory(p_VulkanApp)); }
    //static RectangleMultiDrawFactory* m_Singleton;

	// Pipeline
	QMap<QString, QPair<VkPipeline, VkPipelineLayout> > m_GraphicsPipelineMap;

private:
	void CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap = false);
	void RecordCommandBuffer();
	void CreateVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride);
	void CreateCommandBuffers(); // Overide the default implementation as per application requirement
    virtual void UpdateModelList(Model3D* p_Item)
    {
        m_ModelList.push_back(p_Item);
        return;

        //RectangleModel* rectangle = dynamic_cast<RectangleModel*>(p_Item);
        //assert(rectangle);

        //// Note: Based on the draw type push the model in respective pipelines
        //// Keep the draw type loose couple with the pipeline type, 
        //// they may be in one-to-one correspondence but that is not necessary.
        //switch (rectangle->GetDrawType())
        //{
        //case RectangleModel::FILLED:
        //    //m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
        //    break;

        //case RectangleModel::OUTLINE:
        //    //m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
        //    break;

        //case RectangleModel::ROUNDED:
        //    // TODO
        //    break;

        //default:
        //    break;
        //}
    }

	VulkanBuffer m_VertexBuffer;

    enum RECTANGLE_GRAPHICS_PIPELINES
    {
        PIPELINE_FILLED = 0,
        PIPELINE_OUTLINE,
        PIPELINE_COUNT,
    };

	// Vertex buffer specific objects
	VkVertexInputBindingDescription		m_VertexInputBinding[1];   // 0 for (position and color)
	VkVertexInputAttributeDescription	m_VertexInputAttribute[2]; // Why 2 = 2(for position and color)

//    typedef std::vector<Model3D*> ModelVector;

    //VulkanBuffer m_VertexBuffer[PIPELINE_COUNT], m_InstanceBuffer[PIPELINE_COUNT];
    //ModelVector m_PipelineTypeModelVector[PIPELINE_COUNT];


    // Store app specific objects
    VulkanApp* m_VulkanApplication;
};

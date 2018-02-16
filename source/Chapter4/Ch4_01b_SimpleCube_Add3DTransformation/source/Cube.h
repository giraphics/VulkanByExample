#pragma once
#include "../../../common/VulkanApp.h"

struct Vertex
{
    glm::vec3 m_Position; // Vertex Position => x, y, z
    glm::vec3 m_Color;    // Color format => r, g, b
};

const float dimension = .5;
static const Vertex cubeVertices[] =
{
	{ glm::vec3(dimension, -dimension, -dimension),		glm::vec3(0.f, 0.f, 0.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(1.f, 0.f, 0.f) },
	{ glm::vec3(dimension,  dimension, -dimension),		glm::vec3(0.f, 1.f, 0.f) },
	{ glm::vec3(dimension,  dimension, -dimension),		glm::vec3(0.f, 1.f, 0.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(1.f, 0.f, 0.f) },
	{ glm::vec3(-dimension,  dimension, -dimension),	glm::vec3(1.f, 1.f, 0.f) },

	{ glm::vec3(dimension, -dimension, dimension),		glm::vec3(0.f, 0.f, 1.f) },
	{ glm::vec3(dimension,  dimension, dimension),		glm::vec3(0.f, 1.f, 1.f) },
	{ glm::vec3(-dimension, -dimension, dimension),		glm::vec3(1.f, 0.f, 1.f) },
	{ glm::vec3(-dimension, -dimension, dimension),		glm::vec3(1.f, 0.f, 1.f) },
	{ glm::vec3(dimension,  dimension, dimension),		glm::vec3(0.f, 1.f, 1.f) },
	{ glm::vec3(-dimension,  dimension, dimension),		glm::vec3(1.f, 1.f, 1.f) },

	{ glm::vec3(dimension, -dimension,  dimension),		glm::vec3(1.f, 1.f, 1.f) },
	{ glm::vec3(dimension, -dimension, -dimension),		glm::vec3(1.f, 1.f, 0.f) },
	{ glm::vec3(dimension,  dimension,  dimension),		glm::vec3(1.f, 0.f, 1.f) },
	{ glm::vec3(dimension,  dimension,  dimension),		glm::vec3(1.f, 0.f, 1.f) },
	{ glm::vec3(dimension, -dimension, -dimension),		glm::vec3(1.f, 1.f, 0.f) },
	{ glm::vec3(dimension,  dimension, -dimension),		glm::vec3(1.f, 0.f, 0.f) },

	{ glm::vec3(-dimension, -dimension,  dimension),	glm::vec3(0.f, 1.f, 1.f) },
	{ glm::vec3(-dimension, dimension, dimension),		glm::vec3(0.f, 0.f, 1.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(0.f, 1.f, 0.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(0.f, 1.f, 0.f) },
	{ glm::vec3(-dimension,  dimension,  dimension),	glm::vec3(0.f, 0.f, 1.f) },
	{ glm::vec3(-dimension,  dimension, -dimension),	glm::vec3(0.f, 0.f, 0.f) },

	{ glm::vec3(dimension, dimension, -dimension),		glm::vec3(1.f, 1.f, 1.f) },
	{ glm::vec3(-dimension, dimension, -dimension),		glm::vec3(0.f, 1.f, 1.f) },
	{ glm::vec3(dimension, dimension,  dimension),		glm::vec3(1.f, 1.f, 0.f) },
	{ glm::vec3(dimension, dimension,  dimension),		glm::vec3(1.f, 1.f, 0.f) },
	{ glm::vec3(-dimension, dimension, -dimension),		glm::vec3(0.f, 1.f, 1.f) },
	{ glm::vec3(-dimension, dimension,  dimension),		glm::vec3(0.f, 1.f, 0.f) },

	{ glm::vec3(dimension, -dimension, -dimension),		glm::vec3(1.f, 0.f, 1.f) },
	{ glm::vec3(dimension, -dimension,  dimension),		glm::vec3(1.f, 0.f, 0.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(0.f, 0.f, 1.f) },
	{ glm::vec3(-dimension, -dimension, -dimension),	glm::vec3(0.f, 0.f, 1.f) },
	{ glm::vec3(dimension, -dimension,  dimension),		glm::vec3(1.f, 0.f, 0.f) },
	{ glm::vec3(-dimension, -dimension,  dimension),	glm::vec3(0.f, 0.f, 0.f) },
};

struct Camera
{
	glm::mat4 m_Projetion;
	std::vector<glm::mat4> m_View;
};

class Cube : public DrawableInterface
{
public:
    Cube(VulkanApp * p_VulkanApp);
    virtual ~Cube();

	void Setup();
	void Update();

private:
	void CreateGraphicsPipeline();
	void RecordCommandBuffer();
	void CreateVertexBuffer(const void *vertexData, uint32_t dataSize, uint32_t dataStride);
	void CreateCommandBuffers(); // Overide the default implementation as per application requirement

	struct {
		VulkanBuffer m_BufObj;
	} VertexBuffer;

	struct {
		VulkanBuffer					m_BufObj;
		VkDescriptorBufferInfo			m_BufferInfo;		// Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
		std::vector<VkMappedMemoryRange>m_MappedRange;		// Metadata of memory mapped objects
		uint8_t*						m_Data;				// Host pointer containing the mapped device address which is used to write data into.
		size_t							m_DataSize;			// Data size.
	} UniformBuffer;

	void CreateUniformBuffer();
	void DestroyUniformBuffer();
	
	void CreateDescriptorSetLayout(bool useTexture);
	void DestroyDescriptorLayout();

	void CreateDescriptor(bool useTexture);

	// Creates the descriptor pool, this function depends on - 
	// createDescriptorSetLayout()
	void CreateDescriptorPool(bool useTexture);
	// Creates the descriptor sets using descriptor pool.
	// This function depend on the createDescriptorPool() and createUniformBuffer().
	void CreateDescriptorSet(bool useTexture);

	// List of all the VkDescriptorSetLayouts 
	std::vector<VkDescriptorSetLayout> descLayout;

	// Decriptor pool object that will be used for allocating VkDescriptorSet object
	VkDescriptorPool descriptorPool;

	// List of all created VkDescriptorSet
	std::vector<VkDescriptorSet> descriptorSet;
	///////////////////////////////////////////////////////////////////

	// Vertex buffer specific objects
	VkVertexInputBindingDescription		m_VertexInputBinding;
	VkVertexInputAttributeDescription	m_VertexInputAttribute[2]; // Why 2 - for position and color attribute

    // Store app specific objects
    VkPipelineLayout m_hPipelineLayout;
    VkPipeline       m_hGraphicsPipeline;
    VulkanApp*		 m_VulkanApplication;
};

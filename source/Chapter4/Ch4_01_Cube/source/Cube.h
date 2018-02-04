#pragma once
#include "../../../common/VulkanApp.h"

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
    glm::vec4 m_Position; // Vertex Position => x, y, z, w 
    glm::vec4 m_Color;    // Color format => r, g, b, a
};

static const Vertex cubeVertices[] =
{
	{ glm::vec4(1, -1, -1, 1.0f),		glm::vec4(0.f, 0.f, 0.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(1.f, 0.f, 0.f, 1.0f) },
	{ glm::vec4(1,  1, -1, 1.0f),		glm::vec4(0.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(1,  1, -1, 1.0f),		glm::vec4(0.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(1.f, 0.f, 0.f, 1.0f) },
	{ glm::vec4(-1,  1, -1, 1.0f),		glm::vec4(1.f, 1.f, 0.f, 1.0f) },

	{ glm::vec4(1, -1, 1, 1.0f),		glm::vec4(0.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1,  1, 1, 1.0f),		glm::vec4(0.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(-1, -1, 1, 1.0f),		glm::vec4(1.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(-1, -1, 1, 1.0f),		glm::vec4(1.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1,  1, 1, 1.0f),		glm::vec4(0.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(-1,  1, 1, 1.0f),		glm::vec4(1.f, 1.f, 1.f, 1.0f) },

	{ glm::vec4(1, -1,  1, 1.0f),		glm::vec4(1.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(1, -1, -1, 1.0f),		glm::vec4(1.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(1,  1,  1, 1.0f),		glm::vec4(1.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1,  1,  1, 1.0f),		glm::vec4(1.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1, -1, -1, 1.0f),		glm::vec4(1.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(1,  1, -1, 1.0f),		glm::vec4(1.f, 0.f, 0.f, 1.0f) },

	{ glm::vec4(-1, -1,  1, 1.0f),		glm::vec4(0.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(-1,  1,  1, 1.0f),		glm::vec4(0.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(0.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(0.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(-1,  1,  1, 1.0f),		glm::vec4(0.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(-1,  1, -1, 1.0f),		glm::vec4(0.f, 0.f, 0.f, 1.0f) },

	{ glm::vec4(1, 1, -1, 1.0f),		glm::vec4(1.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(-1, 1, -1, 1.0f),		glm::vec4(0.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(1, 1,  1, 1.0f),		glm::vec4(1.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(1, 1,  1, 1.0f),		glm::vec4(1.f, 1.f, 0.f, 1.0f) },
	{ glm::vec4(-1, 1, -1, 1.0f),		glm::vec4(0.f, 1.f, 1.f, 1.0f) },
	{ glm::vec4(-1, 1,  1, 1.0f),		glm::vec4(0.f, 1.f, 0.f, 1.0f) },

	{ glm::vec4(1, -1, -1, 1.0f),		glm::vec4(1.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1, -1,  1, 1.0f),		glm::vec4(1.f, 0.f, 0.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(0.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(-1, -1, -1, 1.0f),		glm::vec4(0.f, 0.f, 1.f, 1.0f) },
	{ glm::vec4(1, -1,  1, 1.0f),		glm::vec4(1.f, 0.f, 0.f, 1.0f) },
	{ glm::vec4(-1, -1,  1, 1.0f),		glm::vec4(0.f, 0.f, 0.f, 1.0f) },
};

class Cube
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
	/************************************************************************/
	void CreateCommandBuffers(); // Overide the default implementation as per application requirement
	/************************************************************************/

	///////////////////////////////////////////////////////////////////
	struct {
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo m_BufferInfo;
	} VertexBuffer;

	struct {
		VkBuffer						m_Buffer;			// Buffer resource object
		VkDeviceMemory					m_Memory;			// Buffer resourece object's allocated device memory
		VkDescriptorBufferInfo			m_BufferInfo;		// Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
		VkMemoryRequirements			m_MemmoryRequirement;		// Store the queried memory requirement of the uniform buffer
		std::vector<VkMappedMemoryRange>m_MappedRange;	// Metadata of memory mapped objects
		uint8_t*						m_Data;			// Host pointer containing the mapped device address which is used to write data into.
	} Uniform; // ########## rename this to uniform

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

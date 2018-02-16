#pragma once
#include "../../../common/VulkanApp.h"

/*********** ASSIMP HEADER FILES ***********/
#include <assimp/scene.h>     
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class SimpleMesh
{
	//struct VertexUI {
	//	glm::vec3 pos;
	//};

	struct {
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		VkDescriptorBufferInfo m_BufferInfo;
	} VertexBuffer;

	struct {
		VkBuffer						buffer;			// Buffer resource object
		VkDeviceMemory					memory;			// Buffer resourece object's allocated device memory
		VkDescriptorBufferInfo			bufferInfo;		// Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
		VkMemoryRequirements			memRqrmnt;		// Store the queried memory requirement of the uniform buffer
		std::vector<VkMappedMemoryRange>mappedRange;	// Metadata of memory mapped objects
		uint8_t*						pData;			// Host pointer containing the mapped device address which is used to write data into.
	} Uniform;

public:
	SimpleMesh(VulkanApp * p_VulkanApp);
	virtual ~SimpleMesh();

	void Setup();
	void Update();

private:
	void CreateGraphicsPipeline();
	void RecordCommandBuffer();
	void CreateCommandBuffers();
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
	VkVertexInputAttributeDescription	m_VertexInputAttribute[1]; // Why 1 - for position

    // Store app specific objects
    VkPipelineLayout m_hPipelineLayout;
    VkPipeline       m_hGraphicsPipeline;
	VulkanApp*		 m_VulkanApplication;
	////////////////////////
	// GPU Data structure for Meshes contain device buffer and memory
	struct MeshVertices{ 
		VkBuffer m_Buffer; 
		VkDeviceMemory m_Memory; 
	};
	struct MeshIndices {
		int m_IndexCount;
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
	};
	struct Mesh {
		struct MeshVertices m_Vertices;
		struct MeshIndices m_Indices;
	} mesh;

	// CPU Data structure for Meshes
	struct Vertex
	{
		Vertex(const glm::vec3& pos) { m_pos = pos; }

		glm::vec3 m_pos;
	};

	struct MeshEntry {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	void LoadMesh();
	void LoadMeshNew();
	bool Load(const char* filename);
	void MeshInit(MeshEntry *meshEntry, const aiMesh* paiMesh, const aiScene* pScene);
	uint32_t numVertices = 0;
	Assimp::Importer Importer;
	const aiScene* pScene;


	std::vector<MeshEntry> m_Entries;
	///////////////////////////////////////////////////////////////////
};
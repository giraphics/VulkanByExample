#pragma once
#include "../../../common/VulkanApp.h"

/*********** ASSIMP HEADER FILES ***********/
#include <assimp/scene.h>     
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

class Cube : public DrawableInterface
{
//	struct {
//		VulkanBuffer m_BufObj;
//    } VertexBuffer;

	struct {
		VulkanBuffer					m_BufObj;
		VkDescriptorBufferInfo			m_BufferInfo;		// Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
        std::vector<VkMappedMemoryRange>m_MappedRange;	// Metadata of memory mapped objects
        uint8_t*						m_Data;			// Host pointer containing the mapped device address which is used to write data into.
    } UniformBuffer;

public:
    Cube(VulkanApp * p_VulkanApp);
    virtual ~Cube();

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
        VulkanBuffer bufObj;
	};
	struct MeshIndices {
		int m_IndexCount;
        VulkanBuffer bufObj;
	};
	struct Mesh {
        struct MeshVertices vertices;
        struct MeshIndices indices;
    } m_Mesh;

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
    bool Load(const char* p_Filename);
    void MeshInit(MeshEntry* p_MeshEntry, const aiMesh* p_pAiMesh);
	uint32_t numVertices = 0;
    Assimp::Importer m_Importer;
    const aiScene* m_pScene;


	std::vector<MeshEntry> m_Entries;
	///////////////////////////////////////////////////////////////////
};

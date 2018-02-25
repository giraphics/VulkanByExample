#pragma once
#include "../../../common/VulkanApp.h"

/*********** ASSIMP HEADER FILES ***********/
#include <assimp/scene.h>     
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

class Cube : public DrawableInterface
{
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
    // GPU Data structure for Meshes contain device buffer and memory
    struct Mesh {
        VulkanBuffer vertexBuffer;
        VulkanBuffer indexBuffer;
        uint32_t indexCount;
    } m_Mesh;

    // CPU Data structure for Meshes
    struct Vertex {
        Vertex(const glm::vec3& pos) { m_pos = pos; }
        glm::vec3 m_pos;
    };

    struct MeshNode {
        std::vector<Vertex> Vertices;
        std::vector<unsigned int> Indices;
    };
    std::vector<MeshNode> m_Nodes;

    void LoadMesh(bool p_UseStaging = true);
    bool Load(const char* p_Filename);
    void MeshInit(MeshNode* p_MeshNode, const aiMesh* p_pAiMesh);
    Assimp::Importer m_Importer;
    const aiScene* m_pScene;

    ///////////////////////////////////////////////////////////////////

	// Vertex buffer specific objects
	VkVertexInputBindingDescription		m_VertexInputBinding;
	VkVertexInputAttributeDescription	m_VertexInputAttribute[1]; // Why 1 - for position

    // Store app specific objects
    VkPipelineLayout m_hPipelineLayout;
    VkPipeline       m_hGraphicsPipeline;
	VulkanApp*		 m_VulkanApplication;
};

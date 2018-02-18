#pragma once
#include "../../../common/VulkanApp.h"
//#include "Quad.h"

struct UniformBufferObject
{
    float xOffset, yOffset;         // Scrolling offset
};

struct Position { float x, y, z, w; };  // Vertex Position
struct Color { float r, g, b, a; };     // Color format Red, Green, Blue, Alpha
struct TexCoord { float u, v; };        // Texture coordinate (u,v)

struct Vertex
{
    Position    m_Position;
    Color       m_Color;
    TexCoord    m_TexCoord;
};

struct Quad
{
    Quad()
    {
        m_VertexBuffer = nullptr;
        m_VertexBufferMemory = nullptr;
		m_TextureImage = {};
		m_textureImageView = {};
        m_descriptorSet = nullptr;
    }

	// Todo: For Selva: Do you like to use Vulkan Buffer instead?
    VkBuffer            m_VertexBuffer;
    VkDeviceMemory      m_VertexBufferMemory;
    
	VulkanImage			m_TextureImage;
	VulkanImageView     m_textureImageView;
    VkDescriptorSet     m_descriptorSet;
};

class Canvas2DApp :public VulkanApp
{
public:
    Canvas2DApp();
    ~Canvas2DApp();

    void Configure();
    void Setup();
    void Update();
    bool Render();

private:
    // Store app specific objects
    vector<string>              m_imageFiles;
    uint32_t                    m_numImageFiles;

    // A Quad object for each image file
    vector<Quad>                m_quad;

    // Total number of columns needed for the canvas
    float                       m_numTotalCols;
    // Number of visible columns to show in the window
    float                       m_numVisibleCols;
    // Number of visible rows to show in the window
    float                       m_numVisibleRows;

    float                       m_scrollDelta;
    float                       m_tmpScrollDelta;

    VkPipelineLayout            m_canvas2DPipelineLayout;
    VkPipeline                  m_canvas2DGraphicsPipeline;

    VkDescriptorSetLayout       m_descSetLayout;
    VkDescriptorPool            m_descriptorPool;

    VkSampler                   m_textureSampler;

    VkBuffer                    m_uniformBuffer;
    VkDeviceMemory              m_uniformBufferMemory;

    VkVertexInputBindingDescription     m_VertexInputBinding;
    VkVertexInputAttributeDescription   m_VertexInputAttribute[3];

    void CreateDescriptorLayout();
    bool CreateGraphicsPipeline();
    void CreateDescriptorPool();
    void CreateUniformBuffer();
    void CreateImageTiles();

    int GetImageFilenames();

    void CreateTextureSampler();
    void BuildCommandBuffers();

    void UpdateUniformBuffer();

    void TransitionImageLayout(VkImage textureImage, VkImageLayout currentLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateTexture(string textureFilename, VulkanImage& textureImage, VulkanImageView& textureView);
    
    VkDescriptorSet CreateDescriptorSet(VkImageView imageView);
};

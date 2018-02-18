#include "Canvas2DApp.h"
#include <QDirIterator>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Canvas2DApp::Canvas2DApp():
    m_scrollDelta(0), m_tmpScrollDelta(1), m_numTotalCols(0),m_numVisibleCols(0),
    m_numVisibleRows(0), m_canvas2DPipelineLayout(VK_NULL_HANDLE), m_canvas2DGraphicsPipeline(VK_NULL_HANDLE), 
    m_textureSampler(VK_NULL_HANDLE), m_uniformBuffer(VK_NULL_HANDLE), m_uniformBufferMemory(VK_NULL_HANDLE)
{
}

Canvas2DApp::~Canvas2DApp()
{
    for (uint32_t i = 0; i < m_numImageFiles; i++)
    {
        if (m_quad[i].m_VertexBuffer != nullptr)
        {
            vkDestroyBuffer(m_hDevice, m_quad[i].m_VertexBuffer, nullptr);
            m_quad[i].m_VertexBuffer = nullptr;
        }

        if (m_quad[i].m_VertexBufferMemory != nullptr)
        {
            vkFreeMemory(m_hDevice, m_quad[i].m_VertexBufferMemory, nullptr);
            m_quad[i].m_VertexBufferMemory = nullptr;
        }

        if (m_quad[i].m_textureImageView != nullptr)
        {
            vkDestroyImageView(m_hDevice, m_quad[i].m_textureImageView, nullptr);
            m_quad[i].m_textureImageView = nullptr;
        }

        if (m_quad[i].m_TextureImage.image != nullptr)
        {
            vkDestroyImage(m_hDevice, m_quad[i].m_TextureImage.image, nullptr);
            m_quad[i].m_TextureImage.image = nullptr;
        }

        if (m_quad[i].m_TextureImage.deviceMemory)
        {
            vkFreeMemory(m_hDevice, m_quad[i].m_TextureImage.deviceMemory, nullptr);
            m_quad[i].m_TextureImage.deviceMemory = nullptr;
        }
    }

    vkDestroySampler(m_hDevice, m_textureSampler, nullptr);

    vkDestroyDescriptorSetLayout(m_hDevice, m_descSetLayout, nullptr);

    vkDestroyDescriptorPool(m_hDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_hDevice, m_canvas2DGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_hDevice, m_canvas2DPipelineLayout, nullptr);

    vkDestroyBuffer(m_hDevice, m_uniformBuffer, nullptr);
    vkFreeMemory(m_hDevice, m_uniformBufferMemory, nullptr);
}

// Get the list of jpg image files in the current folder and 
// store them in m_imageFiles.
// Returns the count of jpg files
int Canvas2DApp::GetImageFilenames()
{
    QDirIterator qDirIt("../source/resource", QStringList() << "*.jpg", QDir::Files, QDirIterator::Subdirectories);

    while (qDirIt.hasNext())
    {
        QString qText = qDirIt.next();
        std::string filename = qText.toUtf8().constData();
        m_imageFiles.push_back(filename);
    }

    return ((int)m_imageFiles.size());
}

void Canvas2DApp::Configure()
{
    SetApplicationName("Canvas2D");
    SetWindowDimension(800, 600);

    m_numImageFiles = GetImageFilenames();

    VulkanHelper::GetInstanceLayerExtensionProperties();

    // Add Validation Layers
    AddValidationLayer("VK_LAYER_LUNARG_standard_validation");

    // Allow the user to specify the Vulkan instance extensions
    AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    assert(m_numImageFiles); // No image files found
}

void Canvas2DApp::Update()
{
    // Update the scrolling offset
    UpdateUniformBuffer();
}

void Canvas2DApp::Setup()
{
    CreateCommandBuffers();
    CreateUniformBuffer();
    CreateTextureSampler();
    CreateDescriptorLayout();
    CreateDescriptorPool();
    CreateImageTiles();
    CreateGraphicsPipeline();
    BuildCommandBuffers();
}

// Create an instance of Quad object for each image
// file found in the executable folder
void Canvas2DApp::CreateImageTiles()
{
    // Num of visible tiles per column
    m_numVisibleCols = 4;
    
    // Num of visible tiles per row
    m_numVisibleRows = 4;

    // Total colums to create
    m_numTotalCols = m_numImageFiles / m_numVisibleRows;

    m_quad.resize(m_numImageFiles);

    float w = 2.0f / m_numVisibleCols;
    float h = 2.0f / m_numVisibleRows;
    uint32_t idx = 0;

    for (float i = 0; i < m_numTotalCols; i++)
    {
        for (float j = 0; j < m_numVisibleRows; j++)
        {
            if (idx < m_numImageFiles)
            {
                // Create vertex buffer for QUAD
				VulkanBuffer buffObj;

                // Calculate vertex positions in normalized device coordinates
                float xf = (float)i*w;
                float yf = (float)j*h;

                float x1 = -1.0f + xf;
                float y1 = -1.0f + yf;
                float x2 = x1 + w;
                float y2 = y1 + h;

                // Initialize vertex positions for the quad in normalized device coordinates
                const Vertex vertices[] =
                {
                    { { x1, y1 },{ 1.0f, 1.0f, 1.0f },{ 0.0f,0.0f } }, // Top left
                    { { x2, y1 },{ 0.0f, 0.0f, 1.0f },{ 1.0f,0.0f } }, // Top right
                    { { x1, y2 },{ 0.0f, 1.0f, 0.0f },{ 0.0f,1.0f } }, // Bottom left
                    { { x2, y2 },{ 1.0f, 0.0f, 0.0f },{ 1.0f,1.0f } }  // Bottom right
                };

				buffObj.m_DataSize = sizeof(vertices);
				buffObj.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				VulkanHelper::CreateBuffer(m_hDevice, m_physicalDeviceInfo.memProp, buffObj);
				VulkanHelper::WriteBuffer(m_hDevice, vertices, buffObj);

                m_quad[idx].m_VertexBuffer = buffObj.m_Buffer;
                m_quad[idx].m_VertexBufferMemory = buffObj.m_Memory;
                
                // Create texture for QUAD
                CreateTexture(m_imageFiles[idx], m_quad[idx].m_TextureImage, m_quad[idx].m_textureImageView);

                // Create descriptor for QUAD
                VkDescriptorSet descSet = CreateDescriptorSet(m_quad[idx].m_textureImageView);
                m_quad[idx].m_descriptorSet = descSet;

                idx++;
            }
        }
    }

    // Indicates the rate at which the information will be
    // injected for vertex input.
    m_VertexInputBinding.binding = 0;
    m_VertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_VertexInputBinding.stride = sizeof(Vertex);

    // The VkVertexInputAttribute interpreting the data.
    m_VertexInputAttribute[0].binding = 0;
    m_VertexInputAttribute[0].location = 0;
    m_VertexInputAttribute[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_VertexInputAttribute[0].offset = offsetof(struct Vertex, m_Position);

    m_VertexInputAttribute[1].binding = 0;
    m_VertexInputAttribute[1].location = 1;
    m_VertexInputAttribute[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_VertexInputAttribute[1].offset = offsetof(struct Vertex, m_Color);

    // Texture coordinate
    m_VertexInputAttribute[2].binding = 0;
    m_VertexInputAttribute[2].location = 2;
    m_VertexInputAttribute[2].format = VK_FORMAT_R32G32_SFLOAT;
    m_VertexInputAttribute[2].offset = offsetof(Vertex, m_TexCoord);
}

void Canvas2DApp::CreateTexture(string textureFilename, VulkanImage& textureImage, VkImageView& textureImageView)
{
    // 1: Load the jpg file and retrieve the pixel content.
    int texWidth, texHeight, texChannels;
    stbi_uc* pPixels = stbi_load(textureFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    int imageSize = texWidth * texHeight * 4;

	textureImage.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };

    if (pPixels)
    {
#if 0
        // Linear copy
        // @todo: Vulkan expects the buffer stide to be 32byte aligned
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = textureImage.extent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;  // Linear tiling
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		textureImage.memoryFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VulkanHelper::CreateImage(m_hDevice, m_physicalDeviceInfo.memProp, textureImage, &imageInfo);

        VulkanHelper::UpdateMemory(m_hDevice, textureImage.deviceMemory, 0, imageSize, 0, pPixels);

        //@todo Revisit transition for linear image once transition image layout is generalized
        TransitionImageLayout(textureImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        textureImageView = VulkanHelper::CreateImageView(m_hDevice, textureImage.image);
#else
        // Copy using staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        // Create staging buffer
        VulkanHelper::createBuffer(m_hDevice,usageFlags, propertyFlags, m_physicalDeviceInfo.memProp, imageSize,
            &stagingBuffer,
            &stagingBufferMemory);

        VulkanHelper::UpdateMemory(m_hDevice, stagingBufferMemory, 0, imageSize, 0, pPixels);

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = textureImage.extent;
		imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Optimal tiling
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        //VulkanHelper::CreateImage(m_hDevice,
        //                          m_physicalDeviceInfo.memProp,
        //                          (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
        //                          &imageInfo,
        //                          &textureImage.image,
        //                          &textureImage.deviceMemory);
    	textureImage.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VulkanHelper::CreateImage(m_hDevice, m_physicalDeviceInfo.memProp, textureImage, &imageInfo);
        TransitionImageLayout(textureImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, textureImage.image, texWidth, texHeight);
        TransitionImageLayout(textureImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        textureImageView = VulkanHelper::CreateImageView(m_hDevice, textureImage.image);

        vkDestroyBuffer(m_hDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_hDevice, stagingBufferMemory, nullptr);
#endif

        stbi_image_free(pPixels);
    }
}

void Canvas2DApp::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    if (!m_hCommandPool) { VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo); }

    VkCommandBuffer cmdBuffer;
    // Use command buffer to create the depth image. This includes -
    // Command buffer allocation, recording with begin/end scope and submission.
    VulkanHelper::AllocateCommandBuffer(m_hDevice, m_hCommandPool, &cmdBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VulkanHelper::BeginCommandBuffer(cmdBuffer, &beginInfo);

    // Copy buffer to image
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    vkCmdCopyBufferToImage(cmdBuffer, buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VulkanHelper::EndCommandBuffer(cmdBuffer);
    VulkanHelper::SubmitCommandBuffer(m_hGraphicsQueue, cmdBuffer);

    // Free the allocated one time commmand buffer
    vkFreeCommandBuffers(m_hDevice, m_hCommandPool, 1, &cmdBuffer);
}

void Canvas2DApp::TransitionImageLayout(VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    if (!m_hCommandPool) { VulkanHelper::CreateCommandPool(m_hDevice, m_hCommandPool, m_physicalDeviceInfo); }

    VkCommandBuffer cmdBuffer;
    // Use command buffer to create the depth image. This includes -
    // Command buffer allocation, recording with begin/end scope and submission.
    VulkanHelper::AllocateCommandBuffer(m_hDevice, m_hCommandPool, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VulkanHelper::BeginCommandBuffer(cmdBuffer, &beginInfo);

    // Use command buffer to create the depth image. This includes -
    // Command buffer allocation, recording with begin/end scope and submission.
    {
        // Set the image layout 
        //@todo: Revisit this method to handle all transition types
        VulkanHelper::SetImageLayout(image,
            VK_IMAGE_ASPECT_COLOR_BIT, // DEBUG
            currentLayout,
            newLayout, (VkAccessFlagBits)0, cmdBuffer);
         
        //@todo: Remove this method once SetImageLayout handles all transition
        //VulkanHelper::SetImageLayoutEx(image, currentLayout, newLayout, cmdBuffer);
    }
    VulkanHelper::EndCommandBuffer(cmdBuffer);
    VulkanHelper::SubmitCommandBuffer(m_hGraphicsQueue, cmdBuffer);

    // Free the allocated one time commmand buffer
    vkFreeCommandBuffers(m_hDevice, m_hCommandPool, 1, &cmdBuffer);
}

VkDescriptorSet Canvas2DApp::CreateDescriptorSet(VkImageView imageView)
{
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descSetLayout;

    VkDescriptorSet descSet = 0;

    VkResult result = vkAllocateDescriptorSets(m_hDevice, &allocInfo, &descSet);
    assert(result == VK_SUCCESS);
    
    // Setup buffer info for uniform buffer object
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    // Setup image info for the texture
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = m_textureSampler;

    VkWriteDescriptorSet descriptorWrites[2] = {};

    // Setup descriptor write for buffer info
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    // Setup descriptor write for image info
    descriptorWrites[1].sType =  VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType =  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    // Update the descriptor set
    vkUpdateDescriptorSets(m_hDevice, 2, descriptorWrites, 0, nullptr);

    // Return the newly created descriptor set
    return (descSet);
}

void Canvas2DApp::CreateUniformBuffer()
{
    // memory size to allocate
    VkDeviceSize size = sizeof(UniformBufferObject);

    UniformBufferObject ubo = {};

    // Usage flag set to Uniform buffer
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Set the property flag to indicate the memory is visible on host side
    // and the content is coherent between CPU & GPU
    VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    //@todo Revisit this once CreateBuffer is generalized
    VulkanHelper::createBuffer(m_hDevice, usageFlags, propertyFlags, m_physicalDeviceInfo.memProp, size, &m_uniformBuffer, &m_uniformBufferMemory);
}


void Canvas2DApp::UpdateUniformBuffer()
{
    UniformBufferObject ubo = {};

    ubo.xOffset = (float)(m_scrollDelta);
    ubo.yOffset = 0;

    m_scrollDelta -= m_tmpScrollDelta / 5000.0f;

    float windowWidth = (float)m_windowDim.width;
    float col = m_numTotalCols*(windowWidth / m_numVisibleCols);
    col = (col - windowWidth) / windowWidth * 2.0f;

    if ((m_scrollDelta <= -col) || (m_scrollDelta > 0))
    {
        m_tmpScrollDelta = -m_tmpScrollDelta;
    }

    // Update memory
    VulkanHelper::UpdateMemory(m_hDevice, m_uniformBufferMemory, 0, sizeof(ubo), 0, &ubo);
}

void Canvas2DApp::BuildCommandBuffers()
{
    // The following code records the commands to draw
    // a quad in a blue background

    // The color values are defined in this order (Red,Green,Blue,Alpha)
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    // Offset to render in the frame buffer
    VkOffset2D   renderOffset = { 0, 0 };
    // Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_swapChainExtent;

    // For each command buffers in the command buffer list
    for (size_t i = 0; i < m_hCommandBufferList.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Indicate that the command buffer can be resubmitted to the queue
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Step 1: Begin command buffer
        vkBeginCommandBuffer(m_hCommandBufferList[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_hRenderPass;
        renderPassInfo.framebuffer = m_hFramebuffers[i];
        renderPassInfo.renderArea.offset = renderOffset;
        renderPassInfo.renderArea.extent = renderExtent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // Step 2: Begin render pass
        vkCmdBeginRenderPass(m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Step 3: Bind graphics pipeline
        vkCmdBindPipeline(m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_canvas2DGraphicsPipeline);

        uint32_t c = 0;
        for (int x = 0; x < m_numTotalCols; x++)
        {
            for (int y = 0; y < m_numVisibleRows; y++)
            {
                if (c < m_numImageFiles)
                {
                    vkCmdBindDescriptorSets(m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_canvas2DPipelineLayout, 0, 1, &(m_quad[c].m_descriptorSet), 0, nullptr);

                    VkBuffer vertexBuffers[] = { m_quad[c].m_VertexBuffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(m_hCommandBufferList[i], 0, 1, vertexBuffers, offsets);

                    // Step 4: Draw a quad using 4 vertices 
                    vkCmdDraw(m_hCommandBufferList[i], 4, 1, 0, 0);
                    c++;
                }
            }
        }

        // Step 5: End the Render pass
        vkCmdEndRenderPass(m_hCommandBufferList[i]);

        // Step 6: End the Command buffer
        VkResult result = vkEndCommandBuffer(m_hCommandBufferList[i]);
        assert(result == VK_SUCCESS);
    }
}

void Canvas2DApp::CreateDescriptorLayout()
{
    // Setup binding for uniform buffer for Vertex shader
    // and sampler for the fragment shader
    VkDescriptorSetLayoutBinding bindings[2] = {};

    // Initialize the layout binding for Uniform buffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Initialize the layout binding for the sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    VkResult result = vkCreateDescriptorSetLayout(m_hDevice,
                                        &layoutInfo,
                                        nullptr,
                                        &m_descSetLayout);
    assert(result == VK_SUCCESS);
}

bool Canvas2DApp::Render()
{ 
    return VulkanApp::Render(); 
}

bool Canvas2DApp::CreateGraphicsPipeline()
{
    bool result = true;

    // Compile the vertex shader
    VkShaderModule vertShader = VulkanHelper::CreateShader(m_hDevice,"../source/shaders/QuadVert.spv");
    // Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
    VkShaderModule fragShader = VulkanHelper::CreateShader(m_hDevice,"../source/shaders/QuadFrag.spv");
    // Setup the fragment shader stage create info structures
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Setup the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = (sizeof(m_VertexInputAttribute) / sizeof(VkVertexInputAttributeDescription));
    vertexInputInfo.pVertexBindingDescriptions =  &m_VertexInputBinding;
    vertexInputInfo.pVertexAttributeDescriptions = &m_VertexInputAttribute[0];

    // Setup input assembly
    // We will be rendering 2 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport to the maximum widht and height of the window
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChainExtent.width;
    viewport.height = (float)m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Setup scissor rect
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;

    // Setup view port state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Setup the rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Setup multi sampling. In our first example we will be using single sampling mode
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Setup color output masks.
    // Set to write out RGBA components
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    // Setup color blending
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout;

    VkResult vkResult = vkCreatePipelineLayout(m_hDevice, &pipelineLayoutInfo, nullptr, &m_canvas2DPipelineLayout);
    assert(vkResult == VK_SUCCESS);

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_canvas2DPipelineLayout;
    pipelineInfo.renderPass = m_hRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vkResult = vkCreateGraphicsPipelines(m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_canvas2DGraphicsPipeline);
    assert(vkResult == VK_SUCCESS);

    // Cleanup
    vkDestroyShaderModule(m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_hDevice, vertShader, nullptr);

    return (result);
}

void Canvas2DApp::CreateDescriptorPool() 
{
    VkDescriptorPoolSize poolSizes[2] = {};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = m_numImageFiles;
    
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = m_numImageFiles;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = m_numImageFiles;

    VkResult result = vkCreateDescriptorPool(m_hDevice, &poolInfo, nullptr, &m_descriptorPool);
    assert(result == VK_SUCCESS);
}

void Canvas2DApp::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult result = vkCreateSampler(m_hDevice, &samplerInfo, nullptr, &m_textureSampler);

    assert (result == VK_SUCCESS);
}

int main(int argc, char **argv)
{
    QApplication qtApp(argc, argv);

    Canvas2DApp* pCanvas2DApp = new Canvas2DApp(); // Create Vulkan app instance
    pCanvas2DApp->EnableDepthBuffer(false);
    pCanvas2DApp->Initialize();
    qtApp.exec();

    delete pCanvas2DApp;
    return 0;
}

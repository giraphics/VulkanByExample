#include "Cube.h"

#include "../../../common/VulkanHelper.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

using namespace std;

Cube::Cube(VulkanApp* p_VulkanApp)
{
	m_hPipelineLayout = VK_NULL_HANDLE;
    m_hGraphicsPipeline = VK_NULL_HANDLE;

	memset(&UniformBuffer, 0, sizeof(UniformBuffer));
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));

    m_VulkanApplication = p_VulkanApp;
}

Cube::~Cube()
{
    vkDestroyPipeline(m_VulkanApplication->m_hDevice, m_hGraphicsPipeline, nullptr);

	// Destroy Vertex Buffer
    vkDestroyBuffer(m_VulkanApplication->m_hDevice, VertexBuffer.m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanApplication->m_hDevice, VertexBuffer.m_BufObj.m_Memory, NULL);

	// Destroy descriptors
	for (int i = 0; i < descLayout.size(); i++) {
        vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
	}
	descLayout.clear();

    vkDestroyPipelineLayout(m_VulkanApplication->m_hDevice, m_hPipelineLayout, nullptr);

    vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
    vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);

    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Memory, NULL);
}

void Cube::Setup()
{
    uint32_t dataSize = sizeof(cubeVertices);
    uint32_t dataStride = sizeof(cubeVertices[0]);
    CreateVertexBuffer(cubeVertices, dataSize, dataStride);
	
	CreateDescriptor(false);

	CreateGraphicsPipeline();

	CreateCommandBuffers(); // Create command buffers //XXXXXXXXXXXXXXXX

	RecordCommandBuffer();
}

void Cube::Update()
{
	glm::mat4 MVP = (*m_Projection) * (*m_View) * m_Model;

    VkResult res = vkInvalidateMappedMemoryRanges(m_VulkanApplication->m_hDevice, 1, &UniformBuffer.m_MappedRange[0]);
	assert(res == VK_SUCCESS);

	// Copy updated data into the mapped memory
    memcpy(UniformBuffer.m_Data, &MVP, sizeof(MVP));

    res = vkFlushMappedMemoryRanges(m_VulkanApplication->m_hDevice, 1, &UniformBuffer.m_MappedRange[0]);
	assert(res == VK_SUCCESS);
}

void Cube::CreateGraphicsPipeline()
{
    // Compile the vertex shader
    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/TriangleVert.spv"); // Relative path to binary output dir

	// Setup the vertex shader stage create info structures
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.pName = "main";

    // Compile the fragment shader
    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/TriangleFrag.spv"); // Relative path to binary output dir

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
	vertexInputInfo.vertexBindingDescriptionCount = sizeof(m_VertexInputBinding) / sizeof(VkVertexInputBindingDescription);
	vertexInputInfo.pVertexBindingDescriptions = &m_VertexInputBinding;
	vertexInputInfo.vertexAttributeDescriptionCount = sizeof(m_VertexInputAttribute) / sizeof(VkVertexInputAttributeDescription);
	vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute;


    // Setup input assembly
    // We will be rendering 1 triangle using triangle strip topology
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Setup viewport to the maximum widht and height of the window
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Setup scissor rect
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
    scissor.extent = m_VulkanApplication->m_swapChainExtent;

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
	rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthClampEnable = true;

	// Setup multi sampling. In our first example we will be using single sampling mode
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = NUM_SAMPLES;

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
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.setLayoutCount = (uint32_t)descLayout.size();
	pipelineLayoutInfo.pSetLayouts = descLayout.data();

    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &m_hPipelineLayout);

	if (vkResult != VK_SUCCESS)
	{
		VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
		assert(false);
	}

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
	pipelineInfo.layout = m_hPipelineLayout;
    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_hGraphicsPipeline);
	if (vkResult != VK_SUCCESS)
	{
		VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
		assert(false);
	}

	// Cleanup
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
}

void Cube::RecordCommandBuffer()
{
	// Specify the clear color value
	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Offset to render in the frame buffer
	VkOffset2D   renderOffset = { 0, 0 };
	// Width & Height to render in the frame buffer
    VkExtent2D   renderExtent = m_VulkanApplication->m_swapChainExtent;

	// For each command buffers in the command buffer list
    for (size_t i = 0; i < m_VulkanApplication->m_hCommandBufferList.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// Indicate that the command buffer can be resubmitted to the queue
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		// Step 1: Begin command buffer
        vkBeginCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_VulkanApplication->m_hRenderPass;
        renderPassInfo.framebuffer = m_VulkanApplication->m_hFramebuffers[i];
		renderPassInfo.renderArea.offset = renderOffset;
		renderPassInfo.renderArea.extent = renderExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		// Begin render pass
        vkCmdBeginRenderPass(m_VulkanApplication->m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind graphics pipeline
        vkCmdBindPipeline(m_VulkanApplication->m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_hGraphicsPipeline);

        vkCmdBindDescriptorSets(m_VulkanApplication->m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_hPipelineLayout,
			0, 1, descriptorSet.data(), 0, NULL);

		// Specify vertex buffer information
		const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(m_VulkanApplication->m_hCommandBufferList[i], 0, 1, &VertexBuffer.m_BufObj.m_Buffer, offsets);
		
		// Draw a quad using 3 vertices 
        vkCmdDraw(m_VulkanApplication->m_hCommandBufferList[i], 3 * 2 * 6, 1, 0, 0);

		// End the Render pass
        vkCmdEndRenderPass(m_VulkanApplication->m_hCommandBufferList[i]);

		// End the Command buffer
        VkResult vkResult = vkEndCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i]);
		if (vkResult != VK_SUCCESS)
		{
			VulkanHelper::LogError("vkEndCommandBuffer() failed!");
			assert(false);
		}
	}
}

void Cube::CreateVertexBuffer(const void * vertexData, uint32_t dataSize, uint32_t dataStride)
{
	VertexBuffer.m_BufObj.m_DataSize = dataSize;
	VertexBuffer.m_BufObj.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VulkanHelper::CreateBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_physicalDeviceInfo.memProp,
							   vertexData, dataSize, VertexBuffer.m_BufObj);

	// Indicates the rate at which the information will be
	// injected for vertex input.
	m_VertexInputBinding.binding = 0;
	m_VertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	m_VertexInputBinding.stride = dataStride;

	// The VkVertexInputAttribute interpreting the data.
	m_VertexInputAttribute[0].binding = 0;
	m_VertexInputAttribute[0].location = 0;
	m_VertexInputAttribute[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_VertexInputAttribute[0].offset = offsetof(struct Vertex, m_Position);

	m_VertexInputAttribute[1].binding = 0;
	m_VertexInputAttribute[1].location = 1;
	m_VertexInputAttribute[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_VertexInputAttribute[1].offset = offsetof(struct Vertex, m_Color);
}

void Cube::CreateCommandBuffers()
{
    m_VulkanApplication->CreateCommandBuffers();
}

void Cube::CreateUniformBuffer()
{
	UniformBuffer.m_BufObj.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	UniformBuffer.m_BufObj.m_DataSize = sizeof(glm::mat4);

	VkResult  result;
	// Create buffer resource states using VkBufferCreateInfo
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufInfo.size = UniformBuffer.m_BufObj.m_DataSize;
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	VulkanHelper::CreateBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_physicalDeviceInfo.memProp, NULL, 0, UniformBuffer.m_BufObj, &bufInfo);

	// Map the GPU memory on to local host
	result = vkMapMemory(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Memory, 0, UniformBuffer.m_BufObj.m_MemRqrmnt.size, 0, (void **)&UniformBuffer.m_Data);
	assert(result == VK_SUCCESS);

	// We have only one Uniform buffer object to update
    UniformBuffer.m_MappedRange.resize(1);

	// Populate the VkMappedMemoryRange data structure
    UniformBuffer.m_MappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    UniformBuffer.m_MappedRange[0].memory = UniformBuffer.m_BufObj.m_Memory;
    UniformBuffer.m_MappedRange[0].offset = 0;
    UniformBuffer.m_MappedRange[0].size = sizeof(glm::mat4);

	// Update the local data structure with uniform buffer for house keeping
    UniformBuffer.m_BufferInfo.buffer = UniformBuffer.m_BufObj.m_Buffer;
    UniformBuffer.m_BufferInfo.offset = 0;
    UniformBuffer.m_BufferInfo.range = sizeof(glm::mat4);
}

void Cube::DestroyUniformBuffer()
{
    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Memory);
    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Buffer, NULL);
    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer.m_BufObj.m_Memory, NULL);
}

void Cube::CreateDescriptorSetLayout(bool useTexture)
{
	// Define the layout binding information for the descriptor set(before creating it)
	// Specify binding point, shader type(like vertex shader below), count etc.
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0; // DESCRIPTOR_SET_BINDING_INDEX
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = NULL;

	// If texture is being used then there existing second binding in the fragment shader
	if (useTexture) // ############## remove this flag and related stuff with textures
	{
		layoutBindings[1].binding = 1; // DESCRIPTOR_SET_BINDING_INDEX
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;
	}

	// Specify the layout bind into the VkDescriptorSetLayoutCreateInfo
	// and use it to create a descriptor set layout
	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = useTexture ? 2 : 1;
	descriptorLayout.pBindings = layoutBindings;

	VkResult  result;
	// Allocate required number of descriptor layout objects and  
	// create them using vkCreateDescriptorSetLayout()
	descLayout.resize(1);
    result = vkCreateDescriptorSetLayout(m_VulkanApplication->m_hDevice, &descriptorLayout, NULL, descLayout.data());
	assert(result == VK_SUCCESS);
}

void Cube::DestroyDescriptorLayout()
{
	for (int i = 0; i < descLayout.size(); i++) {
        vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
	}
	descLayout.clear();
}

void Cube::CreateDescriptor(bool useTexture)
{
	CreateDescriptorSetLayout(useTexture);

	// Create the uniform bufunifer resource 
	CreateUniformBuffer();

	// Create the descriptor pool and 
	// use it for descriptor set allocation
	CreateDescriptorPool(useTexture);

	// Create descriptor set with uniform buffer data in it
	CreateDescriptorSet(useTexture);
}

void Cube::CreateDescriptorPool(bool useTexture)
{
	VkResult  result;
	// Define the size of descriptor pool based on the
	// type of descriptor set being used.
	std::vector<VkDescriptorPoolSize> descriptorTypePool;

	// The first descriptor pool object is of type Uniform buffer
	descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

	// If texture is supported then define second object with 
	// descriptor type to be Image sampler
	if (useTexture) {
		descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 });
	}

	// Populate the descriptor pool state information
	// in the create info structure.
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorTypePool.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorTypePool.data();

	// Create the descriptor pool using the descriptor 
	// pool create info structure
    result = vkCreateDescriptorPool(m_VulkanApplication->m_hDevice, &descriptorPoolCreateInfo, NULL, &descriptorPool);
	assert(result == VK_SUCCESS);
}

void Cube::CreateDescriptorSet(bool useTexture)
{
	VkResult  result;

	// Create the descriptor allocation structure and specify the descriptor 
	// pool and descriptor layout
	VkDescriptorSetAllocateInfo dsAllocInfo[1];
	dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo[0].pNext = NULL;
	dsAllocInfo[0].descriptorPool = descriptorPool;
	dsAllocInfo[0].descriptorSetCount = 1;
	dsAllocInfo[0].pSetLayouts = descLayout.data();

	// Allocate the number of descriptor sets needs to be produced
	descriptorSet.resize(1);

	// Allocate descriptor sets
    result = vkAllocateDescriptorSets(m_VulkanApplication->m_hDevice, dsAllocInfo, descriptorSet.data());
	assert(result == VK_SUCCESS);

	// Allocate two write descriptors for - 1. MVP and 2. Texture
	VkWriteDescriptorSet writes[2];
	memset(&writes, 0, sizeof(writes));

	// Specify the uniform buffer related 
	// information into first write descriptor
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = NULL;
	writes[0].dstSet = descriptorSet[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &UniformBuffer.m_BufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0; // DESCRIPTOR_SET_BINDING_INDEX

							  // If texture is used then update the second write descriptor structure
	if (useTexture)
	{
		// In this sample textures are not used
		writes[1] = {};
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = descriptorSet[0];
		writes[1].dstBinding = 1; // DESCRIPTOR_SET_BINDING_INDEX
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = NULL;
		writes[1].dstArrayElement = 0;
	}

	// Update the uniform buffer into the allocated descriptor set
    vkUpdateDescriptorSets(m_VulkanApplication->m_hDevice, useTexture ? 2 : 1, writes, 0, NULL);
}

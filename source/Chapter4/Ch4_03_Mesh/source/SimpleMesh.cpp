#include "SimpleMesh.h"

#include "../../../common/VulkanHelper.h"

#include <QMainWindow>
#include <QHBoxLayout>
#include <QApplication>

using namespace std;

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 MVP;

SimpleMesh::SimpleMesh(VulkanApp* p_VulkanApp)
{
	m_hPipelineLayout = VK_NULL_HANDLE;
    m_hGraphicsPipeline = VK_NULL_HANDLE;

	memset(&Uniform, 0, sizeof(Uniform));
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));

	m_VulkanApplication = p_VulkanApp;
}

SimpleMesh::~SimpleMesh()
{
	vkDestroyPipeline(m_VulkanApplication->m_hDevice, m_hGraphicsPipeline, nullptr);

	// Destroy Vertex Buffer
	vkDestroyBuffer(m_VulkanApplication->m_hDevice, VertexBuffer.m_Buffer, NULL);
	vkFreeMemory(m_VulkanApplication->m_hDevice, VertexBuffer.m_Memory, NULL);

	// Destroy descriptors
	for (int i = 0; i < descLayout.size(); i++) {
		vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
	}
	descLayout.clear();

	vkDestroyPipelineLayout(m_VulkanApplication->m_hDevice, m_hPipelineLayout, nullptr);

	vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
	vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);

	vkUnmapMemory(m_VulkanApplication->m_hDevice, Uniform.memory);
	vkDestroyBuffer(m_VulkanApplication->m_hDevice, Uniform.buffer, NULL);
	vkFreeMemory(m_VulkanApplication->m_hDevice, Uniform.memory, NULL);
}

void SimpleMesh::Setup()
{
	//CreateVertexBuffer(s_TriangleVertices, sizeof(s_TriangleVertices), sizeof(Vertex));
//	LoadMesh();
	LoadMeshNew();
	CreateDescriptor(false);
	CreateGraphicsPipeline();

	CreateCommandBuffers(); // Create command buffers //XXXXXXXXXXXXXXXX

	////////////////////////////////////////////////////
	///////////////////////////
	RecordCommandBuffer();
}

void SimpleMesh::Update()
{
	Projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(0, 0, 100),		// Camera is in World Space
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, 1, 0)		// Head is up
		);
	Model = glm::mat4(1.0f);
	static float rot = 0;
	rot += .0005f;
	Model = glm::rotate(Model, rot, glm::vec3(0.0, 1.0, 0.0)) * glm::rotate(Model, rot, glm::vec3(1.0, 1.0, 1.0));

	glm::mat4 MVP = Projection * View * Model;

	// Invalidate the range of mapped buffer in order to make it visible to the host.
	// If the memory property is set with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	// then the driver may take care of this, otherwise for non-coherent 
	// mapped memory vkInvalidateMappedMemoryRanges() needs to be called explicitly.
	VkResult res = vkInvalidateMappedMemoryRanges(m_VulkanApplication->m_hDevice, 1, &Uniform.mappedRange[0]);
	assert(res == VK_SUCCESS);

	// Copy updated data into the mapped memory
	memcpy(Uniform.pData, &MVP, sizeof(MVP));

	// Flush the range of mapped buffer in order to make it visible to the device
	// If the memory is coherent (memory property must be beVK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	// then the driver may take care of this, otherwise for non-coherent 
	// mapped memory vkFlushMappedMemoryRanges() needs to be called explicitly to flush out 
	// the pending writes on the host side.
	res = vkFlushMappedMemoryRanges(m_VulkanApplication->m_hDevice, 1, &Uniform.mappedRange[0]);
	assert(res == VK_SUCCESS);
}

void SimpleMesh::CreateGraphicsPipeline()
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

	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
	depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateInfo.pNext = NULL;
	depthStencilStateInfo.flags = 0;
	depthStencilStateInfo.depthTestEnable = true;
	depthStencilStateInfo.depthWriteEnable = true;
	depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStateInfo.back.compareMask = 0;
	depthStencilStateInfo.back.reference = 0;
	depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.writeMask = 0;
	depthStencilStateInfo.minDepthBounds = 0;
	depthStencilStateInfo.maxDepthBounds = 0;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateInfo.front = depthStencilStateInfo.back;

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
	pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

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

void SimpleMesh::RecordCommandBuffer()
{
	// Specify the clear color value
	VkClearValue clearColor[2];
	clearColor[0].color.float32[0] = 0.0f;
	clearColor[0].color.float32[1] = 0.0f;
	clearColor[0].color.float32[2] = 0.0f;
	clearColor[0].color.float32[3] = 0.0f;

	// Specify the depth/stencil clear value
	clearColor[1].depthStencil.depth = 1.0f;
	clearColor[1].depthStencil.stencil = 0;

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

		// Begin command buffer
		vkBeginCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_VulkanApplication->m_hRenderPass;
		renderPassInfo.framebuffer = m_VulkanApplication->m_hFramebuffers[i];
		renderPassInfo.renderArea.offset = renderOffset;
		renderPassInfo.renderArea.extent = renderExtent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColor;

		// Begin render pass
		vkCmdBeginRenderPass(m_VulkanApplication->m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind graphics pipeline
		vkCmdBindPipeline(m_VulkanApplication->m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_hGraphicsPipeline);

		vkCmdBindDescriptorSets(m_VulkanApplication->m_hCommandBufferList[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_hPipelineLayout,
			0, 1, descriptorSet.data(), 0, NULL);

		// Specify vertex buffer information
		const VkDeviceSize offsets[1] = { 0 };
		//vkCmdBindVertexBuffers(m_VulkanApplication->m_hCommandBufferList[i], 0, 1, &VertexBuffer.m_Buffer, offsets);
		
		/////////////////////////////////////////////////////////////////////
		// Bind mesh vertex buffer
		vkCmdBindVertexBuffers(m_VulkanApplication->m_hCommandBufferList[i], 0, 1, &mesh.m_Vertices.m_Buffer, offsets);

		// Bind mesh index buffer
		vkCmdBindIndexBuffer(m_VulkanApplication->m_hCommandBufferList[i], mesh.m_Indices.m_Buffer, 0, VK_INDEX_TYPE_UINT32);

		// Render mesh vertex buffer using it's indices
		vkCmdDrawIndexed(m_VulkanApplication->m_hCommandBufferList[i], mesh.m_Indices.m_IndexCount, 1, 0, 0, 0);
		////////////////////////////////////////////////////////////////////

		// End the Render pass
		vkCmdEndRenderPass(m_VulkanApplication->m_hCommandBufferList[i]);

		VkResult vkResult = vkEndCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i]);
		if (vkResult != VK_SUCCESS)
		{
			VulkanHelper::LogError("vkEndCommandBuffer() failed!");
			assert(false);
		}
	}
}

void SimpleMesh::CreateCommandBuffers()
{
	m_VulkanApplication->CreateCommandBuffers();
}

bool SimpleMesh::Load(const char* filename)
{
	pScene = Importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

	if (pScene)
	{
		m_Entries.clear();
		m_Entries.resize(pScene->mNumMeshes);
		// Read in all meshes in the scene
		for (auto i = 0; i < m_Entries.size(); i++)
		{
			//m_Entries[i].vertexBase = numVertices;
			numVertices += pScene->mMeshes[i]->mNumVertices;
			const aiMesh* paiMesh = pScene->mMeshes[i];
			MeshInit(&m_Entries[i], paiMesh, pScene);
		}
		return true;
	}
	else
	{
		VulkanHelper::LogError("Error parsing");
		//VulkanHelper::LogError("Error parsing '%s': '%s'", filename, Importer.GetErrorString());
		return false;
	}

}

void SimpleMesh::MeshInit(MeshEntry *meshEntry, const aiMesh* paiMesh, const aiScene* pScene)
{
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		aiVector3D* position = &(paiMesh->mVertices[i]);
		meshEntry->Vertices.push_back(Vertex(glm::vec3(position->x, -position->y, position->z)));
	}

	uint32_t indexBase = static_cast<uint32_t>(meshEntry->Indices.size());
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		if (Face.mNumIndices != 3)
			continue;
		meshEntry->Indices.push_back(indexBase + Face.mIndices[0]);
		meshEntry->Indices.push_back(indexBase + Face.mIndices[1]);
		meshEntry->Indices.push_back(indexBase + Face.mIndices[2]);
	}
}

void SimpleMesh::LoadMeshNew()
{
	//////////////////////////////////////////////////////////////////////////////////////
	Load("../../../resources/teapot.dae");

	// Generate vertex buffer
	std::vector<Vertex> vertexBuffer;
	// Iterate through all meshes in the file
	// and extract the vertex information used in this demo
	for (uint32_t m = 0; m < m_Entries.size(); m++)
	{
		for (uint32_t i = 0; i < m_Entries[m].Vertices.size(); i++)
		{
			Vertex vertex(m_Entries[m].Vertices[i].m_pos);
			vertexBuffer.push_back(vertex);
		}
	}
	uint32_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);

	// Generate index buffer from loaded mesh file
	std::vector<uint32_t> indexBuffer;
	for (uint32_t m = 0; m < m_Entries.size(); m++)
	{
		uint32_t indexBase = indexBuffer.size();
		for (uint32_t i = 0; i < m_Entries[m].Indices.size(); i++)
		{
			indexBuffer.push_back(m_Entries[m].Indices[i] + indexBase);
		}
	}

	uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
	mesh.m_Indices.m_IndexCount = indexBuffer.size();

	// Static mesh should always be device local

	bool useStaging = true;

	if (useStaging)
	{
		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create staging buffers
		// Vertex data
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			vertexBufferSize,
			vertexBuffer.data(),
			&vertexStaging.buffer,
			&vertexStaging.memory);
		// Index data
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			indexBufferSize,
			indexBuffer.data(),
			&indexStaging.buffer,
			&indexStaging.memory);

		// Create device local buffers
		// Vertex buffer
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			vertexBufferSize,
			nullptr,
			&mesh.m_Vertices.m_Buffer,
			&mesh.m_Vertices.m_Memory);
		// Index buffer
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			indexBufferSize,
			nullptr,
			&mesh.m_Indices.m_Buffer,
			&mesh.m_Indices.m_Memory);


		// Copy from staging buffers
		//VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkCommandBuffer copyCmd;
		VulkanHelper::AllocateCommandBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, &copyCmd);
		VulkanHelper::BeginCommandBuffer(copyCmd);

		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			vertexStaging.buffer,
			mesh.m_Vertices.m_Buffer,
			1,
			&copyRegion);

		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			indexStaging.buffer,
			mesh.m_Indices.m_Buffer,
			1,
			&copyRegion);

		//VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);
		VulkanHelper::EndCommandBuffer(copyCmd);
		VulkanHelper::SubmitCommandBuffer(m_VulkanApplication->m_hGraphicsQueue, copyCmd);
		vkFreeCommandBuffers(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, 1, &copyCmd);

		vkDestroyBuffer(m_VulkanApplication->m_hDevice, vertexStaging.buffer, nullptr);
		vkFreeMemory(m_VulkanApplication->m_hDevice, vertexStaging.memory, nullptr);
		vkDestroyBuffer(m_VulkanApplication->m_hDevice, indexStaging.buffer, nullptr);
		vkFreeMemory(m_VulkanApplication->m_hDevice, indexStaging.memory, nullptr);
	}
	else
	{
		// Vertex buffer
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			vertexBufferSize,
			vertexBuffer.data(),
			&mesh.m_Vertices.m_Buffer,
			&mesh.m_Vertices.m_Memory);
		// Index buffer
		VulkanHelper::createBuffer(
			m_VulkanApplication->m_hDevice,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_VulkanApplication->m_physicalDeviceInfo.memProp,
			indexBufferSize,
			indexBuffer.data(),
			&mesh.m_Indices.m_Buffer,
			&mesh.m_Indices.m_Memory);
	}

	// Indicates the rate at which the information will be
	// injected for vertex input.
	m_VertexInputBinding.binding = 0;
	m_VertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	m_VertexInputBinding.stride = sizeof(Vertex);

	// The VkVertexInputAttribute interpreting the data.
	m_VertexInputAttribute[0].binding = 0;
	m_VertexInputAttribute[0].location = 0;
	m_VertexInputAttribute[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_VertexInputAttribute[0].offset = offsetof(struct Vertex, m_pos);
	//////////////////////////////////////////////////////////////////////////////////////
}

void SimpleMesh::LoadMesh()
{
	////////////////////////////////////////////////////////////////////////////////////////
	//MeshLoader* meshLoader = new MeshLoader(m_VulkanApplication);
	//meshLoader->Load("../../../resources/teapot.dae");

	//// Generate vertex buffer
	//float scale = 1.0f;
	//std::vector<VertexUI> vertexBuffer;
	//// Iterate through all meshes in the file
	//// and extract the vertex information used in this demo
	//for (uint32_t m = 0; m < meshLoader->m_Entries.size(); m++)
	//{
	//	for (uint32_t i = 0; i < meshLoader->m_Entries[m].Vertices.size(); i++)
	//	{
	//		VertexUI vertex;

	//		vertex.pos = meshLoader->m_Entries[m].Vertices[i].m_pos * scale;
	//		//vertex.normal = meshLoader->m_Entries[m].Vertices[i].m_normal;
	//		//vertex.uv = meshLoader->m_Entries[m].Vertices[i].m_tex;
	//		//vertex.color = meshLoader->m_Entries[m].Vertices[i].m_color;

	//		vertexBuffer.push_back(vertex);
	//	}
	//}
	//uint32_t vertexBufferSize = vertexBuffer.size() * sizeof(VertexUI);

	//// Generate index buffer from loaded mesh file
	//std::vector<uint32_t> indexBuffer;
	//for (uint32_t m = 0; m < meshLoader->m_Entries.size(); m++)
	//{
	//	uint32_t indexBase = indexBuffer.size();
	//	for (uint32_t i = 0; i < meshLoader->m_Entries[m].Indices.size(); i++)
	//	{
	//		indexBuffer.push_back(meshLoader->m_Entries[m].Indices[i] + indexBase);
	//	}
	//}

	//uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
	//mesh.indices.count = indexBuffer.size();

	//// Static mesh should always be device local

	//bool useStaging = true;

	//if (useStaging)
	//{
	//	struct {
	//		VkBuffer buffer;
	//		VkDeviceMemory memory;
	//	} vertexStaging, indexStaging;

	//	// Create staging buffers
	//	// Vertex data
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		vertexBufferSize,
	//		vertexBuffer.data(),
	//		&vertexStaging.buffer,
	//		&vertexStaging.memory);
	//	// Index data
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		indexBufferSize,
	//		indexBuffer.data(),
	//		&indexStaging.buffer,
	//		&indexStaging.memory);

	//	// Create device local buffers
	//	// Vertex buffer
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		vertexBufferSize,
	//		nullptr,
	//		&mesh.vertices.buf,
	//		&mesh.vertices.mem);
	//	// Index buffer
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		indexBufferSize,
	//		nullptr,
	//		&mesh.indices.buf,
	//		&mesh.indices.mem);


	//	// Copy from staging buffers
	//	//VkCommandBuffer copyCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	//	VkCommandBuffer copyCmd;
	//	VulkanHelper::AllocateCommandBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, &copyCmd);
	//	VulkanHelper::BeginCommandBuffer(copyCmd);

	//	VkBufferCopy copyRegion = {};

	//	copyRegion.size = vertexBufferSize;
	//	vkCmdCopyBuffer(
	//		copyCmd,
	//		vertexStaging.buffer,
	//		mesh.vertices.buf,
	//		1,
	//		&copyRegion);

	//	copyRegion.size = indexBufferSize;
	//	vkCmdCopyBuffer(
	//		copyCmd,
	//		indexStaging.buffer,
	//		mesh.indices.buf,
	//		1,
	//		&copyRegion);

	//	//VulkanExampleBase::flushCommandBuffer(copyCmd, queue, true);
	//	VulkanHelper::EndCommandBuffer(copyCmd);
	//	VulkanHelper::SubmitCommandBuffer(m_VulkanApplication->m_hGraphicsQueue, copyCmd);
	//	vkFreeCommandBuffers(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, 1, &copyCmd);

	//	vkDestroyBuffer(m_VulkanApplication->m_hDevice, vertexStaging.buffer, nullptr);
	//	vkFreeMemory(m_VulkanApplication->m_hDevice, vertexStaging.memory, nullptr);
	//	vkDestroyBuffer(m_VulkanApplication->m_hDevice, indexStaging.buffer, nullptr);
	//	vkFreeMemory(m_VulkanApplication->m_hDevice, indexStaging.memory, nullptr);
	//}
	//else
	//{
	//	// Vertex buffer
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		vertexBufferSize,
	//		vertexBuffer.data(),
	//		&mesh.vertices.buf,
	//		&mesh.vertices.mem);
	//	// Index buffer
	//	VulkanHelper::createBuffer(
	//		m_VulkanApplication->m_hDevice,
	//		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	//		m_VulkanApplication->m_physicalDeviceInfo.memProp,
	//		indexBufferSize,
	//		indexBuffer.data(),
	//		&mesh.indices.buf,
	//		&mesh.indices.mem);
	//}

	//delete(meshLoader);

	//// Indicates the rate at which the information will be
	//// injected for vertex input.
	//m_VertexInputBinding.binding = 0;
	//m_VertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	//m_VertexInputBinding.stride = sizeof(VertexUI);

	//// The VkVertexInputAttribute interpreting the data.
	//m_VertexInputAttribute[0].binding = 0;
	//m_VertexInputAttribute[0].location = 0;
	//m_VertexInputAttribute[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	//m_VertexInputAttribute[0].offset = offsetof(struct VertexUI, pos);
	////////////////////////////////////////////////////////////////////////////////////////
}

void SimpleMesh::CreateUniformBuffer()
{
	VkResult  result;
	bool  pass;
	Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(10, 3, 10),	// Camera in World Space
		glm::vec3(0, 0, 0),		// and looks at the origin
		glm::vec3(0, -1, 0)		// Head is up
		);
	Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;

	// Create buffer resource states using VkBufferCreateInfo
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufInfo.size = sizeof(MVP);
	bufInfo.queueFamilyIndexCount = 0;
	bufInfo.pQueueFamilyIndices = NULL;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufInfo.flags = 0;

	// Use create buffer info and create the buffer objects
	result = vkCreateBuffer(m_VulkanApplication->m_hDevice, &bufInfo, NULL, &Uniform.buffer);
	assert(result == VK_SUCCESS);

	// Get the buffer memory requirements
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(m_VulkanApplication->m_hDevice, Uniform.buffer, &memRqrmnt);

	VkMemoryAllocateInfo memAllocInfo = {}; // ############# rename this
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = memRqrmnt.size;

	// Determine the type of memory required 
	// with the help of memory properties
	//pass = VulkanHelper::MemoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);
	pass = VulkanHelper::MemoryTypeFromProperties(m_VulkanApplication->m_physicalDeviceInfo.memProp, memRqrmnt.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memAllocInfo.memoryTypeIndex);

	assert(pass);

	// Allocate the memory for buffer objects
	result = vkAllocateMemory(m_VulkanApplication->m_hDevice, &memAllocInfo, NULL, &(Uniform.memory));
	assert(result == VK_SUCCESS);

	// Map the GPU memory on to local host
	result = vkMapMemory(m_VulkanApplication->m_hDevice, Uniform.memory, 0, memRqrmnt.size, 0, (void **)&Uniform.pData);
	assert(result == VK_SUCCESS);

	// Copy computed data in the mapped buffer
	memcpy(Uniform.pData, &MVP, sizeof(MVP));

	// We have only one Uniform buffer object to update
	Uniform.mappedRange.resize(1);

	// Populate the VkMappedMemoryRange data structure
	Uniform.mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	Uniform.mappedRange[0].memory = Uniform.memory;
	Uniform.mappedRange[0].offset = 0;
	Uniform.mappedRange[0].size = sizeof(MVP);

	// Invalidate the range of mapped buffer in order to make it visible to the host.
	// If the memory property is set with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	// then the driver may take care of this, otherwise for non-coherent 
	// mapped memory vkInvalidateMappedMemoryRanges() needs to be called explicitly.
	vkInvalidateMappedMemoryRanges(m_VulkanApplication->m_hDevice, 1, &Uniform.mappedRange[0]);

	// Bind the buffer device memory 
	result = vkBindBufferMemory(m_VulkanApplication->m_hDevice, Uniform.buffer, Uniform.memory, 0);
	assert(result == VK_SUCCESS);

	// Update the local data structure with uniform buffer for house keeping
	Uniform.bufferInfo.buffer = Uniform.buffer;
	Uniform.bufferInfo.offset = 0;
	Uniform.bufferInfo.range = sizeof(MVP);
	Uniform.memRqrmnt = memRqrmnt;
}

void SimpleMesh::DestroyUniformBuffer()
{
	vkUnmapMemory(m_VulkanApplication->m_hDevice, Uniform.memory);
	vkDestroyBuffer(m_VulkanApplication->m_hDevice, Uniform.buffer, NULL);
	vkFreeMemory(m_VulkanApplication->m_hDevice, Uniform.memory, NULL);
}

void SimpleMesh::CreateDescriptorSetLayout(bool useTexture)
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

void SimpleMesh::DestroyDescriptorLayout()
{
	for (int i = 0; i < descLayout.size(); i++) {
		vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
	}
	descLayout.clear();
}

void SimpleMesh::CreateDescriptor(bool useTexture)
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

void SimpleMesh::CreateDescriptorPool(bool useTexture)
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

void SimpleMesh::CreateDescriptorSet(bool useTexture)
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
	writes[0].pBufferInfo = &Uniform.bufferInfo;
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
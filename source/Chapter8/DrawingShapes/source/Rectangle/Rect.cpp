#include "Rect.h"

#include "RectangleDescriptorSet.h"
#include "RectangleGeometry.h"
#include "RectangleMultiDrawScheme.h"
#include "RectangleShaderTypes.h"

Rectangl::Rectangl(Scene *p_Scene, Node *p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name, SHAPE p_ShapeType)
    : Node(p_Scene, p_Parent, p_BoundedRegion, p_Name, p_ShapeType)
    , m_DrawType(FILLED)
{
}

RenderSchemeFactory* Rectangl::GetRenderSchemeFactory()
{
    if (m_ShapeType == SHAPE::SHAPE_RECTANGLE_MULTIDRAW)
    {
        return new RectangleMultiDrawScheme(static_cast<VulkanApp*>(m_Scene->GetApplication()));
    }
    else if (m_ShapeType == SHAPE::SHAPE_RECTANGLE_INSTANCED)
    {
        return new RectangleMultiDrawScheme(static_cast<VulkanApp*>(m_Scene->GetApplication()));
    }

    assert(false);
    return NULL;
}

void Rectangl::Setup()
{
    CreateRectVertexBuffer();

    Node::Setup();
}

void Rectangl::CreateRectVertexBuffer()
{
    glm::mat4 parentTransform = GetAbsoluteTransformation();//m_Model * GetParentsTransformation(GetParent());

    Vertex rectVertices[6];
    memcpy(rectVertices, rectFilledVertices, sizeof(Vertex) * 6);
    uint32_t dataSize = sizeof(rectVertices);
    uint32_t dataStride = sizeof(rectVertices[0]);
    const int vertexCount = dataSize / dataStride;
    for (int i = 0; i < vertexCount; ++i)
    {
        glm::vec4 pos(rectFilledVertices[i].m_Position, 1.0);
        pos.x = pos.x * m_BoundedRegion.m_Dimension.x;
        pos.y = pos.y * m_BoundedRegion.m_Dimension.y;

        pos = parentTransform * pos;
        //std::cout << m_Name.toStdString() << "=+ x:" << pos.x << ", y:" << pos.y << ", z:" << pos.z << endl;

        rectVertices[i].m_Position.x = pos.x;
        rectVertices[i].m_Position.y = pos.y;
        rectVertices[i].m_Position.z = pos.z;
    }

    VulkanApp* app = static_cast<VulkanApp*>(m_Scene->GetApplication());
    const VkDevice& device = app->m_hDevice;

    if (m_VertexBuffer.m_Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, m_VertexBuffer.m_Buffer, NULL);
        vkFreeMemory(device, m_VertexBuffer.m_Memory, NULL);
    }

    m_VertexBuffer.m_DataSize = dataSize;
    m_VertexBuffer.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    const VkPhysicalDeviceMemoryProperties& memProp = app->m_physicalDeviceInfo.memProp;
    VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectVertices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//RectangleMultiDrawScheme::RectangleMultiDrawScheme(VulkanApp* p_VulkanApp)
//{
//    assert(p_VulkanApp);
//    m_VulkanApplication = p_VulkanApp;

//    m_DescriptorSet = NULL;
//}

//RectangleMultiDrawScheme::~RectangleMultiDrawScheme()
//{
//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
//        const int modelSize = m_ModelList.size();
//        if (!modelSize) continue;

//        // Destroy Vertex Buffer
//        for (int j = 0; j < modelSize; j++)
//        {
//            if (m_ModelList.at(j)->GetRefShapeType() == SHAPE::SHAPE_RECTANGLE_MULTIDRAW)
//            {
//                Rectangl* model = (static_cast<Rectangl*>(m_ModelList.at(j)));
//                if (!model) return;

//                vkDestroyBuffer(m_VulkanApplication->m_hDevice, model->m_VertexBuffer.m_Buffer, NULL);
//                vkFreeMemory(m_VulkanApplication->m_hDevice, model->m_VertexBuffer.m_Memory, NULL);
//            }
//        }
//    }

//    //// Destroy descriptors
//    //for (int i = 0; i < descLayout.size(); i++) {
//    //       vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
//    //}
//    //descLayout.clear();

//    //vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
//    //vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);

//    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
//    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
//    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
//    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
//}

//void RectangleMultiDrawScheme::Setup()
//{
//    m_DescriptorSet = std::make_shared<RectangleDescriptorSet>(m_VulkanApplication);

//    CreateVertexLayoutBinding();

//    CreateGraphicsPipeline();

//    // Build the push constants
//    createPushConstants();
//}

//void RectangleMultiDrawScheme::Update()
//{
//    RectangleDescriptorSet::UniformBufferObj* UniformBuffer = m_DescriptorSet->UniformBuffer;
//    VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
//        UniformBuffer->m_MappedMemory,
//        UniformBuffer->m_MappedRange,
//        UniformBuffer->m_BufObj.m_MemoryFlags,
//        &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));
//}

//void RectangleMultiDrawScheme::ResizeWindow(VkCommandBuffer& p_CommandBuffer)
//{
//    CreateGraphicsPipeline(true);

//    Render(p_CommandBuffer);
//}

//void RectangleMultiDrawScheme::CreateRectOutlinePipeline()
//{
//    // Compile the vertex shader
//#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceVert.spv"); // Relative path to binary output dir
//                                                                                                                                      // Setup the vertex shader stage create info structures
//#elif __APPLE__
//    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/RectInstanceVert.spv");
////    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice,
////    "/Users/Parminder/Dev/MVK/VulkanByExample-master/source/Chapter8/DrawingShapes/source/shaders/RectInstanceVert.spv");
//#endif
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShader;
//    vertShaderStageInfo.pName = "main";

//    // Compile the fragment shader
//#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceFrag.spv"); // Relative path to binary output dir
//                                                                                                                                      // Setup the fragment shader stage create info structures
//#elif __APPLE__
//    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/RectInstanceFrag.spv");
////    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice,
////    "/Users/Parminder/Dev/MVK/VulkanByExample-master/source/Chapter8/DrawingShapes/source/shaders/RectInstanceFrag.spv");
//#endif

//    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShader;
//    fragShaderStageInfo.pName = "main";

//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//    // Setup the vertex input
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    // Check the size where every necessrry
//    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexInputBinding[PIPELINE_OUTLINE].size();
//    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_OUTLINE].data();
//    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexInputAttribute[PIPELINE_OUTLINE].size();
//    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

//    // Setup input assembly
//    // We will be rendering 1 triangle using triangle strip topology
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;

//    // Setup viewport to the maximum widht and height of the window
//    VkViewport viewport = {};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
//    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;

//    // Setup scissor rect
//    VkRect2D scissor = {};
//    scissor.offset = { 0, 0 };
//    scissor.extent = m_VulkanApplication->m_swapChainExtent;

//    // Setup view port state
//    VkPipelineViewportStateCreateInfo viewportState = {};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;

//    // Setup the rasterizer state
//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_NONE;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//    rasterizer.depthClampEnable = true;

//    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
//    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencilStateInfo.pNext = NULL;
//    depthStencilStateInfo.flags = 0;
//    depthStencilStateInfo.depthTestEnable = true;
//    depthStencilStateInfo.depthWriteEnable = true;
//    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
//    depthStencilStateInfo.back.compareMask = 0;
//    depthStencilStateInfo.back.reference = 0;
//    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.writeMask = 0;
//    depthStencilStateInfo.minDepthBounds = 0;
//    depthStencilStateInfo.maxDepthBounds = 0;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.front = depthStencilStateInfo.back;

//    // Setup multi sampling. In our first example we will be using single sampling mode
//    VkPipelineMultisampleStateCreateInfo multisampling = {};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = NUM_SAMPLES;

//    // Setup color output masks.
//    // Set to write out RGBA components
//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_FALSE;

//    // Setup color blending
//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;

//    // Create pipeline layout
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 0;
//    pipelineLayoutInfo.pushConstantRangeCount = 0;
//    pipelineLayoutInfo.setLayoutCount = (uint32_t)m_DescriptorSet->descLayout.size();
//    pipelineLayoutInfo.pSetLayouts = m_DescriptorSet->descLayout.data();

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
//        assert(false);
//    }

//    // Create graphics pipeline
//    VkGraphicsPipelineCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = graphicsPipelineLayout;
//    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

//    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
//    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
//        assert(false);
//    }

//    m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

//    // Cleanup
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
//}

//void RectangleMultiDrawScheme::CreateRectFillPipeline()
//{
//    // Compile the vertex shader
//#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceVert.spv"); // Relative path to binary output dir
//#elif __APPLE__
//    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/RectInstanceVert.spv");
//#endif
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShader;
//    vertShaderStageInfo.pName = "main";

//    // Compile the fragment shader
//#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/RectInstanceFrag.spv"); // Relative path to binary output dir
//#elif __APPLE__
//    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/RectInstanceFrag.spv");
//#endif

//    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShader;
//    fragShaderStageInfo.pName = "main";

//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//    // Setup the vertex input
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexInputBinding[PIPELINE_FILLED].size();
//    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_FILLED].data();
//    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexInputAttribute[PIPELINE_FILLED].size();
//    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_FILLED].data();

//    // Setup input assembly
//    // We will be rendering 1 triangle using triangle strip topology
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;

//    // Setup viewport to the maximum widht and height of the window
//    VkViewport viewport = {};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
//    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;

//    // Setup scissor rect
//    VkRect2D scissor = {};
//    scissor.offset = { 0, 0 };
//    scissor.extent = m_VulkanApplication->m_swapChainExtent;

//    // Setup view port state
//    VkPipelineViewportStateCreateInfo viewportState = {};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;

//    // Setup the rasterizer state
//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//    rasterizer.depthClampEnable = true;

//    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
//    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencilStateInfo.pNext = NULL;
//    depthStencilStateInfo.flags = 0;
//    depthStencilStateInfo.depthTestEnable = true;
//    depthStencilStateInfo.depthWriteEnable = true;
//    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
//    depthStencilStateInfo.back.compareMask = 0;
//    depthStencilStateInfo.back.reference = 0;
//    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.writeMask = 0;
//    depthStencilStateInfo.minDepthBounds = 0;
//    depthStencilStateInfo.maxDepthBounds = 0;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.front = depthStencilStateInfo.back;

//    // Setup multi sampling. In our first example we will be using single sampling mode
//    VkPipelineMultisampleStateCreateInfo multisampling = {};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = NUM_SAMPLES;

//    // Setup color output masks.
//    // Set to write out RGBA components
//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_TRUE;

//    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

//    // Setup color blending
//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;

//    // Setup the push constant range
//    const unsigned pushConstantRangeCount = 1;
//    VkPushConstantRange pushConstantRanges[pushConstantRangeCount] = {};
//    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    pushConstantRanges[0].offset = 0;
//    pushConstantRanges[0].size = 16 + sizeof(glm::mat4);
//    //pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    //pushConstantRanges[1].offset = 16;
//    //pushConstantRanges[1].size = sizeof(glm::mat4);

//    // Create pipeline layout
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 0;
//    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
//    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
//    pipelineLayoutInfo.setLayoutCount = (uint32_t)m_DescriptorSet->descLayout.size();
//    pipelineLayoutInfo.pSetLayouts = m_DescriptorSet->descLayout.data();

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
//        assert(false);
//    }

//    // Create graphics pipeline
//    VkGraphicsPipelineCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = graphicsPipelineLayout;
//    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

//    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
//    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
//        assert(false);
//    }

//    m_GraphicsPipelineMap[PIPELINE_RECT_FILLED] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

//    // Cleanup
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
//}

//void RectangleMultiDrawScheme::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
//{
//    if (p_ClearGraphicsPipelineMap)
//    {
//        const VkDevice& device = m_VulkanApplication->m_hDevice;
//        QMap<QString, QPair<VkPipeline, VkPipelineLayout> >::iterator i;
//        for (i = m_GraphicsPipelineMap.begin(); i != m_GraphicsPipelineMap.end(); ++i)
//        {
//            vkDestroyPipeline(m_VulkanApplication->m_hDevice, i.value().first, nullptr);
//            vkDestroyPipelineLayout(device, i.value().second, nullptr);
//        }

//        m_GraphicsPipelineMap.clear();
//    }

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
//    if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
//    {
//        graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
//        graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
//        return;
//    }

//    CreateRectFillPipeline();
//    CreateRectOutlinePipeline();
//}

//void RectangleMultiDrawScheme::createPushConstants()
//{
//    return;
//    VkCommandBuffer copyCmd;
//    VulkanHelper::AllocateCommandBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, &copyCmd);
//    VulkanHelper::BeginCommandBuffer(copyCmd);

//    enum ColorFlag {
//        RED = 1,
//        GREEN = 2,
//        BLUE = 3,
//        MIXED_COLOR = 4,
//    };

//    float mixerValue = 0.3f;
//    unsigned constColorRGBFlag = BLUE;

//    // Create push constant data, this contain a constant
//    // color flag and mixer value for non-const color
//    unsigned pushConstants[2] = {};
//    pushConstants[0] = constColorRGBFlag;
//    memcpy(&pushConstants[1], &mixerValue, sizeof(float));

//    // Check if number of push constants does not exceed the allowed size
//    int maxPushContantSize = m_VulkanApplication->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
//    if (sizeof(pushConstants) > maxPushContantSize) {
//        assert(0);
//        printf("Push constand size is greater than expected, max allow size is %d", maxPushContantSize);
//    }

//    //for each (VulkanDrawable* drawableObj in drawableList)
//    //{
//    //    vkCmdPushConstants(cmdPushConstant, drawableObj->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants);
//    //}

//    VulkanHelper::EndCommandBuffer(copyCmd);
//    VulkanHelper::SubmitCommandBuffer(m_VulkanApplication->m_hGraphicsQueue, copyCmd);
//    vkFreeCommandBuffers(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_hCommandPool, 1, &copyCmd);

//    //CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdPushConstant);
//    //CommandBufferMgr::beginCommandBuffer(cmdPushConstant);


//    //CommandBufferMgr::endCommandBuffer(cmdPushConstant);
//    //CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdPushConstant);
//}

//void RectangleMultiDrawScheme::CreateVertexLayoutBinding()
//{
//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        if (pipelineIdx == PIPELINE_FILLED)
//        {
//            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
//            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

//            // Indicates the rate at which the information will be
//            // injected for vertex input.
//            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

//            // The VkVertexInputAttribute interpreting the data.
//            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][0].location = 0;
//            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

//            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][1].location = 1;
//            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);

////            m_VertexInputAttribute[pipelineIdx][2].binding = VERTEX_BUFFER_BIND_IDX;
////            m_VertexInputAttribute[pipelineIdx][2].location = 2;
////            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32_UINT;
////            m_VertexInputAttribute[pipelineIdx][2].offset = offsetof(struct Vertex, m_DrawType);
//        }
//        else if (pipelineIdx == PIPELINE_OUTLINE)
//        {
//            m_VertexInputBinding[pipelineIdx].resize(1);   // 0 for position and 1 for color
//            m_VertexInputAttribute[pipelineIdx].resize(2); // Why 2 = 2(for position and color

//            // Indicates the rate at which the information will be
//            // injected for vertex input.
//            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//            m_VertexInputBinding[pipelineIdx][0].stride = sizeof(Vertex);

//            // The VkVertexInputAttribute interpreting the data.
//            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][0].location = 0;
//            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

//            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][1].location = 1;
//            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
//        }
//    }
//}

//void RectangleMultiDrawScheme::UpdateNodeList(Node *p_Item)
//{
//    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Item);
//    assert(rectangle);

//    // Note: Based on the draw type push the model in respective pipelines
//    // Keep the draw type loose couple with the pipeline type,
//    // they may be in one-to-one correspondence but that is not necessary.
//    switch (rectangle->GetDrawType())
//    {
//    case Rectangl::FILLED:
//        m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
//        break;

//    case Rectangl::OUTLINE:
//        m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
//        break;

//    case Rectangl::ROUNDED:
//        // TODO
//        break;

//    default:
//        break;
//    }
//}

//void RectangleMultiDrawScheme::RemoveNodeList(Node* p_Model)
//{
////    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Model);
////    assert(rectangle);

////    std::vector<Node*>* modelVector = &m_PipelineTypeModelVector[rectangle->GetDrawType()];
////    if (!modelVector) return;

////    bool isUpdated = false;
////    while (1)
////    {
////        auto result = std::find(std::begin(*modelVector), std::end(*modelVector), rectangle);
////        if (result == std::end(*modelVector)) break;

////        modelVector->erase(result);
////        isUpdated = true;
////    }

////    if (isUpdated)
////    {
////        RECTANGLE_GRAPHICS_PIPELINES pipeline = PIPELINE_COUNT;

////        switch (rectangle->GetDrawType())
////        {
////        case Rectangl::FILLED:
////            pipeline = PIPELINE_FILLED;
////            break;

////        case Rectangl::OUTLINE:
////            pipeline = PIPELINE_OUTLINE;
////            break;

////        case Rectangl::ROUNDED:
////            // TODO
////            break;

////        default:
////            break;
////        }

////        if (pipeline == PIPELINE_COUNT) return;

////        PrepareInstanceData(pipeline);
////    }
//}

//void RectangleMultiDrawScheme::Prepare(Scene* p_Scene)
//{
//    // Update the uniform
//    //if (!CDS)
//    //{
//    //	CDS = std::make_shared<RectangleDescriptorSet>(m_VulkanApplication);
//    //	CDS->CreateDescriptor();
//    //	UniformBuffer = CDS->UniformBuffer;
//    //}

//    //VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
//    //	UniformBuffer->m_MappedMemory,
//    //	UniformBuffer->m_MappedRange,
//    //	UniformBuffer->m_BufObj.m_MemoryFlags,
//    //	&m_Transform, sizeof(m_Transform));
//}
//#include <QDebug>
//void RectangleMultiDrawScheme::Render(VkCommandBuffer& p_CmdBuffer)
//{
//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        NodeVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
//        const int modelSize = m_ModelList.size();
//        if (!modelSize) continue;

//        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//        VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
//        if (pipelineIdx == PIPELINE_FILLED)
//        {
//            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
//            {
//                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
//                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
//            }
//        }
//        else if (pipelineIdx == PIPELINE_OUTLINE)
//        {
//            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_OUTLINE))
//            {
//                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].first;
//                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].second;
//            }
//        }
//        else
//        {
//            assert(false);
//        }

//        vkCmdBindDescriptorSets(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout, 0, 1, m_DescriptorSet->descriptorSet.data(), 0, NULL);
//        vkCmdBindPipeline(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

//        for (int j = 0; j < modelSize; j++)
//        {
//            Rectangl* model = (static_cast<Rectangl*>(m_ModelList.at(j)));
//            if (!model || !model->GetVisible()) continue;

//            //////////////////////////////////////////////////////////////////////////////////
//            struct pushConst
//            {
//                glm::vec4 inColor;
//                glm::mat4 modelMatrix;
//            }PC;


//            PC.inColor = model->GetColor();
//            //PC.inColor.a = 0.5;
//            //PC.modelMatrix = /*(*model->GetScene()->GetProjection()) * (*model->GetScene()->GetView()) */ model->GetAbsoluteTransformation();
//            //PC.modelMatrix = (*GetProjection()) * (*GetView()) model->GetModelTransformation();// GetAbsoluteTransformation();

//            // Check if number of push constants does not exceed the allowed size
//            int maxPushContantSize = m_VulkanApplication->m_physicalDeviceInfo.prop.limits.maxPushConstantsSize;
//            if (sizeof(PC) > maxPushContantSize)
//            {
//                printf("Push constand size is greater than expected max allow size is %d", maxPushContantSize);
//                assert(0);
//            }

//            vkCmdPushConstants(p_CmdBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PC), &PC);
//            ////////////////////////////////////////////////////////////////////////////////

//            int vertexCount = 0;
//            if (model->GetDrawType() == Rectangl::FILLED)
//            {
//                vertexCount = sizeof(rectFilledVertices) / sizeof(Vertex);
//            }
//            else if (model->GetDrawType() == Rectangl::OUTLINE)
//            {
//                vertexCount = sizeof(rectOutlineVertices) / sizeof(Vertex);
//            }

//            const VkDeviceSize offsets[1] = { 0 };
//            vkCmdBindVertexBuffers(p_CmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &model->m_VertexBuffer.m_Buffer, offsets);
//            vkCmdDraw(p_CmdBuffer, vertexCount, /*INSTANCE_COUNT*/1, 0, 0);

//            model->SetDirtyType(DIRTY_TYPE::NONE);
//        }

//    }
//}

///////////////////////////////////////////////////////////////////////////////////////////

//void RectangleDescriptorSet::CreateUniformBuffer()
//{
//    UniformBuffer->m_BufObj.m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//    UniformBuffer->m_BufObj.m_DataSize = sizeof(glm::mat4);

//    // Create buffer resource states using VkBufferCreateInfo
//    VulkanHelper::CreateBuffer(m_VulkanApplication->m_hDevice, m_VulkanApplication->m_physicalDeviceInfo.memProp, UniformBuffer->m_BufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

//    // Map the GPU memory on to local host
//    VulkanHelper::MapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, 0, UniformBuffer->m_BufObj.m_MemRqrmnt.size, 0, UniformBuffer->m_MappedMemory);

//    // We have only one Uniform buffer object to update
//    UniformBuffer->m_MappedRange.resize(1);

//    // Populate the VkMappedMemoryRange data structure
//    UniformBuffer->m_MappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
//    UniformBuffer->m_MappedRange[0].memory = UniformBuffer->m_BufObj.m_Memory;
//    UniformBuffer->m_MappedRange[0].offset = 0;
//    UniformBuffer->m_MappedRange[0].size = UniformBuffer->m_BufObj.m_DataSize;

//    // Update descriptor buffer info in order to write the descriptors
//    UniformBuffer->m_DescriptorBufInfo.buffer = UniformBuffer->m_BufObj.m_Buffer;
//    UniformBuffer->m_DescriptorBufInfo.offset = 0;
//    UniformBuffer->m_DescriptorBufInfo.range = UniformBuffer->m_BufObj.m_DataSize;
//}

//void RectangleDescriptorSet::DestroyUniformBuffer()
//{
//    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
//    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
//    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
//}

//void RectangleDescriptorSet::CreateDescriptorSetLayout()
//{
//    // Define the layout binding information for the descriptor set(before creating it)
//    // Specify binding point, shader type(like vertex shader below), count etc.
//    VkDescriptorSetLayoutBinding layoutBindings[1];
//    layoutBindings[0].binding = 0; // DESCRIPTOR_SET_BINDING_INDEX
//    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    layoutBindings[0].descriptorCount = 1;
//    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    layoutBindings[0].pImmutableSamplers = NULL;

//    // Specify the layout bind into the VkDescriptorSetLayoutCreateInfo
//    // and use it to create a descriptor set layout
//    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
//    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    descriptorLayout.pNext = NULL;
//    descriptorLayout.bindingCount = 1;
//    descriptorLayout.pBindings = layoutBindings;

//    VkResult  result;
//    // Allocate required number of descriptor layout objects and
//    // create them using vkCreateDescriptorSetLayout()
//    descLayout.resize(1);
//    result = vkCreateDescriptorSetLayout(m_VulkanApplication->m_hDevice, &descriptorLayout, NULL, descLayout.data());
//    assert(result == VK_SUCCESS);
//}

//void RectangleDescriptorSet::DestroyDescriptorLayout()
//{
//    for (int i = 0; i < descLayout.size(); i++) {
//        vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
//    }
//    descLayout.clear();
//}

//void RectangleDescriptorSet::CreateDescriptor()
//{
//    CreateDescriptorSetLayout();
//    CreateUniformBuffer();
//    CreateDescriptorPool();
//    CreateDescriptorSet();
//}

//void RectangleDescriptorSet::CreateDescriptorPool()
//{
//    VkResult  result;
//    // Define the size of descriptor pool based on the
//    // type of descriptor set being used.
//    std::vector<VkDescriptorPoolSize> descriptorTypePool;

//    // The first descriptor pool object is of type Uniform buffer
//    descriptorTypePool.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 });

//    // Populate the descriptor pool state information
//    // in the create info structure.
//    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
//    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//    descriptorPoolCreateInfo.pNext = NULL;
//    descriptorPoolCreateInfo.maxSets = 1;
//    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//    descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorTypePool.size();
//    descriptorPoolCreateInfo.pPoolSizes = descriptorTypePool.data();

//    // Create the descriptor pool using the descriptor
//    // pool create info structure
//    result = vkCreateDescriptorPool(m_VulkanApplication->m_hDevice, &descriptorPoolCreateInfo, NULL, &descriptorPool);
//    assert(result == VK_SUCCESS);
//}

//void RectangleDescriptorSet::CreateDescriptorSet()
//{
//    VkResult  result;

//    // Create the descriptor allocation structure and specify the descriptor
//    // pool and descriptor layout
//    VkDescriptorSetAllocateInfo dsAllocInfo[1];
//    dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    dsAllocInfo[0].pNext = NULL;
//    dsAllocInfo[0].descriptorPool = descriptorPool;
//    dsAllocInfo[0].descriptorSetCount = 1;
//    dsAllocInfo[0].pSetLayouts = descLayout.data();

//    // Allocate the number of descriptor sets needs to be produced
//    descriptorSet.resize(1);

//    // Allocate descriptor sets
//    result = vkAllocateDescriptorSets(m_VulkanApplication->m_hDevice, dsAllocInfo, descriptorSet.data());
//    assert(result == VK_SUCCESS);

//    // Allocate one write descriptors for transformation (MVP)
//    VkWriteDescriptorSet writes[1];
//    memset(&writes, 0, sizeof(writes));

//    // Specify the uniform buffer related
//    // information into first write descriptor
//    writes[0] = {};
//    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    writes[0].pNext = NULL;
//    writes[0].dstSet = descriptorSet[0];
//    writes[0].descriptorCount = 1;
//    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    writes[0].pBufferInfo = &UniformBuffer->m_DescriptorBufInfo;
//    writes[0].dstArrayElement = 0;
//    writes[0].dstBinding = 0; // DESCRIPTOR_SET_BINDING_INDEX

//    // Update the uniform buffer into the allocated descriptor set
//    vkUpdateDescriptorSets(m_VulkanApplication->m_hDevice, 1, writes, 0, NULL);
//}

////////////////////////////////////////////////////////////////////////
//// REview this design:
//std::shared_ptr<RectangleDescriptorSet> CDS = NULL;// std::make_shared<CubeDescriptorSet>(m_VulkanApplication);
//RectangleDescriptorSet::UniformBufferObj* UniformBuffer = NULL;

//// Move the descriptor in seperate class

//RectangleInstancingScheme::RectangleInstancingScheme(VulkanApp* p_VulkanApp)
//{
//    assert(p_VulkanApp);
//    m_VulkanApplication = p_VulkanApp;

//    memset(&UniformBuffer, 0, sizeof(UniformBuffer));
//    memset(&m_VertexBuffer, 0, sizeof(VulkanBuffer) * PIPELINE_COUNT);
//    memset(&m_InstanceBuffer, 0, sizeof(VulkanBuffer) * PIPELINE_COUNT);
//    memset(&m_OldInstanceDataSize, 0, sizeof(int) * PIPELINE_COUNT);
//}

//RectangleInstancingScheme::~RectangleInstancingScheme()
//{
//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
//    if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
//    {
//        graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
//        vkDestroyPipeline(m_VulkanApplication->m_hDevice, graphicsPipeline, nullptr);

//        //// Destroy descriptors
//        //for (int i = 0; i < descLayout.size(); i++) {
//        //       vkDestroyDescriptorSetLayout(m_VulkanApplication->m_hDevice, descLayout[i], NULL);
//        //}
//        //descLayout.clear();
//        graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
//        vkDestroyPipelineLayout(m_VulkanApplication->m_hDevice, graphicsPipelineLayout, nullptr);
//    }

//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        // Destroy Vertex Buffer
//        vkDestroyBuffer(m_VulkanApplication->m_hDevice, m_VertexBuffer[pipelineIdx].m_Buffer, NULL);
//        vkFreeMemory(m_VulkanApplication->m_hDevice, m_VertexBuffer[pipelineIdx].m_Memory, NULL);
//    }

//    //vkFreeDescriptorSets(m_VulkanApplication->m_hDevice, descriptorPool, (uint32_t)descriptorSet.size(), &descriptorSet[0]);
//    //vkDestroyDescriptorPool(m_VulkanApplication->m_hDevice, descriptorPool, NULL);

//    vkUnmapMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory);
//    vkDestroyBuffer(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Buffer, NULL);
//    vkFreeMemory(m_VulkanApplication->m_hDevice, UniformBuffer->m_BufObj.m_Memory, NULL);
//}

//void RectangleInstancingScheme::Setup()
//{
//    if (!CDS)
//    {
//        CDS = std::make_shared<RectangleDescriptorSet>(m_VulkanApplication);
//        //CDS->CreateDescriptor();
//        UniformBuffer = CDS->UniformBuffer;
//    }

//    CreateVertexBuffer();

//    CreateGraphicsPipeline();

//    m_VulkanApplication->CreateCommandBuffers(); // Create command buffers

//    PrepareInstanceData(); // Todo check how to remove this from Setup

//    RecordCommandBuffer();
//}

//void RectangleInstancingScheme::Update()
//{
//    VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
//        UniformBuffer->m_MappedMemory,
//        UniformBuffer->m_MappedRange,
//        UniformBuffer->m_BufObj.m_MemoryFlags,
////        &m_Transform, sizeof(m_Transform));
//    &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));

//    PrepareInstanceData();
//}

//void RectangleInstancingScheme::UpdateDirty()
//{
//    VulkanHelper::WriteMemory(m_VulkanApplication->m_hDevice,
//        UniformBuffer->m_MappedMemory,
//        UniformBuffer->m_MappedRange,
//        UniformBuffer->m_BufObj.m_MemoryFlags,
////        &m_Transform, sizeof(m_Transform));
//        &m_ProjectViewMatrix, sizeof(m_ProjectViewMatrix));

//    UpdateDirtyInstanceData();
//}

//void RectangleInstancingScheme::ResizeWindow(int width, int height)
//{
//    CreateGraphicsPipeline(true);

//    m_VulkanApplication->CreateCommandBuffers(); // Create command buffers

//    RecordCommandBuffer();
//}

//void RectangleInstancingScheme::CreateRectOutlinePipeline()
//{
//    // Compile the vertex shader
//#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/CubeVert.spv"); // Relative path to binary output dir
//                                                                                                                              // Setup the vertex shader stage create info structures
//#elif __APPLE__
//    VkShaderModule vertShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/CubeVert.spv");
//#endif

//    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShader;
//    vertShaderStageInfo.pName = "main";

//    // Compile the fragment shader
//#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/CubeFrag.spv"); // Relative path to binary output dir
//                                                                                                                              // Setup the fragment shader stage create info structures
//#elif __APPLE__
//    VkShaderModule fragShader = VulkanHelper::CreateShaderFromQRCResource(m_VulkanApplication->m_hDevice, "://source/shaders/CubeFrag.spv");
//#endif
//    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShader;
//    fragShaderStageInfo.pName = "main";

//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//    // Setup the vertex input
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    // Check the size where every necessrry
//    vertexInputInfo.vertexBindingDescriptionCount = m_VertexInputBinding[PIPELINE_OUTLINE].size();
//    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_OUTLINE].data();
//    vertexInputInfo.vertexAttributeDescriptionCount = m_VertexInputAttribute[PIPELINE_OUTLINE].size();
//    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_OUTLINE].data();

//    // Setup input assembly
//    // We will be rendering 1 triangle using triangle strip topology
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;

//    // Setup viewport to the maximum widht and height of the window
//    VkViewport viewport = {};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
//    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;

//    // Setup scissor rect
//    VkRect2D scissor = {};
//    scissor.offset = { 0, 0 };
//    scissor.extent = m_VulkanApplication->m_swapChainExtent;

//    // Setup view port state
//    VkPipelineViewportStateCreateInfo viewportState = {};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;

//    // Setup the rasterizer state
//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//    rasterizer.depthClampEnable = true;

//    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
//    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencilStateInfo.pNext = NULL;
//    depthStencilStateInfo.flags = 0;
//    depthStencilStateInfo.depthTestEnable = true;
//    depthStencilStateInfo.depthWriteEnable = true;
//    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
//    depthStencilStateInfo.back.compareMask = 0;
//    depthStencilStateInfo.back.reference = 0;
//    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.writeMask = 0;
//    depthStencilStateInfo.minDepthBounds = 0;
//    depthStencilStateInfo.maxDepthBounds = 0;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.front = depthStencilStateInfo.back;

//    // Setup multi sampling. In our first example we will be using single sampling mode
//    VkPipelineMultisampleStateCreateInfo multisampling = {};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = NUM_SAMPLES;

//    // Setup color output masks.
//    // Set to write out RGBA components
//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_TRUE;

//    // Setup color blending
//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_TRUE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;

//    // Create pipeline layout
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 0;
//    pipelineLayoutInfo.pushConstantRangeCount = 0;
//    pipelineLayoutInfo.setLayoutCount = (uint32_t)CDS->descLayout.size();
//    pipelineLayoutInfo.pSetLayouts = CDS->descLayout.data();

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
//        assert(false);
//    }

//    // Create graphics pipeline
//    VkGraphicsPipelineCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = graphicsPipelineLayout;
//    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

//    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
//    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
//        assert(false);
//    }

//    m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

//    // Cleanup
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
//}

//void RectangleInstancingScheme::CreateRectFillPipeline()
//{
//    // Compile the vertex shader
//#ifdef _WIN32
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/CubeVert.spv"); // Relative path to binary output dir
//#elif __APPLE__
//    VkShaderModule vertShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice,
//    "/Users/parminder/Dev/Giraphics/VulkanByExample/source/Chapter6/TempEx7_PerformanceImprovmentAudioMixer/source/shaders/CubeVert.spv"); // Relative path to binary output dir
//#endif

//    // Setup the vertex shader stage create info structures
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    vertShaderStageInfo.module = vertShader;
//    vertShaderStageInfo.pName = "main";

//    // Compile the fragment shader
//#ifdef _WIN32
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice, "../source/shaders/CubeFrag.spv"); // Relative path to binary output dir
//#elif __APPLE__
//    VkShaderModule fragShader = VulkanHelper::CreateShader(m_VulkanApplication->m_hDevice,
//    "/Users/parminder/Dev/Giraphics/VulkanByExample/source/Chapter6/TempEx7_PerformanceImprovmentAudioMixer/source/shaders/CubeFrag.spv"); // Relative path to binary output dir
//#endif
//    // Setup the fragment shader stage create info structures
//    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShader;
//    fragShaderStageInfo.pName = "main";

//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//    // Setup the vertex input
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vertexInputInfo.vertexBindingDescriptionCount = m_VertexInputBinding[PIPELINE_FILLED].size();// sizeof(m_VertexInputBinding[PIPELINE_FILLED]) / sizeof(VkVertexInputBindingDescription);
//    vertexInputInfo.pVertexBindingDescriptions = m_VertexInputBinding[PIPELINE_FILLED].data();
//    vertexInputInfo.vertexAttributeDescriptionCount = m_VertexInputAttribute[PIPELINE_FILLED].size();// sizeof(m_VertexInputAttribute[PIPELINE_FILLED]) / sizeof(VkVertexInputAttributeDescription);
//    vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttribute[PIPELINE_FILLED].data();

//    // Setup input assembly
//    // We will be rendering 1 triangle using triangle strip topology
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;

//    // Setup viewport to the maximum widht and height of the window
//    VkViewport viewport = {};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)m_VulkanApplication->m_swapChainExtent.width;
//    viewport.height = (float)m_VulkanApplication->m_swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;

//    // Setup scissor rect
//    VkRect2D scissor = {};
//    scissor.offset = { 0, 0 };
//    scissor.extent = m_VulkanApplication->m_swapChainExtent;

//    // Setup view port state
//    VkPipelineViewportStateCreateInfo viewportState = {};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;

//    // Setup the rasterizer state
//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizer.depthClampEnable = VK_FALSE;
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    rasterizer.cullMode = VK_CULL_MODE_NONE/*VK_CULL_MODE_BACK_BIT*/;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//    rasterizer.depthBiasEnable = VK_FALSE;
//    rasterizer.depthClampEnable = true;

//    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
//    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencilStateInfo.pNext = NULL;
//    depthStencilStateInfo.flags = 0;
//    depthStencilStateInfo.depthTestEnable = true;
//    depthStencilStateInfo.depthWriteEnable = true;
//    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
//    depthStencilStateInfo.back.compareMask = 0;
//    depthStencilStateInfo.back.reference = 0;
//    depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
//    depthStencilStateInfo.back.writeMask = 0;
//    depthStencilStateInfo.minDepthBounds = 0;
//    depthStencilStateInfo.maxDepthBounds = 0;
//    depthStencilStateInfo.stencilTestEnable = VK_FALSE;
//    depthStencilStateInfo.front = depthStencilStateInfo.back;

//    // Setup multi sampling. In our first example we will be using single sampling mode
//    VkPipelineMultisampleStateCreateInfo multisampling = {};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = NUM_SAMPLES;

//    // Setup color output masks.
//    // Set to write out RGBA components
//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_TRUE;

//    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

//    // Setup color blending
//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY;
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f;
//    colorBlending.blendConstants[1] = 0.0f;
//    colorBlending.blendConstants[2] = 0.0f;
//    colorBlending.blendConstants[3] = 0.0f;

//    // Create pipeline layout
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 0;
//    pipelineLayoutInfo.pushConstantRangeCount = 0;
//    pipelineLayoutInfo.setLayoutCount = (uint32_t)CDS->descLayout.size();
//    pipelineLayoutInfo.pSetLayouts = CDS->descLayout.data();

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkResult vkResult = vkCreatePipelineLayout(m_VulkanApplication->m_hDevice, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreatePipelineLayout() failed!");
//        assert(false);
//    }

//    // Create graphics pipeline
//    VkGraphicsPipelineCreateInfo pipelineInfo = {};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.layout = graphicsPipelineLayout;
//    pipelineInfo.renderPass = m_VulkanApplication->m_hRenderPass;
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
//    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;

//    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
//    vkResult = vkCreateGraphicsPipelines(m_VulkanApplication->m_hDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
//    if (vkResult != VK_SUCCESS)
//    {
//        VulkanHelper::LogError("vkCreateGraphicsPipelines() failed!");
//        assert(false);
//    }

//    m_GraphicsPipelineMap[PIPELINE_RECT_FILLED] = qMakePair(graphicsPipeline, graphicsPipelineLayout);

//    // Cleanup
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, fragShader, nullptr);
//    vkDestroyShaderModule(m_VulkanApplication->m_hDevice, vertShader, nullptr);
//}

//void RectangleInstancingScheme::CreateGraphicsPipeline(bool p_ClearGraphicsPipelineMap)
//{
//    if (p_ClearGraphicsPipelineMap)
//    {
//        const VkDevice& device = m_VulkanApplication->m_hDevice;
//        QMap<QString, QPair<VkPipeline, VkPipelineLayout> >::iterator i;
//        for (i = m_GraphicsPipelineMap.begin(); i != m_GraphicsPipelineMap.end(); ++i)
//        {
//            vkDestroyPipeline(m_VulkanApplication->m_hDevice, i.value().first, nullptr);
//            vkDestroyPipelineLayout(device, i.value().second, nullptr);
//        }

//        m_GraphicsPipelineMap.clear();
//    }

//    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
//    if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
//    {
//        graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
//        graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
//        return;
//    }

//    CreateRectFillPipeline();
//    CreateRectOutlinePipeline();
//}

//void RectangleInstancingScheme::RecordCommandBuffer()
//{
//    // Specify the clear color value
//    VkClearValue clearColor[2];
//    clearColor[0].color.float32[0] = 0.0f;
//    clearColor[0].color.float32[1] = 0.0f;
//    clearColor[0].color.float32[2] = 0.0f;
//    clearColor[0].color.float32[3] = 0.0f;

//    // Specify the depth/stencil clear value
//    clearColor[1].depthStencil.depth = 1.0f;
//    clearColor[1].depthStencil.stencil = 0;

//    // Offset to render in the frame buffer
//    VkOffset2D   renderOffset = { 0, 0 };
//    // Width & Height to render in the frame buffer
//    VkExtent2D   renderExtent = m_VulkanApplication->m_swapChainExtent;

//    // For each command buffers in the command buffer list
//    for (size_t i = 0; i < m_VulkanApplication->m_hCommandBufferList.size(); i++)
//    {
//        VkCommandBufferBeginInfo beginInfo = {};
//        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        // Indicate that the command buffer can be resubmitted to the queue
//        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

//        // Begin command buffer
//        vkBeginCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i], &beginInfo);

//        VkRenderPassBeginInfo renderPassInfo = {};
//        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//        renderPassInfo.renderPass = m_VulkanApplication->m_hRenderPass;
//        renderPassInfo.framebuffer = m_VulkanApplication->m_hFramebuffers[i];
//        renderPassInfo.renderArea.offset = renderOffset;
//        renderPassInfo.renderArea.extent = renderExtent;
//        renderPassInfo.clearValueCount = 2;
//        renderPassInfo.pClearValues = clearColor;

//        // Begin render pass
//        vkCmdBeginRenderPass(m_VulkanApplication->m_hCommandBufferList[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//        Render(m_VulkanApplication->m_hCommandBufferList[i]); // consider using shared ptr/smart pointers

//        // End the Render pass
//        vkCmdEndRenderPass(m_VulkanApplication->m_hCommandBufferList[i]);

//        // End the Command buffer
//        VkResult vkResult = vkEndCommandBuffer(m_VulkanApplication->m_hCommandBufferList[i]);
//        if (vkResult != VK_SUCCESS)
//        {
//            VulkanHelper::LogError("vkEndCommandBuffer() failed!");
//            assert(false);
//        }
//    }
//}

//void RectangleInstancingScheme::CreateVertexBuffer()
//{
//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        m_VertexBuffer[pipelineIdx].m_MemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

//        const VkPhysicalDeviceMemoryProperties& memProp = m_VulkanApplication->m_physicalDeviceInfo.memProp;
//        const VkDevice& device = m_VulkanApplication->m_hDevice;
//        if (pipelineIdx == PIPELINE_FILLED)
//        {
//            m_VertexBuffer[pipelineIdx].m_DataSize = sizeof(rectFilledVertices);
//            m_VertexCount[PIPELINE_FILLED] = sizeof(rectFilledVertices) / sizeof(Vertex);

//            uint32_t dataStride = sizeof(rectFilledVertices[0]);
//            VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer[pipelineIdx], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectFilledVertices);

//            m_VertexInputBinding[pipelineIdx].resize(2);   // 0 for position and 1 for color
//            m_VertexInputAttribute[pipelineIdx].resize(9); // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

//            // Indicates the rate at which the information will be
//            // injected for vertex input.
//            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//            m_VertexInputBinding[pipelineIdx][0].stride = dataStride;

//            m_VertexInputBinding[pipelineIdx][1].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
//            m_VertexInputBinding[pipelineIdx][1].stride = sizeof(InstanceData);

//            // The VkVertexInputAttribute interpreting the data.
//            m_VertexInputAttribute[pipelineIdx][0].binding = 0;
//            m_VertexInputAttribute[pipelineIdx][0].location = 0;
//            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

//            m_VertexInputAttribute[pipelineIdx][1].binding = 0;
//            m_VertexInputAttribute[pipelineIdx][1].location = 1;
//            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
//            ////////////////////////////////////////////////////////////////////////////////////

//            // Model Matrix
//            m_VertexInputAttribute[pipelineIdx][2].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][2].location = 2;
//            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][2].offset = 0;

//            m_VertexInputAttribute[pipelineIdx][3].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][3].location = 3;
//            m_VertexInputAttribute[pipelineIdx][3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][3].offset = 16 * 1;

//            m_VertexInputAttribute[pipelineIdx][4].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][4].location = 4;
//            m_VertexInputAttribute[pipelineIdx][4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][4].offset = 16 * 2;

//            m_VertexInputAttribute[pipelineIdx][5].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][5].location = 5;
//            m_VertexInputAttribute[pipelineIdx][5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][5].offset = 16 * 3;

//            // Dimension
//            m_VertexInputAttribute[pipelineIdx][6].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][6].location = 6;
//            m_VertexInputAttribute[pipelineIdx][6].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][6].offset = 16 * 4;

//            // Color
//            m_VertexInputAttribute[pipelineIdx][7].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][7].location = 7;
//            m_VertexInputAttribute[pipelineIdx][7].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][7].offset = 16 * 5;

//            // BoolFlags
//            m_VertexInputAttribute[pipelineIdx][8].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][8].location = 8;
//            m_VertexInputAttribute[pipelineIdx][8].format = VK_FORMAT_R32_UINT;
//            m_VertexInputAttribute[pipelineIdx][8].offset = /*16 * 5*/offsetof(struct InstanceData, m_BoolFlags);

//        }
//        else if (pipelineIdx == PIPELINE_OUTLINE)
//        {
//            m_VertexBuffer[pipelineIdx].m_DataSize = sizeof(rectOutlineVertices);
//            m_VertexCount[PIPELINE_OUTLINE] = sizeof(rectOutlineVertices) / sizeof(Vertex);
//            const uint32_t dataStride = sizeof(rectOutlineVertices[0]);
//            VulkanHelper::CreateBuffer(device, memProp, m_VertexBuffer[pipelineIdx], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, rectOutlineVertices);

//            m_VertexInputBinding[pipelineIdx].resize(2);   // 0 for position and 1 for color
//            m_VertexInputAttribute[pipelineIdx].resize(9); // Why 7 = 2(for position and color) + 5 (transform and rotation) + Color

//            // Indicates the rate at which the information will be
//            // injected for vertex input.
//            m_VertexInputBinding[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//            m_VertexInputBinding[pipelineIdx][0].stride = dataStride;

//            m_VertexInputBinding[pipelineIdx][1].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputBinding[pipelineIdx][1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
//            m_VertexInputBinding[pipelineIdx][1].stride = sizeof(InstanceData);

//            // The VkVertexInputAttribute interpreting the data.
//            m_VertexInputAttribute[pipelineIdx][0].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][0].location = 0;
//            m_VertexInputAttribute[pipelineIdx][0].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][0].offset = offsetof(struct Vertex, m_Position);

//            m_VertexInputAttribute[pipelineIdx][1].binding = VERTEX_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][1].location = 1;
//            m_VertexInputAttribute[pipelineIdx][1].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][1].offset = offsetof(struct Vertex, m_Color);
//            ////////////////////////////////////////////////////////////////////////////////////

//            // Model Matrix
//            m_VertexInputAttribute[pipelineIdx][2].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][2].location = 2;
//            m_VertexInputAttribute[pipelineIdx][2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][2].offset = 0;

//            m_VertexInputAttribute[pipelineIdx][3].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][3].location = 3;
//            m_VertexInputAttribute[pipelineIdx][3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][3].offset = 16 * 1;

//            m_VertexInputAttribute[pipelineIdx][4].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][4].location = 4;
//            m_VertexInputAttribute[pipelineIdx][4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][4].offset = 16 * 2;

//            m_VertexInputAttribute[pipelineIdx][5].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][5].location = 5;
//            m_VertexInputAttribute[pipelineIdx][5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][5].offset = 16 * 3;

//            // Dimension
//            m_VertexInputAttribute[pipelineIdx][6].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][6].location = 6;
//            m_VertexInputAttribute[pipelineIdx][6].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][6].offset = /*16 * 4*/offsetof(struct InstanceData, m_Rect);

//            // Color
//            m_VertexInputAttribute[pipelineIdx][7].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][7].location = 7;
//            m_VertexInputAttribute[pipelineIdx][7].format = VK_FORMAT_R32G32B32_SFLOAT;
//            m_VertexInputAttribute[pipelineIdx][7].offset = /*16 * 5*/offsetof(struct InstanceData, m_Color);

//            // BoolFlags
//            m_VertexInputAttribute[pipelineIdx][8].binding = INSTANCE_BUFFER_BIND_IDX;
//            m_VertexInputAttribute[pipelineIdx][8].location = 8;
//            m_VertexInputAttribute[pipelineIdx][8].format = VK_FORMAT_R32_UINT;
//            m_VertexInputAttribute[pipelineIdx][8].offset = /*16 * 5*/offsetof(struct InstanceData, m_BoolFlags);
//        }
//    }
//}

//void RectangleInstancingScheme::UpdateNodeList(Node* p_Item)
//{
//    //m_ModelList.push_back(p_Item);

////    RectangleModel* rectangle = dynamic_cast<RectangleModel*>(p_Item);
//    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Item);

//    assert(rectangle);

//    // Note: Based on the draw type push the model in respective pipelines
//    // Keep the draw type loose couple with the pipeline type,
//    // they may be in one-to-one correspondence but that is not necessary.
//    switch (rectangle->GetDrawType())
//    {
//    case Rectangl::FILLED:
//        m_PipelineTypeModelVector[PIPELINE_FILLED].push_back(p_Item);
//        break;

//    case Rectangl::OUTLINE:
//        m_PipelineTypeModelVector[PIPELINE_OUTLINE].push_back(p_Item);
//        break;

//    case Rectangl::ROUNDED:
//        // TODO
//        break;

//    default:
//        break;
//    }
//}

//void RectangleInstancingScheme::RemoveNodeList(Node* p_Model)
//{
//    Rectangl* rectangle = dynamic_cast<Rectangl*>(p_Model);
//    assert(rectangle);

//    std::vector<Node*>* modelVector = &m_PipelineTypeModelVector[rectangle->GetDrawType()];
//    if (!modelVector) return;

//    bool isUpdated = false;
//    while (1)
//    {
//        auto result = std::find(std::begin(*modelVector), std::end(*modelVector), rectangle);
//        if (result == std::end(*modelVector)) break;

//        modelVector->erase(result);
//        isUpdated = true;
//    }

//    if (isUpdated)
//    {
//        RECTANGLE_GRAPHICS_PIPELINES pipeline = PIPELINE_COUNT;

//        switch (rectangle->GetDrawType())
//        {
//        case Rectangl::FILLED:
//            pipeline = PIPELINE_FILLED;
//            break;

//        case Rectangl::OUTLINE:
//            pipeline = PIPELINE_OUTLINE;
//            break;

//        case Rectangl::ROUNDED:
//            // TODO
//            break;

//        default:
//            break;
//        }

//        if (pipeline == PIPELINE_COUNT) return;

//        PrepareInstanceData(pipeline);
//    }
//}

//void RectangleInstancingScheme::PrepareInstanceData(RECTANGLE_GRAPHICS_PIPELINES p_Pipeline /*=PIPELINE_COUNT*/)
//{
//    bool update = false;
//    bool isSinglePipelinePrepareRequest = (p_Pipeline != PIPELINE_COUNT);
//    for (int pipelineIdx = (isSinglePipelinePrepareRequest ? p_Pipeline : 0); (pipelineIdx < PIPELINE_COUNT); (isSinglePipelinePrepareRequest ? (p_Pipeline = PIPELINE_COUNT/*break with single iteration*/) : pipelineIdx++))
//    {
//        ModelVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
//        const int modelSize = m_ModelList.size();
//        if (!modelSize) continue;

//        //int oldInstanceDataSize = m_InstanceData.size();
//        std::vector<InstanceData> instanceData;
//        //m_InstanceData.clear();
//        instanceData.resize(modelSize);

//        for (int i = 0; i < modelSize; i++)
//        {
//            instanceData[i].m_Model = m_ModelList.at(i)->GetModelTransformation();
//            instanceData[i].m_Rect.x = m_ModelList.at(i)->GetBoundedRegion().m_Dimension.x;
//            instanceData[i].m_Rect.y = m_ModelList.at(i)->GetBoundedRegion().m_Dimension.y;
//            instanceData[i].m_Color = m_ModelList.at(i)->GetColor();
//            instanceData[i].m_BoolFlags = m_ModelList.at(i)->GetVisible() ? 1 : 0;
//            m_ModelList.at(i)->SetGpuMemOffset(i * sizeof(InstanceData));
//        }

//        if (modelSize != 0)
//        {
//            VkMemoryPropertyFlags memoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//            m_InstanceBuffer[pipelineIdx].m_MemoryFlags = memoryProperty;
//            m_InstanceBuffer[pipelineIdx].m_DataSize = modelSize * sizeof(InstanceData);
//            // Re-Create instance buffer if size not same.

//            VulkanHelper::CreateStagingBuffer(m_VulkanApplication->m_hDevice,
//                m_VulkanApplication->m_physicalDeviceInfo.memProp,
//                m_VulkanApplication->m_hCommandPool,
//                m_VulkanApplication->m_hGraphicsQueue,
//                m_InstanceBuffer[pipelineIdx],
//                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
//                instanceData.data());

//            if (modelSize != m_OldInstanceDataSize[pipelineIdx] && modelSize != 0)
//            {
//                update = true;
//                //RecordCommandBuffer();
//            }
//        }

//        m_OldInstanceDataSize[pipelineIdx] = modelSize;
//    }

//    if (update)
//    {
//        RecordCommandBuffer();
//    }
//}

//void RectangleInstancingScheme::UpdateDirtyInstanceData()
//{
//    bool update = false;
//    for (int pipelineIdx = 0; pipelineIdx < PIPELINE_COUNT; pipelineIdx++)
//    {
//        ModelVector& modelList = m_PipelineTypeModelVector[pipelineIdx];
//        const int modelSize = static_cast<int>(modelList.size());
//        if (!modelSize) continue;

//        std::vector<InstanceData> instanceData;
//        std::vector<unsigned int> destOffset;
//        std::vector<VkBufferCopy> copyRegion;
//        const uint64_t instanceDataSize = static_cast<uint64_t>(sizeof(InstanceData));
//        for (int i = 0; i < modelSize; i++)
//        {
//            if (modelList.at(i)->GetDirtyType() == DIRTY_TYPE::ATTRIBUTES)
//            {
//                InstanceData data;
//                data.m_Model = modelList.at(i)->GetModelTransformation();
//                data.m_Rect.x = modelList.at(i)->GetBoundedRegion().m_Dimension.x;
//                data.m_Rect.y = modelList.at(i)->GetBoundedRegion().m_Dimension.y;
//                data.m_Color = modelList.at(i)->GetColor();
//                data.m_BoolFlags = modelList.at(i)->GetVisible() ? 1 : 0;

//                instanceData.push_back(data);
//                destOffset.push_back(modelList.at(i)->GetGpuMemOffset());

//                VkBufferCopy copyRgnItem;
//                copyRgnItem.size = static_cast<VkDeviceSize>(instanceDataSize);
//                copyRgnItem.srcOffset = static_cast<VkDeviceSize>(copyRegion.size() * instanceDataSize);
//                copyRgnItem.dstOffset = static_cast<VkDeviceSize>(modelList.at(i)->GetGpuMemOffset());
//                copyRegion.push_back(copyRgnItem);

//                modelList.at(i)->SetDirtyType(DIRTY_TYPE::NONE);
//            }
//        }

//        const size_t dirtyItemSize = instanceData.size();
//        if (dirtyItemSize != 0)
//        {
//            VulkanHelper::CreateStagingBufferCopyRegion(m_VulkanApplication->m_hDevice,
//                m_VulkanApplication->m_physicalDeviceInfo.memProp,
//                m_VulkanApplication->m_hCommandPool,
//                m_VulkanApplication->m_hGraphicsQueue,
//                m_InstanceBuffer[pipelineIdx],
//                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
//                instanceData.data(),
//                dirtyItemSize * sizeof(InstanceData),
//                NULL,
//                copyRegion);
//        }

//        if (dirtyItemSize != 0)
//        {
//            update = true;
//        }

//        m_OldInstanceDataSize[pipelineIdx] = modelSize;
//    }

//    if (!update) return;

//    RecordCommandBuffer();
//}

//void RectangleInstancingScheme::Render(VkCommandBuffer& p_CmdBuffer)
//{
//    for (int pipelineIdx = 0; pipelineIdx < RECTANGLE_GRAPHICS_PIPELINES::PIPELINE_COUNT; pipelineIdx++)
//    {
//        ModelVector& m_ModelList = m_PipelineTypeModelVector[pipelineIdx];
//        if (!m_ModelList.size()) continue;

//        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
//        VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
//        if (pipelineIdx == PIPELINE_FILLED)
//        {
//            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_FILLED))
//            {
//                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].first;
//                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_FILLED].second;
//            }
//        }
//        else if (pipelineIdx == PIPELINE_OUTLINE)
//        {
//            if (m_GraphicsPipelineMap.contains(PIPELINE_RECT_OUTLINE))
//            {
//                graphicsPipeline = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].first;
//                graphicsPipelineLayout = m_GraphicsPipelineMap[PIPELINE_RECT_OUTLINE].second;
//            }
//        }
//        else
//        {
//            assert(false);
//        }

//        // Bind graphics pipeline
//        vkCmdBindPipeline(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//        vkCmdBindDescriptorSets(p_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout,
//            0, 1, CDS->descriptorSet.data(), 0, NULL);

//        // Specify vertex buffer information
//        const VkDeviceSize offsets[1] = { 0 };
//        vkCmdBindVertexBuffers(p_CmdBuffer, VERTEX_BUFFER_BIND_IDX, 1, &m_VertexBuffer[pipelineIdx].m_Buffer, offsets);
//        vkCmdBindVertexBuffers(p_CmdBuffer, INSTANCE_BUFFER_BIND_IDX, 1, &m_InstanceBuffer[pipelineIdx].m_Buffer, offsets);

//        // Draw the Rectangle
//        vkCmdDraw(p_CmdBuffer, m_VertexCount[pipelineIdx], m_ModelList.size(), 0, 0);
//    }
//}

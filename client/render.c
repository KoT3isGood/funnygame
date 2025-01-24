#include "render.h"
#include "vulkan/vulkan.h"
#include "stdio.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifdef __linux
#include "X11/Xlib.h"
#include "vulkan/vulkan_xlib.h"
#endif
#ifdef __WIN32__
#include "windows.h"
#include "vulkan/vulkan_win32.h"
#endif
#include "../common/common.h"
#include "stdlib.h"
#include "memory.h"
#include "window.h"
#include "../includes/vk_mem_alloc.h"
#include "../common/cvar.h"

#include "vk_mem_alloc.h"

#ifdef _SERVER



#else

VkInstance instance;

VkPhysicalDevice physicalDevice;
VkDevice device;
VkQueue draw;
VkQueue present;

VmaAllocator allocator;

VkCommandPool cmdPool;
VkCommandBuffer cmd[2];
VkCommandBuffer stagingCommandBuffer;
VkCommandPool stagingCommandPool;

VkFence fence[2];
extern VkSemaphore* graphicsSemaphore;
extern VkSemaphore* presentSemaphore;


int imageIndex;

cvar_t* fov;
cvar_t* barycentrics_mode;


vk_extensions extensions;

void draw_initmodels();
void draw_init() { 
  // console variables
  fov = cvar_fget("r_fov",90.0,0,0);
  barycentrics_mode = cvar_fget("r_barycentrics_mode",0,CVAR_READONLY,
      "0 = automatic selection based on GPU (default)\n"
      "1 = force barycentrics trough geometry shaders\n"
      "      might be slower\n"
      "2 = force barycentrics trough VK_KHR_fragment_shader_barycentrics\n"
      "      might not work on older GPUs\n"
      "other values will default to 0\n"
      );

  {
    // instance
    // 1.3 is required
    // might migrate soon to 1.4
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    const char* extensions[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef __linux
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#ifdef __WIN32__
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
    };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    VkResult r = vkCreateInstance(&createInfo,0,&instance);
    VK_PRINTRES("vkCreateInstance",r);
  }
  {

    // device
    uint32_t count = 0;
    VkResult r = vkEnumeratePhysicalDevices(instance,&count,0);
    VK_PRINTRES("vkEnumeratePhysicalDevices",r);
    if (!count) {
      fuck("failed to find any GPUs");
    }
    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*count);
    r = vkEnumeratePhysicalDevices(instance,&count,physicalDevices);
    VK_PRINTRES("vkEnumeratePhysicalDevices(2)",r);
    for (int i = 0;i<count;i++) {
      VkPhysicalDeviceProperties features = {};
      vkGetPhysicalDeviceProperties(physicalDevices[0], &features);
      physicalDevice=physicalDevices[i];
      break;
    };
    free(physicalDevices);

    uint32_t numavaliableextensions;
    vkEnumerateDeviceExtensionProperties(physicalDevice, 0,&numavaliableextensions,0);
    VkExtensionProperties* availableextensions=malloc(sizeof(VkExtensionProperties)*numavaliableextensions);
    vkEnumerateDeviceExtensionProperties(physicalDevice, 0,&numavaliableextensions,availableextensions);

    uint32_t numextensions=0;
    char** deviceextensions;


    #define REQUIRE(a) \
      numextensions++;

    #define OPTIONAL(a) \
    for(int i = 0;i<numavaliableextensions;i++) { \
      if (!strcmp(#a,availableextensions[i].extensionName)) { \
        numextensions++; \
        break; \
      } \
    }

    #include "render/extensions.txt"

    #undef REQUIRE
    #undef OPTIONAL

    deviceextensions = malloc(sizeof(char*)*numextensions);

    int x = 0;

    #define REQUIRE(a) \
      extensions._##a=1; \
      deviceextensions[x]=#a; \
      x++;

    #define OPTIONAL(a) \
    extensions._##a=0; \
    for(int i = 0;i<numavaliableextensions;i++) { \
      if (!strcmp(#a,availableextensions[i].extensionName)) { \
        extensions._##a=1; \
        deviceextensions[x]=#a;\
        x++;\
      } \
    } \


    #include "render/extensions.txt"

    #undef REQUIRE
    #undef OPTIONAL



    for(int i = 0;i<numextensions;i++) {
      printf("enabling %s\n",deviceextensions[i]);
    }


    free(availableextensions);


    float priority = 1.0;
    VkDeviceQueueCreateInfo queueCreateInfo={};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority; 
    VkPhysicalDeviceVulkan12Features pdv12={};
    pdv12.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    pdv12.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR pdfsbf = {};
    pdfsbf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR;
    pdfsbf.fragmentShaderBarycentric = VK_TRUE;
    pdfsbf.pNext=&pdv12;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = numextensions;
    createInfo.ppEnabledExtensionNames = deviceextensions;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pNext = &pdfsbf;

    VkPhysicalDeviceFeatures features = {};
    features.geometryShader=VK_TRUE;
    createInfo.pEnabledFeatures=&features;
    r=vkCreateDevice(physicalDevice,&createInfo, 0, &device);
    VK_PRINTRES("vkCreateDevice",r);
    vkGetDeviceQueue(device,0,0,&draw);
    vkGetDeviceQueue(device,0,0,&present);

    //free(extensions);
  }
  {
    VmaVulkanFunctions functions = {};
    functions.vkGetInstanceProcAddr=vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr=vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo createInfo = {};
    createInfo.instance=instance;
    createInfo.physicalDevice=physicalDevice;
    createInfo.device=device;
    createInfo.pVulkanFunctions=&functions;
    VkResult r = vmaCreateAllocator(&createInfo,&allocator);
    printf("%i\n",r);
    VK_PRINTRES("vmaCreateAllocator",r);
  }
  {
    // command buffers
    VkCommandPoolCreateInfo cmdPoolCreateInfo={};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = 0;
    vkCreateCommandPool(device, &cmdPoolCreateInfo, 0, &cmdPool);

    VkCommandBufferAllocateInfo cmdAllocateInfo={};
    cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocateInfo.commandBufferCount = 2;
    cmdAllocateInfo.commandPool = cmdPool;
    vkAllocateCommandBuffers(device, &cmdAllocateInfo, cmd);

    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = 0;
    vkCreateCommandPool(device, &cmdPoolCreateInfo, 0, &stagingCommandPool);

    cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocateInfo.commandBufferCount = 1;
    cmdAllocateInfo.commandPool = stagingCommandPool;
    vkAllocateCommandBuffers(device, &cmdAllocateInfo, &stagingCommandBuffer);
  }
  {
    // sync
    VkFenceCreateInfo fenceCreateInfo={};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo={};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < 2;i++) {
      vkCreateFence(device, &fenceCreateInfo, 0, &fence[i]);
    }

  }
  printf("created drawer\n");

  draw_initmodels();
};

extern int windowcount;
extern VkSwapchainKHR* swapchains;
extern uint32_t* imageindexes;


void draw_sync() {

  // nothing to draw
  if (!windowcount) {
    return;
  }

  vkWaitForFences(device,1,&fence[imageIndex],VK_TRUE,UINT64_MAX);
  vkResetFences(device,1,&fence[imageIndex]);

  for (int i = 0; i<windowcount;i++) {
    VkResult r = vkAcquireNextImageKHR(device, swapchains[i], UINT64_MAX, presentSemaphore[i], 0, &imageindexes[i]);
  }
};

extern window mainwindow;
void draw_rendermodels();

void draw_flush() {
  if (!windowcount) {
    return;
  }

  

  // command buffer
  vkResetCommandBuffer(cmd[imageIndex],0);
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd[imageIndex],&beginInfo);
  VkImageSubresourceRange isr = {};
  isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  isr.layerCount=1;
  isr.levelCount=1;
  VkClearColorValue ccv = {
    1,0,0,0
  };
  vkCmdClearColorImage(cmd[imageIndex],sys_getwindowimage(mainwindow), VK_IMAGE_LAYOUT_GENERAL,&ccv,1,&isr);
  draw_rendermodels();
  vkEndCommandBuffer(cmd[imageIndex]);

  // flush
  VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = windowcount;
	submitInfo.pWaitSemaphores = presentSemaphore;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd[imageIndex];

	submitInfo.signalSemaphoreCount = windowcount;
	submitInfo.pSignalSemaphores = graphicsSemaphore;

	vkQueueSubmit(draw, 1, &submitInfo, fence[imageIndex]);

	VkResult* results = (VkResult*)malloc(sizeof(VkResult)*windowcount);
	VkPresentInfoKHR presentInfo={};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = windowcount;
	presentInfo.pWaitSemaphores = graphicsSemaphore;
	presentInfo.swapchainCount = windowcount;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = imageindexes;
	presentInfo.pResults = results;
	VkResult r = vkQueuePresentKHR(present, &presentInfo);
};
void draw_deinit() {
  vkDestroyDevice(device,0);
  vkDestroyInstance(instance,0);
};

uint32_t findmemorytype(uint32_t typeFilter, VkMemoryPropertyFlags properties);

vk_buffer vk_genbuffer(uint32_t size, VkBufferUsageFlags usage, VmaMemoryUsage memorytype) {
  vk_buffer buffer;
  buffer.size = size;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = memorytype;

  VkBufferCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.size = size;
  createInfo.usage = usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkResult r = vmaCreateBuffer(allocator,&createInfo,&allocInfo,&buffer.buffer,&buffer.memory,0);
  VK_PRINTRES("vmaCreateBuffer",r);

  VkBufferDeviceAddressInfo info = {};
  info.sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;  
  info.buffer=buffer.buffer;
  buffer.address=vkGetBufferDeviceAddress(device,&info);

  return buffer;
};
void vk_freebuffer(vk_buffer buffer) {
  vmaDestroyBuffer(allocator,buffer.buffer,buffer.memory);
};




VkImageView vk_genimageview(const VkImage image, VkFormat format) {	
	VkImageView imageView = 0;
	VkImageViewCreateInfo imageViewCreateInfo={};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	VkResult r= vkCreateImageView(device, &imageViewCreateInfo, 0, &imageView);
  VK_PRINTRES("vkCreateImageView",r);
	return imageView;
}

uint32_t findmemorytype(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return 0;
}



vk_shader vk_genshader(const char* shaderfile, VkShaderStageFlagBits shadertype, const char* entry) {
  FILE* f = fopen(shaderfile,"rb");
  if (!f) {
    fuck("shader not found");
  }
	uint32_t size = 0;
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* filedata = (char*)malloc(size+1);
  fread(filedata, sizeof(char), size, f);
  if (!size) {
    fuck(
        "invalid spirv\n"
        "size = 0"
        );
  }
  fclose(f);

  VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = (uint32_t*)filedata;
	
	VkShaderModule shadermodule;
	VkResult r=vkCreateShaderModule(device, &createInfo, 0, &shadermodule);
  if (r) {
    printf("failed to compile shader %s\n",shaderfile);
    printf("result %i",r);
    fuck("");
  }

	VkPipelineShaderStageCreateInfo shaderStageInfo={};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = shadertype;
	shaderStageInfo.module = shadermodule;
	shaderStageInfo.pName = "main";

  vk_shader shader = {};
  shader.module = shadermodule;
  shader.stageinfo=shaderStageInfo;
  shader.type=shadertype;

  free(filedata);

  return shader;
};



VkDescriptorSetLayout vk_gendescriptorset(uint32_t numbindings,VkDescriptorSetLayoutBinding* bindings) {
  VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo dslci={};
	dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslci.bindingCount = numbindings;
	dslci.pBindings = bindings;

	vkCreateDescriptorSetLayout(device, &dslci, 0, &layout);
};

vk_tripipeline vk_gentripipeline(vk_tripipeline_info info) {

  vk_tripipeline pipeline;

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo={};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts = info.descriptors;
	pipelineLayoutCreateInfo.setLayoutCount = info.desciptorsnum;

	VkPushConstantRange pushConstantRange={};
	pushConstantRange.size = info.pushsize;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	if (info.pushsize) {
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	}

	VkResult r=vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipeline.layout);
  VK_PRINTRES("vkCreatePipelineLayout",r);

	VkDynamicState states[2] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState={};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = states;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo={};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription vertexBindingDescriptions={};
	vertexBindingDescriptions.stride = 12;

	VkVertexInputAttributeDescription vertexAttributeDescriptions={};
	vertexAttributeDescriptions.format = VK_FORMAT_R32G32B32_SFLOAT;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions;
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly={};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport={};
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	viewport.width = 1280;
	viewport.height = 720;

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor={};
	scissor.offset.x=0;
	scissor.offset.y=0;

	VkPipelineViewportStateCreateInfo viewportState={};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer={};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = info.polygonmode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

  // TODO: MSAA
	VkPipelineMultisampleStateCreateInfo multisampling={};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = 0; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment={};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending={};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkAttachmentReference depthAttachmentRef={};
	depthAttachmentRef.attachment = info.renderpassnum;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  VkAttachmentDescription* attachments = malloc(sizeof(VkAttachmentDescription)*(info.renderpassnum+1));

	attachments[info.renderpassnum].format = VK_FORMAT_D32_SFLOAT;
	attachments[info.renderpassnum].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[info.renderpassnum].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[info.renderpassnum].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[info.renderpassnum].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[info.renderpassnum].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[info.renderpassnum].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[info.renderpassnum].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[info.renderpassnum].flags=0;


  VkAttachmentReference* colorAttachments = malloc(sizeof(VkAttachmentReference)*info.renderpassnum);

  for (int i = 0;i<info.renderpassnum;i++) {
    attachments[i].format = info.renderpass->format;
    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachments[i].flags=0;

    colorAttachments[i].attachment = i;
    colorAttachments[i].layout = VK_IMAGE_LAYOUT_GENERAL;

  }

	VkSubpassDescription subpass={};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = info.renderpassnum;
	subpass.pColorAttachments = colorAttachments;
	if (info.depth) {
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	VkRenderPassCreateInfo renderPassInfo={};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = info.depth?(1+info.renderpassnum):info.renderpassnum;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	r = vkCreateRenderPass(device, &renderPassInfo, 0, &pipeline.renderpass);
  VK_PRINTRES("vkCreateRenderPass",r);

	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.layout = pipeline.layout;
	createInfo.stageCount = info.numshaders;
  VkPipelineShaderStageCreateInfo* stages = malloc(sizeof(VkPipelineShaderStageCreateInfo)*info.numshaders);
  for (int i = 0;i<info.numshaders;i++) {
    stages[i]=info.shaders[i].stageinfo;
  }
	createInfo.pStages = stages;

	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pViewportState = &viewportState;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pColorBlendState = &colorBlending;
	createInfo.pDynamicState = &dynamicState;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = info.depth;
	depthStencil.depthWriteEnable = info.depth;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.stencilTestEnable = VK_FALSE;

	createInfo.pDepthStencilState = &depthStencil;
	createInfo.renderPass = pipeline.renderpass;
	createInfo.subpass = 0;

	r = vkCreateGraphicsPipelines(device, 0, 1, &createInfo, 0, &pipeline.pipeline);
  VK_PRINTRES("vkCreateGraphicsPipelines",r);
  free(stages);
  return pipeline;
};


vk_image vk_genimage(unsigned int x, unsigned int y, VkFormat format) {
  vk_image image = {};

  VkImageCreateInfo imageInfo={};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = x > 1 ? x : 1;
	imageInfo.extent.height = y > 1 ? y : 1;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
  imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (format == VK_FORMAT_D32_SFLOAT) {
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  };
  imageInfo.usage |=VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.format=format;	
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  VmaAllocationCreateInfo alloc = {};
  alloc.usage=VMA_MEMORY_USAGE_GPU_ONLY;
  VkResult r = vmaCreateImage(allocator, &imageInfo,&alloc, &image.image, &image.memory,0);
  VK_PRINTRES("vkCreateImage",r);

  image.x=imageInfo.extent.width;
  image.y=imageInfo.extent.width;
  image.format=format;
  return image;
};
void vk_freeimage(vk_image image) {
  vmaDestroyImage(allocator,image.image,image.memory);
};

void vk_barrier(vk_image image, VkImageLayout layout) {
  VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE_KHR;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (image.format == VK_FORMAT_D32_SFLOAT) {
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	};
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image.image;  // The VkImage being transitioned
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(cmd[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, 0, 0, 1, &imageMemoryBarrier);
};

void vk_beginstagingcmd() {
  VkCommandBufferBeginInfo beginInfo = {
    .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(stagingCommandBuffer,&beginInfo);
};
void vk_flushstagingcmd() {
  vkEndCommandBuffer(stagingCommandBuffer);
  VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &stagingCommandBuffer;

	vkQueueSubmit(draw, 1, &submitInfo, 0);
	vkQueueWaitIdle(draw);
  vkResetCommandBuffer(stagingCommandBuffer,0);
};

#endif

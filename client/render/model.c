#include "../render.h"
#include "stdlib.h"
#include "memory.h"
#include <vulkan/vulkan_core.h>
#include "stdio.h"
#include "../window.h"
#include "../../includes/cglm/include/cglm/cglm.h"
#include "../../includes/cglm/include/cglm/affine.h"
extern VkInstance instance;

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue draw;
extern VkQueue present;

extern VkCommandPool cmdPool;
extern VkCommandBuffer cmd[2];
extern VkCommandBuffer stagingCommandBuffer;
extern VkCommandPool stagingCommandPool;

extern window mainwindow;
extern int imageIndex;
extern vk_extensions extensions;


typedef struct vk_model {
  struct vk_mesh {
    vk_buffer vertices;
    vk_buffer indicies;
    float matrix[16];
  }* meshes;
  uint32_t nummeshes;
} vk_model;

typedef struct vk_staticmesh {
  vk_model* model;
  struct mesh {
    struct mesh* next;
    float matrix[16];
  }* meshes;
  struct vk_staticmesh* next;
} vk_staticmesh;
vk_staticmesh* meshes=0;


model draw_genmodel(modelinfo_t* info) {

  vk_model* mod = (vk_model*)malloc(sizeof(vk_model));
  mod->nummeshes=info->nummodels;
  struct vk_mesh* meshes=malloc(sizeof(struct vk_mesh)*mod->nummeshes);

  int i = 0;
  for (struct model_t* model=info->models;model;model=model->next) {
    vk_buffer vertices = vk_genbuffer(model->vertexcount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vk_buffer indices = vk_genbuffer(model->indexcount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    void* buffer;
    VkResult r = vkMapMemory(device,vertices.memory,0,vertices.size,0,&buffer);
    memcpy(buffer,model->vertices,vertices.size);
    vkUnmapMemory(device,vertices.memory);

    vkMapMemory(device,indices.memory,0,indices.size,0,&buffer);
    memcpy(buffer,model->indexes,indices.size);
    vkUnmapMemory(device,indices.memory);


    meshes[i].vertices=vertices;
    meshes[i].indicies=indices;
    i++;
  }
  mod->meshes=meshes;


  return (model)mod;
};
void draw_copymodel(model m);
void draw_destroymodel(model m);

int numdrawedmodels = 0;
int numuniquemodels = 0;


void draw_model(model m, float matrix[16]) {
  vk_staticmesh* foundmesh=0;
  for (vk_staticmesh* mesh = meshes;mesh;mesh=mesh->next) {
    if (mesh->model==m) {
      foundmesh=mesh;
      break;
    }
  };
  if (!foundmesh) {
    foundmesh = malloc(sizeof(vk_staticmesh));
    foundmesh->model = m;
    foundmesh->meshes=0;
    foundmesh->next = meshes;
    meshes = foundmesh;
    numuniquemodels++;
  }

  struct mesh* mesh = malloc(sizeof(struct mesh));
  memcpy(mesh->matrix,matrix,64);
  mesh->next = foundmesh->meshes;
  foundmesh->meshes = mesh;
  numdrawedmodels++;
};
void draw_skinned(model m, skeleton s, animdata a);

vk_buffer instances = {};
vk_buffer matrices = {};
vk_tripipeline pipeline;
VkFramebuffer fb=0;
vk_image depth = {};
VkImageView depthp=0;
void draw_initmodels() {


  memset(&instances,0,sizeof(vk_buffer));
  vk_shader shader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_COMPUTE_BIT,"transform");

  extern cvar_t* barycentrics_mode;

  vk_tripipeline_info info = {};
  vk_shader shaders[3];
  int barymode = atoi(barycentrics_mode->value);
  if (barymode==0) {
    if(extensions._VK_KHR_fragment_shader_barycentric) {
      vk_shader vertshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_VERTEX_BIT,"vertex");
      vk_shader fragshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_FRAGMENT_BIT,"fragment");
      shaders[0]=vertshader;
      shaders[1]=fragshader;
      info.numshaders=2;
    } else {
      printf("------------geometry\n");
      vk_shader vertshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_VERTEX_BIT,"vertex");
      vk_shader fragshader = vk_genshader("shaders/mesh_soft.spv",VK_SHADER_STAGE_FRAGMENT_BIT,"fragment_soft");
      vk_shader geomshader = vk_genshader("shaders/mesh_soft.spv",VK_SHADER_STAGE_GEOMETRY_BIT,"geometry");
      shaders[0]=vertshader;
      shaders[1]=geomshader;
      shaders[2]=fragshader;
      info.numshaders=3;
    }
  }
  if (barymode==1) { 
    vk_shader vertshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_VERTEX_BIT,"vertex");
    vk_shader fragshader = vk_genshader("shaders/mesh_soft.spv",VK_SHADER_STAGE_FRAGMENT_BIT,"fragment_soft");
    vk_shader geomshader = vk_genshader("shaders/mesh_soft.spv",VK_SHADER_STAGE_GEOMETRY_BIT,"geometry");
    shaders[0]=vertshader;
    shaders[1]=geomshader;
    shaders[2]=fragshader;
    info.numshaders=3;
  }
  if (barymode==2) { 
    vk_shader vertshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_VERTEX_BIT,"vertex");
    vk_shader fragshader = vk_genshader("shaders/mesh.spv",VK_SHADER_STAGE_FRAGMENT_BIT,"fragment");
    shaders[0]=vertshader;
    shaders[1]=fragshader;
    info.numshaders=2;
  }
  info.shaders = shaders;

  info.desciptorsnum=0;
  info.polygonmode=VK_POLYGON_MODE_FILL;
  vk_renderpass renderpass[2] = {};
  renderpass[0].format = VK_FORMAT_R8G8B8A8_SRGB;
  info.pushsize=64;
  info.renderpassnum=1;
  info.renderpass = renderpass;
  info.depth=1;

  pipeline = vk_gentripipeline(info);
}
void draw_rendermodels() { 

  if (instances.buffer!=0) {
    vk_freebuffer(instances);
  }
  memset(&instances,0,sizeof(vk_buffer));
  // there is nothing to render
  if (!numuniquemodels) {
    return;
  }
  if (!numdrawedmodels) {
    return;
  }
  
  int x = sys_getwindowidth(mainwindow);
  int y = sys_getwindoheight(mainwindow);
	if (fb) {
		vkDestroyFramebuffer(device, fb, 0);
    vkDestroyImageView(device,depthp,0);
    vk_freeimage(depth);
	}
  depth=vk_genimage(x,y,VK_FORMAT_D32_SFLOAT);
  depthp=vk_genimageview(depth.image,VK_FORMAT_D32_SFLOAT);

	VkFramebufferCreateInfo framebufferInfo={};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = pipeline.renderpass;
	framebufferInfo.attachmentCount = 2;
  VkImageView attachments[2] = {
    sys_getwindowimageview(mainwindow),
    depthp,
  };
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = x;
	framebufferInfo.height = y;
	framebufferInfo.layers = 1;	

	vkCreateFramebuffer(device, &framebufferInfo, 0, &fb);


  vk_barrier(depth,VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
  VkClearDepthStencilValue depthval = {};
  depthval.depth=1;
  VkImageSubresourceRange subresourceRange ={};
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  vkCmdClearDepthStencilImage(cmd[imageIndex],depth.image,VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,&depthval,1,&subresourceRange);

	VkRenderPassBeginInfo renderPassInfo={};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pipeline.renderpass;
	renderPassInfo.framebuffer = fb;
	renderPassInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
	renderPassInfo.renderArea.extent = (VkExtent2D){ x,y };
	renderPassInfo.clearValueCount = 0;

	vkCmdBeginRenderPass(cmd[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

	VkViewport viewport={};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = x;
	viewport.height = y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd[imageIndex], 0, 1, &viewport);

	VkRect2D scissor={};
	scissor.offset = (VkOffset2D){ 0, 0 };
	scissor.extent = (VkExtent2D){ x,y };
	vkCmdSetScissor(cmd[imageIndex], 0, 1, &scissor);
    mat4 matrix = GLM_MAT4_IDENTITY;
    glm_translate(matrix,(vec4){0,0,-30,0});

    mat4 perspectiv = GLM_MAT4_IDENTITY;
    glm_perspective(glm_rad(90),(float)x/(float)y,0.01,100.0,perspectiv);
    glm_mul(perspectiv,matrix,matrix);
    

    vkCmdPushConstants(cmd[imageIndex],pipeline.layout,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,64,matrix);

  for (int i = 0;i<numuniquemodels;i++) {
   
    VkDeviceSize offsets[1] = {
      0
    };
    for (int x = 0;x<meshes[i].model->nummeshes;x++) {
      vkCmdBindVertexBuffers(cmd[imageIndex],0,1,&meshes[i].model->meshes[x].vertices.buffer,offsets);
      vkCmdBindIndexBuffer(cmd[imageIndex],meshes[i].model->meshes[x].indicies.buffer,0,VK_INDEX_TYPE_UINT32);

      vkCmdDrawIndexed(cmd[imageIndex],meshes[i].model->meshes[x].indicies.size/4,1,0,0,0);
    }

  }

  vkCmdEndRenderPass(cmd[imageIndex]);
  



  vk_staticmesh* nextmesh=0;
  for (vk_staticmesh* mesh = meshes;mesh;mesh=nextmesh) {
    nextmesh = mesh->next;
    struct mesh* next=0;
    for (struct mesh* m = mesh->meshes;m;m=next) {
      next = m->next;
      free(m);
    }
    free(mesh);
  };
  meshes = 0;
  numdrawedmodels=0;
  numuniquemodels=0;
};


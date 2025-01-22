#include "mesh_shared.slang"


// vertex shader

struct instances {
  float4x4* matrix;
  uint32_t numinstances;
}

[[vk::binding(1,0)]]
Buffer<instances> batches;

struct vertexresult {
    float4 position : SV_Position;
};

[shader("vertex")]
vertexresult vertex(
  float3 position: POSITION,
) {
  return {mul(vp,float4(position.x,position.y,position.z,1))};
}





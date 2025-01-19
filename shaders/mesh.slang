struct mesh {
  float3* vertices;
  uint3* indicies;
  float4x4 matrix;
  uint32_t id;
}
struct constants {
  float4x4 viewprojection;
}

[shader("compute")]
[numthreads(1,1,1)]
void transform(
  uint3 threadID: SV_DispatchThreadID,
  [[vk::binding(0,0)]]
  RWStructuredBuffer<float4x4> matrices,
  uniform uint32_t nummatrices,
  uniform constants c,
) {
  // code which causes the hung 
  //for (int i = 0;i<nummatrices;i++) {
  //  matrices[i]=matrices[i]*c.viewprojection;
  //}
  return;
}

[[vk::push_constant]]
cbuffer constants {
  float4x4 vp;
}


// vertex shader

struct instances {
  float4x4* matrix;
  uint32_t numinstances;
}

[[vk::binding(1,0)]]
Buffer<instances> batches;

struct vertexresult {
  float4 position: SV_Position;
};

[shader("vertex")]
vertexresult vertex(
  float3 position: POSITION,
) {
  return {mul(vp,float4(position.x,position.y,position.z,1))};
}

// simply draw triangles with barycetrics and other mesh data
[shader("fragment")]
float4 fragment( 
  float3 barycentic: SV_Barycentrics,
  uint32_t triangleID: SV_PrimitiveID,
  uint32_t instanceID: SV_InstanceID,
) {
  return float4(float3(barycentic),0.5);
}

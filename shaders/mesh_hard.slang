#include "mesh_shared.slang"

[shader("fragment")]
float4 fragment(
    float3 barycentric: SV_Barycentrics,
    uint32_t triangleID: SV_PrimitiveID,
    uint32_t instanceID: SV_InstanceID,
) {
  uint triangle = triangleID;
  uint instance = instanceID;
  uint mesh = meshID;
  float r = hash(triangle);
  float g = hash(triangle+1);
  float b = hash(triangle+2);
  float3 result = float3(r, g, b);
  return float4(result,0.0);
}
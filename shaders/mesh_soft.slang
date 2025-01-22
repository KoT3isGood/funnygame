#include "mesh_shared.slang"

[shader("fragment")]
fragmentoutput fragment(
  float3 barycentric: COLOR, 
  uint32_t triangleID: SV_PrimitiveID,
  uint32_t instanceID: SV_InstanceID,
) {
  return fragmentoutput(barycentric.xy,triangleID,meshID);
}
// software barycentics
struct geometryinput {
    float4 position : SV_POSITION;
}

struct geometryoutput {
    float4 position : SV_POSITION;
    float3 color : COLOR;
}

// compute em
[shader("geometry")]
[maxvertexcount(3)]
void main(
    triangle geometryinput input[3],
    inout TriangleStream<geometryoutput> stream
) {
    geometryoutput output;
    output.position = input[0].position;
    output.color = float3(1, 0, 0);
    stream.Append(output);
    output.position = input[1].position;
    output.color = float3(0, 1, 0);
    stream.Append(output);
    output.position = input[2].position;
    output.color = float3(0, 0, 1);
    stream.Append(output);
}
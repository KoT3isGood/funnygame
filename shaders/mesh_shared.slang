struct fragmentoutput {
    float2 barycentics;
    uint32_t triangle;
    uint32_t mesh;
}

[[vk::push_constant]]
cbuffer constants {
    float4x4 vp;
    uint32_t meshID;
};

float hash(int value) {
    value += 1;
    value = (value ^ (value >> 16)) * 0x45d9f3b;
    value = (value ^ (value >> 16)) * 0x45d9f3b;
    value = (value ^ (value >> 16));
    return float(value & 0xFFFFFF) / 0xFFFFFF; // Normalize to [0.0, 1.0]
}
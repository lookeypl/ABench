layout (location = 0) in vec3 InPos;
layout (location = 1) in vec3 InNorm;
layout (location = 2) in vec2 InUV;


out gl_PerVertex
{
    vec4 gl_Position;
};

// temporarily changed from block because glslang has a bug
layout (location = 0) out vec3 VertNorm;
layout (location = 1) out vec2 VertUV;
layout (location = 2) out vec3 VertPosNoProj;



layout (std140, set = 0, binding = 0) uniform dynamicCb
{
    mat4 worldMatrix;
} dynamicCBuffer;

layout (std140, set = 0, binding = 1) uniform cb
{
    mat4 viewMatrix;
    mat4 projMatrix;
} CBuffer;


void main()
{
    mat4 worldView = CBuffer.viewMatrix * dynamicCBuffer.worldMatrix;
    mat4 worldViewProj = CBuffer.projMatrix * worldView;
    gl_Position = worldViewProj * vec4(InPos, 1.0);
    VertNorm = InNorm;
    VertUV = InUV;
    VertPosNoProj = (dynamicCBuffer.worldMatrix * vec4(InPos, 1.0)).xyz;
}

layout (location = 0) in vec3 InPos;


layout (set = 0, binding = 0) uniform dynamicCb
{
    mat4 worldMatrix;
} dynamicCBuffer;

layout (set = 0, binding = 1) uniform cb
{
    mat4 viewMatrix;
    mat4 projMatrix;
} CBuffer;


out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
    mat4 worldView = CBuffer.viewMatrix * dynamicCBuffer.worldMatrix;
    mat4 worldViewProj = CBuffer.projMatrix * worldView;
    gl_Position = worldViewProj * vec4(InPos, 1.0);
}
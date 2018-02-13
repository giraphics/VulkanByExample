// Filename: Quad.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform UniformBufferObject 
{
    vec2 offset;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{   
    vec2 pos = inPosition;
    pos += ubo.offset;
    gl_Position = vec4(pos, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
// Filename: RectInstance.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (std140, binding = 0) uniform TransformBufferStruct {
	mat4 mvp;
} TransformBuffer;

layout(push_constant) uniform colorBlock {
    vec4 inNewColor;
} pushConstantsColorBlock;

// Vextex attributes
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 fragColor;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() 
{
	gl_Position   = TransformBuffer.mvp * inPosition;
    fragColor = pushConstantsColorBlock.inNewColor;
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
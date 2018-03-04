// Filename: Light.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (std140, binding = 0) uniform bufferVals {
	mat4 normalMatrix; // normal mat
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 lightPosition;
} myBufferVals;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inNormal; // normals
layout(location = 0) out vec4 fragColor;

out gl_PerVertex { 
    vec4 gl_Position;
};

vec3 LightColor = vec3(0.6, 0.7, 0.4);
vec3 MaterialColor = vec3(0.3, 0.6, 0.5);

void main() 
{
	vec4 pos = myBufferVals.model * inPosition;
    vec3 nNormal = normalize(mat3(myBufferVals.normalMatrix) * inNormal);
	vec3 eyeCoord = vec3 (myBufferVals.view * inPosition);
    vec3 nLight = normalize(myBufferVals.lightPosition.xyz - eyeCoord);

	float cosAngle = max( 0.0, dot( nNormal, nLight ));
	vec3 diffuse = MaterialColor * LightColor;
	fragColor = vec4(cosAngle * diffuse, 1);

    gl_Position = myBufferVals.projection * myBufferVals.view * myBufferVals.model * inPosition;
}
// Filename: Triangle.vert
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (std140, binding = 0) uniform bufferVals {
mat4 mvp; // normal mat
mat4 p;
mat4 v;
mat4 m;
} myBufferVals;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inColor; // normals
layout(location = 0) out vec4 fragColor;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() 
{
    //gl_Position = myBufferVals.mvp * inPosition;
/*	vec3 nNormal   = normalize ( myBufferVals.mvp * inColor );
	vec3 eyeCoord  = vec3 ( myBufferVals.v * myBufferVals.m * inPosition );
	vec3 LightPosition = vec3(0.0, 1.0, 0.0);
    vec3 nLight    = normalize( LightPosition - eyeCoord );
	    // Calculate cosine between Normal and Light vector
    float cosAngle = max( 0.0, dot( nNormal, nLight ));

    // Compute diffuse Light and Material interaction
    vec3 diffuse   = vec3(0.0, 1.0, 0.0);
	vec3 color 	   = cosAngle * diffuse;
    fragColor 	   = vec4(color, 1);*/

	vec3 LightPosition = vec3(0.0, 0.0, 1.0);
	vec4 pos = myBufferVals.m * inPosition;
    vec3 outNormal = mat3(myBufferVals.m) * inColor;
	vec3 lPos = mat3(myBufferVals.m) * LightPosition;
    vec3 outLightVec = lPos - pos.xyz;
	vec3 outViewVec = -pos.xyz;

	vec3 N = -normalize(outNormal);
	vec3 L = normalize(outLightVec);
	vec3 V = normalize(outViewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0, 1.0, 0.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);

    gl_Position = myBufferVals.p * myBufferVals.v * myBufferVals.m * inPosition;
    fragColor = vec4(diffuse, 1.0);
//	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

//    gl_Position.y = -gl_Position.y; // From GL to Vulkan conventions
}
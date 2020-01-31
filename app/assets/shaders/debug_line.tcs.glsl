#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices=3) out;

layout(location = 1) in vec4 vPosition[];
layout(location = 2) in vec3 normal[];
layout(location = 3) in vec4 tangent[];
layout(location = 4) out vec4 tcPosition[];
layout(location = 5) out vec3 tcNormal[];
layout(location = 6) out vec4 tcTangent[];

void main(void) {
	tcPosition[gl_InvocationID]=vPosition[gl_InvocationID];
	tcNormal[gl_InvocationID]=normal[gl_InvocationID];
	tcTangent[gl_InvocationID]=tangent[gl_InvocationID];
 if(gl_InvocationID==0){
	gl_TessLevelOuter[0] = 1.0;
	gl_TessLevelOuter[1] = 1.0;
	gl_TessLevelOuter[2] = 1.0;
	gl_TessLevelInner[0] = 1.0;
	gl_TessLevelInner[1] = 1.0;
	}
}
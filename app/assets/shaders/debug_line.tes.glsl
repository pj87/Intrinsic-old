#version 450

layout(triangles,equal_spacing,ccw) in;

layout(location = 0) in vec4 tcPosition[];
layout(location = 1) in vec3 tcNormal[];
layout(location = 2) in vec4 tcTangent[];

layout(binding = 0) uniform PerInstance
{
  uniform mat4 invTransp;
  uniform mat4 mvp;//MVP
  uniform mat4 m;
  uniform mat4 mv;
  uniform mat4 invView;
}
uboPerInstance;

layout(location = 3) out vec4 position;// position of the vertex in world space
layout(location = 4) out vec3 varyingNormalDirection;
layout(location = 5) out mat3 tangenteSpace;

 
void main(void) {
	vec3 p = gl_TessCoord.xyz;

	vec4 vertCoord = vec4(normalize(tcPosition[0]*p.x + tcPosition[1]*p.y + tcPosition[2]*p.z).xyz,1.0);
	vec3 vertNormal = normalize(tcNormal[0]*p.x + tcNormal[1]*p.y + tcNormal[2]*p.z);
	vec4 tangent = vec4(normalize(tcTangent[0]*p.x + tcTangent[1]*p.y + tcTangent[2]*p.z).xyzw);

	//comnpute tangent space
	tangenteSpace[0] = normalize(tangent.xyz);
	tangenteSpace[2] = normalize(vertNormal);
	tangenteSpace[1] = normalize(cross(tangenteSpace[0],tangenteSpace[2])*tangent.w);
    tangenteSpace = transpose(tangenteSpace);
	
	//for no bump map
	varyingNormalDirection = normalize(mat3(uboPerInstance.invTransp)*vertNormal);

	//position in world space
	position = uboPerInstance.m * vertCoord;
    //f_texcoord = vec2(texCoord);
 
	gl_Position = uboPerInstance.mvp*vertCoord;
}

#version 450

layout(binding = 0, RGBA8) uniform image2D _TextureTex;
layout(binding = 1, RG8) uniform image2D _NormalTex;
layout(binding = 2, RG8) uniform image2D _PBRTex;

layout(local_size_x = 1u, local_size_y = 1u, local_size_z = 1u) in;
void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID);
	
	imageStore(_TextureTex, id.xy, vec4(1.0, 0.0, 0.0, 1.0));
	imageStore(_NormalTex, id.xy, vec4(1.0, 1.0, 0.0, 1.0));
	imageStore(_PBRTex, id.xy, vec4(0.0, 1.0, 0.0, 1.0));
}

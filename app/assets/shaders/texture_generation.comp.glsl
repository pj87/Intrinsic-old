#version 450

layout(binding = 0, RGBA8) uniform image2D _TextureTex;
layout(binding = 1, RG8) uniform image2D _NormalTex;
layout(binding = 2, RG8) uniform image2D _PBRTex;

layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID);
	
	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 32; y++)
			{
				imageStore(_TextureTex, id.xy + 64 * ivec2(x, y), vec4(1.0, 0.0, 0.0, 1.0));
			}
	
	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 32; y++)
			{
				imageStore(_NormalTex, id.xy + 64 * ivec2(x, y), vec4(1.0, 1.0, 0.0, 1.0));
			}

	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 32; y++)
			{
				imageStore(_PBRTex, id.xy + 64 * ivec2(x, y), vec4(0.0, 1.0, 0.0, 1.0));
			}
}

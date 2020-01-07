#version 450

layout(binding = 0, RGBA8) uniform image2D _NormalTex;

layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID);

	//imageStore(_NormalTex, id, vec4(normalize(vec3(dx,dy,dz)), 0.0));
	/*
	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 32; y++)
			for (int z = 0; z < 12; z++)
			{
				imageStore(_NormalTex, id.xyz + 64 * ivec2(x, y, 0), vec4(1.0, 0.0, 0.0, 1.0));
			}
	
	//imageStore(_NormalTex, id.xy, vec4(1.0, 0.0, 0.0, 1.0));
	//imageStore(_NormalTex, id.xy + ivec2(64, 64), vec4(1.0, 0.0, 0.0, 1.0));
	*/
	
	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 32; y++)
			{
				imageStore(_NormalTex, id.xy + 64 * ivec2(x, y), vec4(1.0, 0.0, 0.0, 1.0));
			}
}

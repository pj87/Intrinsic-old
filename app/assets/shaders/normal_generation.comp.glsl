#version 450

//#pragma kernel CSMain

//StructuredBuffer<float> _Noise;

//RWTexture3D<float3> _Result;

layout(binding = 0) uniform PerInstance { float _dummy; }
uboPerInstance;

layout(binding = 1) buffer _NoiseBuffer 
{
	float _Noise[];
};
//layout(binding = 2, r11f_g11f_b10f) uniform image3D volLightScatterBufferTex;
layout(binding = 2, r11f_g11f_b10f) uniform image3D _NormalBuffer;

int _Width = 64, _Height = 64;

layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID);
	
	float v = _Noise[id.x + id.y * _Width + id.z * _Width * _Height];
	
	float dx = v - _Noise[(id.x+1) + id.y * _Width + id.z * _Width * _Height];
	
	float dy = v -_Noise[id.x + (id.y+1) * _Width + id.z * _Width * _Height];
	
	float dz = v -_Noise[id.x + id.y * _Width + (id.z+1) * _Width * _Height];
	

	//_Result[id] = normalize(vec3(dx,dy,dz));
	imageStore(_NormalBuffer, id, vec4(normalize(vec3(dx,dy,dz)), 0.0));
}

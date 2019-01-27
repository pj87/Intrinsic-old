// Copyright 2017 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#version 450

struct Vert
{
	vec4 position;
	vec3 normal;
};

struct Debug
{
	vec4 debug1;
	vec4 debug2;
};

float _Target = 0.0f;
int _Width = 64, _Height = 64, _Depth = 64, _Border = 1;

layout(binding = 0) uniform PerInstance { float _dummy; }
uboPerInstance;

layout(binding = 1) buffer positionBuffer 
{
	uint _Buffer[];
};
layout(binding = 2) buffer triangleConnectionBuffer 
{
	int _TriangleConnectionTable[];
};
layout(binding = 3) buffer voxelBuffer 
{
	float _Voxels[];
};
layout(binding = 4) buffer debugBuffer 
{
	Debug _DebugBuffer[];
};
layout(binding = 5) uniform sampler3D volLightScatterBufferTex1; // do odczytu
//layout(binding = 6, r11f_g11f_b10f) uniform image3D volLightScatterBufferTex1; // do zapisu

layout(binding = 6) uniform sampler2D gradient3DTex;
layout(binding = 7) uniform sampler2D permTable2DTex;


// edgeConnection lists the index of the endpoint vertices for each of the 12 edges of the cube
ivec2 edgeConnection[12] = {ivec2(0, 1), ivec2(1, 2), ivec2(2, 3), ivec2(3, 0),
						    ivec2(4, 5), ivec2(5, 6), ivec2(6, 7), ivec2(7, 4),
                            ivec2(0, 4), ivec2(1, 5), ivec2(2, 6), ivec2(3, 7)};

// For any edge, if one vertex is inside of the surface and the other is outside
// of the surface
//  then the edge intersects the surface
// For each of the 8 vertices of the cube can be two possible states : either
// inside or outside of the surface For any cube the are 2^8=256 possible sets
// of vertex states This table lists the edges intersected by the surface for
// all 256 possible vertex states There are 12 edges.  For each entry in the
// table, if edge #n is intersected, then bit #n is set to 1

int _CubeEdgeFlags[256] = {
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f,
    0xb06, 0xc0a, 0xd03, 0xe09, 0xf00, 0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f,
    0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 0x230,
    0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936,
    0xe3a, 0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5,
    0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0, 0x460, 0x569,
    0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a,
    0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc,
    0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 0x650, 0x759, 0x453,
    0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53,
    0x859, 0x950, 0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc,
    0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca,
    0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9,
    0x7c0, 0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055,
    0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6,
    0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f,
    0x066, 0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af,
    0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0, 0xd30,
    0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636,
    0x13a, 0x033, 0x339, 0x230, 0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895,
    0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190, 0xf00, 0xe09,
    0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a,
    0x203, 0x109, 0x000};

// edgeDirection lists the direction vector (vertex1-vertex0) for each edge in the cube
vec3 edgeDirection[12] =
{
	vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
	vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),  vec3(0.0f, 0.0f, 1.0f)
};							

// vertexOffset lists the positions, relative to vertex0, of each of the 8 vertices of a cube
vec3 vertexOffset[8] =
{
	vec3(0, 0, 0), vec3(1, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0),
	vec3(0, 0, 1), vec3(1, 0, 1), vec3(1, 1, 1), vec3(0, 1, 1)
};

void FillCube(uint x, uint y, uint z, out float cube[8])
{
	cube[0] = _Voxels[x + y * _Width + z * _Width * _Height];
	cube[1] = _Voxels[(x + 1) + y * _Width + z * _Width * _Height];
	cube[2] = _Voxels[(x + 1) + (y + 1) * _Width + z * _Width * _Height];
	cube[3] = _Voxels[x + (y + 1) * _Width + z * _Width * _Height];

	cube[4] = _Voxels[x + y * _Width + (z + 1) * _Width * _Height];
	cube[5] = _Voxels[(x + 1) + y * _Width + (z + 1) * _Width * _Height];
	cube[6] = _Voxels[(x + 1) + (y + 1) * _Width + (z + 1) * _Width * _Height];
	cube[7] = _Voxels[x + (y + 1) * _Width + (z + 1) * _Width * _Height];
}

// GetOffset finds the approximate point of intersection of the surface
// between two points with the values v1 and v2
float GetOffset(float v1, float v2)
{
	float delta = v2 - v1;
	return (delta == 0.0f) ? 0.5f : (_Target - v1) / delta;
}

Vert CreateVertex(vec3 position, vec3 centre, vec3 size)
{
	Vert vert;
	vert.position = vec4(position - centre, 1.0);

	//vec3 uv = position / size;
	//vert.normal = _Normals.SampleLevel(_LinearClamp, uv, 0);

	return vert;
}

uint convert(vec2 pos0, vec2 pos1)
{
	return(packHalf2x16(pos0) << 16 | packHalf2x16(pos1));
}

void ffff1(uint i, vec3 pos1)
{
	if (i % 2 == 0)
	{
		uint iiii = i + i / 2;
		_Buffer[iiii] = packHalf2x16(pos1.xy);
		vec2 tmp = unpackHalf2x16(_Buffer[iiii + 1]);
		_Buffer[iiii + 1] = packHalf2x16(vec2(pos1.z, tmp.y));
	}
	else
	{	
		uint iiii = i + (i - 1) / 2;
		vec2 tmp = unpackHalf2x16(_Buffer[iiii]);
		_Buffer[iiii] = packHalf2x16(vec2(tmp.x, pos1.x));
		_Buffer[iiii + 1] = packHalf2x16(vec2(pos1.y, pos1.z));
	}
}

							
layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{	
	uvec3 id = gl_GlobalInvocationID;
	uint idx = id.x + id.y * _Width + id.z * _Width * _Height;
	
	//Dont generate verts at the edge as they dont have 
	//neighbours to make a cube from and the normal will 
	//not be correct around border.
	if (id.x >= _Width - 1 - _Border) return;
	if (id.y >= _Height - 1 - _Border) return;
	if (id.z >= _Depth - 1 - _Border) return;

	vec3 pos = vec3(id);
	vec3 centre = vec3(_Width, 0, _Depth) / 2.0;

	float cube[8];
	FillCube(id.x, id.y, id.z, cube);

	int i = 0;
	int flagIndex = 0;
	vec3 edgeVertex[12];

	//Find which vertices are inside of the surface and which are outside
	for (i = 0; i < 8; i++)
	{
		_DebugBuffer[idx].debug1 = vec4(cube[0], cube[1], cube[2], cube[3]);
		_DebugBuffer[idx].debug2 = vec4(cube[4], cube[5], cube[6], cube[7]);
		if (cube[i] <= _Target) flagIndex |= 1 << i;
	}

	//Find which edges are intersected by the surface
	int edgeFlags = _CubeEdgeFlags[flagIndex];
	
	//_Buffer[idx] = edgeFlags;
	
	// no connections, return
	if (edgeFlags == 0) return;
	
	//Find the point of intersection of the surface with each edge
	for (i = 0; i < 12; i++)
	{
		//if there is an intersection on this edge
		if ((edgeFlags & (1 << i)) != 0)
		{
			float offset = GetOffset(cube[edgeConnection[i].x], cube[edgeConnection[i].y]);

			edgeVertex[i] = pos + (vertexOffset[edgeConnection[i].x] + offset * edgeDirection[i]);
		}
	}

	vec3 size = vec3(_Width - 1, _Height - 1, _Depth - 1);
	
	//Save the triangles that were found. There can be up to five per cube
	for (i = 0; i < 5; i++)
	{
		vec3 position;
		
		//If the connection table is not -1 then this a triangle.
		if (_TriangleConnectionTable[flagIndex * 16 + 3 * i] >= 0)
		{	
			//packHalf2x16
			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 0)]];
			//_Buffer[idx * 15 + (3 * i + 0)] = CreateVertex(position, centre, size).position;
			ffff1(idx * 15 + (3 * i + 0), CreateVertex(position, centre, size).position.xyz / 10.0 + 0.01 * texelFetch(volLightScatterBufferTex1, ivec3(0), 0).x);
			
			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 1)]];
			//_Buffer[idx * 15 + (3 * i + 1)] = CreateVertex(position, centre, size).position;
			ffff1(idx * 15 + (3 * i + 1), CreateVertex(position, centre, size).position.xyz / 10.0 + 0.01 * texelFetch(volLightScatterBufferTex1, ivec3(0), 0).y);
			
			position = edgeVertex[_TriangleConnectionTable[flagIndex * 16 + (3 * i + 2)]];
			//_Buffer[idx * 15 + (3 * i + 2)] = CreateVertex(position, centre, size).position;
			ffff1(idx * 15 + (3 * i + 2), CreateVertex(position, centre, size).position.xyz / 10.0 + 0.01 * texelFetch(volLightScatterBufferTex1, ivec3(0), 0).z);
		}
	}
	
	//_Buffer[idx] = idx;
}

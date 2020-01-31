#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

void main() {    
	
	int i,j; 
	vec4 scale = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	
	for (j= 0; j < 2; j++)
	{
		for (i = 0; i < 3; i++)
		{
			gl_Position = gl_in[i].gl_Position * scale;

			EmitVertex();
		}
		
		EndPrimitive();
		scale.xy = -scale.xy;
	}
}  
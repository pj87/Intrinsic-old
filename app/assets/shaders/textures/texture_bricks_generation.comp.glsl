// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// See http://www.iquilezles.org/www/articles/warp/warp.htm for details

// https://www.shadertoy.com/view/MslXz8

#version 450

layout(binding = 0, RGBA8) uniform image2D _TextureTex;
layout(binding = 1) buffer _ParametersBuffer 
{
	float _Frequency;
	float _Lacunarity;
	float _Gain;
};

#define iTime 1.0
#define iResolution vec2(2048.0, 2048)

vec3 BRICK_COLOR = vec3(192.0 / 255.0, 106.0 / 255.0, 59.0 / 255.0);
vec3 BRICK_COLOR_VARIATION = vec3( 30.0 /255.0, 20.0/255.0, 20.0/255.0);

vec3 MORTAR_COLOR = vec3(232.0 / 255.0, 216.0 / 255.0, 195.0 / 255.0);

vec2 BRICK_SIZE = vec2(0.08, 0.06);

vec2 BRICK_PCT = vec2(0.95, 0.9);

void main()
{
	ivec3 id = ivec3(gl_GlobalInvocationID);
	vec2 uv = gl_GlobalInvocationID.xy / iResolution.xy;
    
    vec2 position = uv / BRICK_SIZE;
    
    if(fract(position.y * 0.5) > 0.5)
    {
     	position.x += 0.5;   
    }
    
    position = fract(position);
    
    vec2 useBrick = step(position, BRICK_PCT);
    
    //vec3 texSample 	= texture( iChannel0, uv ).rgb;
    
    vec3 color = mix(MORTAR_COLOR, BRICK_COLOR, useBrick.x * useBrick.y) + BRICK_COLOR_VARIATION;// * texSample;
    
	vec4 fragColor = vec4(color,1.0);
	
	imageStore(_TextureTex, id.xy, fragColor);
}

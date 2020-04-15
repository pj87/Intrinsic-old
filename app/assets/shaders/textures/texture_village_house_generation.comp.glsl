// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// See http://www.iquilezles.org/www/articles/warp/warp.htm for details

// https://www.shadertoy.com/view/MslXz8

#version 450

#define iTime 1.0
#define iResolution vec2(2048.0, 2048)

layout(binding = 0, RGBA8) uniform image2D _TextureTex;
layout(binding = 1) buffer _ParametersBuffer 
{
	float _Frequency;
	float _Lacunarity;
	float _Gain;
};

// https://www.shadertoy.com/view/Mds3W7
// Lightning shader
// rand,noise,fmb functions from https://www.shadertoy.com/view/Xsl3zN
// jerome

float rand(vec2 n) {
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
/*
float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float fbm(vec2 n) {
    float total = 0.0, amplitude = 1.0;
    for (int i = 0; i < 7; i++) {
        total += noise(n) * amplitude;
        n += n;
        amplitude *= 0.5;
    }
    return total;
}
*/
// Noise
vec2 random(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

float noise(vec2 st)
{
    vec2 f = fract(st);
    vec2 i = floor(st);
    
    vec2 u = f * f * f * (f * (f * 6. - 15.) + 10.);
    
    float r = mix( mix( dot( random(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
    return r * .5 + .5;
}

float fbm(vec2 st)
{
    float value = 0.;
    float amplitude = .5;
    float frequency = 0.;
    
    for (int i = 0; i < 8; i++)
    {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    
    return value;
}

vec3 textureWood(vec2 uv)
{
    uv.x *= .6;
    
    float no = noise(vec2(1.2, 2.4) + uv * 6.);

    float n0 = .6 + .4 * smoothstep(
        .24,
        0.55,
        fbm(vec2(uv.x * 10., uv.y * 30.) + vec2(15.0, 10.0)));
    uv += no;
    float n1 = fbm(vec2(uv.x * 5., uv.y * 20.) + vec2(2.0, 2.0));
    float n2 = smoothstep(
        1.0,
        0.3,
        fbm(vec2(uv.x * 1., uv.y * 10.)));
    
    vec3 col = n0 * n1 * n2 * vec3(1.92, 1.4, 1.15);
    return pow(col, vec3(2.1));
}

vec3 textureWall(vec2 uv)
{
    vec2 iuv = floor(uv * 10.0);
    vec3 col = clamp(smoothstep(0.14, 0.65, noise(uv * 10.4) * fbm(uv * 18.0)) + .9, 0.0, 1.0) * vec3(0.5);
    return col;
}

vec3 textureFloor(vec2 uv)
{
    vec2 iuv = floor(uv * 100.0);
    float v = float(mod(iuv.x + iuv.y, 2.0) <= 0.01);
    vec3 col = vec3(0.4 + 0.5 * v) * fbm(uv * 15.5) * vec3(0.75, 0.68, 0.591);
    return col;
}

vec3 BRICK_COLOR = vec3(192.0 / 255.0, 106.0 / 255.0, 59.0 / 255.0);
vec3 BRICK_COLOR_VARIATION = vec3( 30.0 /255.0, 20.0/255.0, 20.0/255.0);

vec3 MORTAR_COLOR = vec3(232.0 / 255.0, 216.0 / 255.0, 195.0 / 255.0);

vec2 BRICK_SIZE = vec2(0.01, 0.005);

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
    
	// lightning++
	// draw a line, left side is fixed
    vec2 t = uv * vec2(2.0,1.0);
    float ycenter = fbm(t)*0.5;
    float diff = abs(uv.y - ycenter);
    //float c1 = 1.0 - mix(0.0,1.0,diff*200.0);
	//vec4 col = vec4(c1*0.6,0.2*c1,c1,1.0); // purple color
	float diff1 = uv.y - ycenter;
	// lightning--
	
	vec4 fragColor = vec4(0.0);
	
	//if (uv.y > diff * 10.0 && uv.y < diff * 200.0)
        fragColor = vec4(color, 1.0);
    //else
    //    fragColor = vec4(0.75, 0.75, 0.75, 1.0);
	
	if (id.x < iResolution.x / 2 && id.y < iResolution.y / 2)
		fragColor = vec4(color, 1.0);
	else if (id.x >= iResolution.x / 2 && id.y < iResolution.y / 2)
		fragColor = vec4(textureWood(uv), 1.0);
	else if (id.x < iResolution.x / 2 && id.y >= iResolution.y / 2)
		fragColor = vec4(textureFloor(uv), 1.0);
	else if (id.x >= iResolution.x / 2 && id.y >= iResolution.y / 2)
		fragColor = vec4(textureWall(uv), 1.0);
	
	imageStore(_TextureTex, id.xy, fragColor);
}

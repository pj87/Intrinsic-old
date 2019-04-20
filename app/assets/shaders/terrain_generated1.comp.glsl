#version 450

layout(binding = 0) buffer _SizeBuffer 
{
	int _Width;
	int _Height;
};

layout(binding = 1) buffer _ParametersBuffer 
{
	float _Frequency;
	float _Lacunarity;
	float _Gain;
};

layout(binding = 2) uniform sampler2D _Gradient3D;

layout(binding = 3) uniform sampler2D _PermTable2D;

layout(binding = 4) buffer _VoxelBuffer 
{
	float _Result[];
};

const mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

float noi( in vec2 p )
{
    return 0.5*(cos(6.2831*p.x) + cos(6.2831*p.y));
}

float terrainLow( vec2 p )
{
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<2; i++ )
	{
        t += s*noi( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
	return t*55.0;
}

float terrainMed( vec2 p )
{
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<5; i++ )
	{
        t += s*noi( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
            
    return t*55.0;
}

float terrainHigh( vec2 p )
{
    vec2 q = p;
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<5; i++ )
	{
        t += s*noi( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
    
    //t +=   0.05*textureLod( iChannel0, 0.001*q, 0.0 ).x;
    //t +=   0.03*textureLod( iChannel0, 0.005*q, 0.0 ).x;
    //t += t*0.03*textureLod( iChannel0, 0.020*q, 0.0 ).x;

	return t*55.0;
}

float tubes( vec3 pos, float time )
{
    float sep = 400.0;

    //pos.z -= sep*0.025*noi( 0.005*pos.xz*vec2(0.5,1.5) );
    //pos.x -= sep*0.050*noi( 0.005*pos.zy*vec2(0.5,1.5) );
    
    vec3 qos = vec3(0.0);//mod( pos + sep*0.5, sep ) - sep*0.5; 
    //qos.y = pos.y - 70.0;
    qos.x += sep*0.3*cos( 0.01*pos.z);
    qos.y += sep*0.1*cos( 0.01*pos.x );

    float sph = length( qos.xy )/* - sep*0.012*/;

    //sph -= (1.0-0.8*smoothstep(-10.0,0.0,qos.y))*sep*0.003*noi( 0.15*pos.xy*vec2(0.2,1.0) );

    return sph;
}


float tubesH( vec3 pos, float time )
{
    float t = tubes( pos, time );

    //t += 1.0*texture( iChannel3, 0.01*pos.yz ).x;
    //t += 2.0*texture( iChannel0, 0.005*pos.xy ).x;

    return t;
}

float map( in vec3 pos, float time )
{
    float m = 0.0;
	float h = pos.y - terrainMed(pos.xz);

    float sph = tubes( pos, time );
    float k = 60.0;
    float w = clamp( 0.5 + 0.5*(h-sph)/k, 0.0, 1.0 );
    h = mix( h, sph, w ) - k*w*(1.0-w);
    m = mix( m, 1.0, w ) - 1.0*w*(1.0-w);
    m = clamp(m,0.0,1.0);

	return h;
	
    //return vec2( h, m );
}

float mapScaled(vec3 p, vec4 c)
{
	return map(p / 0.1, 0.0) * 0.1;
	//return Map(p / 0.1) * 0.1;
}

layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{
    uvec3 id = gl_GlobalInvocationID;
	vec3 uv = vec3(id);

	//uv *= 0.0001;

	//uncomment this for fractal noise
	//float n = fBm(uv, 4);

	//uncomment this for turbulent noise
	//float n = turbulence(uv, 4);

	//uncomment this for ridged multi fractal
	//float n = ridgedmf(uv, 4, 1.0);

	vec4 c = 0.45*cos( vec4(0.5,3.9,1.4,1.1) + _Frequency*vec4(1.2,1.7,1.3,2.5) ) - vec4(0.3,0.0,0.0,0.0);
	//vec4 c = vec4(0.4);
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -mapScaled(uv - vec3(25.0f, 10.0f, 0.0f), c);
	_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -mapScaled(uv - vec3(100.0f, 1.0f, 100.0f), c);
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -mapScaled(uv - vec3(100.0f * sin(_Frequency)), c);
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdTorus(uv - vec3(_Frequency), vec2(5.0, 2.0));
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdBox(uv - vec3(10.0), vec3(10.0, 5.0, 5.0));
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdSphere(uv - vec3(10.0), 10.0);

}

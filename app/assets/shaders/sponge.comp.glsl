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

layout(binding = 5) buffer _VoxelNormalBuffer
{
    vec4 _NormalResult[];
};

// Regular Menger Sponge formula. Very simple, but if you're not sure, look it
// up on Wikipedia, and look up a Void Cube image.
float sponge(vec3 q){
    
    vec3 p;
	// Scale factor, and distance.
    float s = 3., d = 0.;
    
    for(int i=0; i<3; i++){
 		// Repeat space.
        p = abs(fract(q/s)*s - s/2.); // Equivalent to: p = abs(mod(q, s) - s/2.);
		// Repeat Void Cubes. Cubes with a cross taken out.
 		d = max(d, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - s/3.);
    	s /= 3.; // Divide space (each dimension) by 3.
    }
 
 	return d;    
}

const mat3 ma = mat3( 0.60, 0.00,  0.80,
                      0.00, 1.00,  0.00,
                     -0.80, 0.00,  0.60 );

float sponge1( in vec3 p )
{
    float d = sdBox(p,vec3(1.0));
    vec4 res = vec4( d, 1.0, 0.0, 0.0 );
	
	const float iTime = _Frequency;
	
    float ani = smoothstep( -0.2, 0.2, -cos(0.5*iTime) );
	float off = 1.5*sin( 0.01*iTime );
	
    float s = 1.0;
    for( int m=0; m<3; m++ )
    {
        p = mix( p, ma*(p+off), ani );
	   
        vec3 a = mod( p*s, 2.0 )-1.0;
        s *= 3.0;
        vec3 r = abs(1.0 - 3.0*abs(a));
        float da = max(r.x,r.y);
        float db = max(r.y,r.z);
        float dc = max(r.z,r.x);
        float c = (min(da,min(db,dc))-1.0)/s;

        if( c>d )
        {
          d = c;
          res = vec4( d, min(res.y,0.2*da*db*dc), (1.0+float(m))/4.0, 0.0 );
        }
    }
	
    return res.x;
}

const float detail = .00002;
float det = 0.;

float de2(vec3 p) {
    vec3 op = p;
    p = abs(1.0 - mod(p, 2.));
    float r = 0., power = 8., dr = 1.;
    vec3 z = p;
    for (int i = 0; i < 2; i++) {
        op = -1.0 + 2.0 * fract(0.5 * op + 0.5);
        float r2 = dot(op, op);
        r = length(z);


        if (r > 1.616) break;
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);

        dr = pow(r, power - 1.) * power * dr + 1.;
        float zr = pow(r, power);
        theta = theta * power;
        phi = phi * power;
        z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += p;
    }
    return (.5 * log(r) * r / dr);
}

float de1(vec3 p) {
    float s = 1.;
    float d = 0.;
    vec3 r = p, q = r;
    for (int j = 0; j < 6; j++) {
	   
        r = max(r = abs(mod(q * s + 1., 2.) - 1.), r.yzx);
	    
        d = max(d, (.3 - length(r * 0.95) * .3) / s);
	    
	s *= 2.;
    }
    return d;
}

float sponge2(vec3 p) {
    return min(de1(p), de2(p));;
}

float sdCross(vec3 p)
{
	// infinity doesn't exist, so 123456789. does the job
	return min(sdBox(p, vec3(123456789., 1., 1.)),
			   min(sdBox(p, vec3(1., 123456789., 1.)),
			   sdBox(p, vec3(1., 1., 123456789.))));
}

float dist2nearest(vec3 p)
{
	// repeat the cross with 1. between each in every direction
	vec3 q = mod(p, 1.) - .5;
	return sdCross(q * 27.) / 27.;
}

float mapScaled(vec3 p, vec4 c)
{
	//return map2(p/0.0025f, c) * 0.0025f;
	//return map3(p/0.0025f) * 0.0025f;
	return sponge1(p / 0.0025) * 0.0025;
	//return sponge2(p / 0.0025) * 0.0025;
}

layout(local_size_x = 8u, local_size_y = 8u, local_size_z = 8u) in;
void main()
{
    uvec3 id = gl_GlobalInvocationID;
	vec3 uv = vec3(id);

	uv *= 0.0001;

	//uncomment this for fractal noise
	//float n = fBm(uv, 4);

	//uncomment this for turbulent noise
	//float n = turbulence(uv, 4);

	//uncomment this for ridged multi fractal
	//float n = ridgedmf(uv, 4, 1.0);

	vec4 c = 0.45*cos( vec4(0.5,3.9,1.4,1.1) + _Frequency*vec4(1.2,1.7,1.3,2.5) ) - vec4(0.3,0.0,0.0,0.0);
	//vec4 c = vec4(0.4);
	_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -mapScaled(uv - vec3(0.002f), c);
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -mapScaled(uv - vec3(100.0f * sin(_Frequency)), c);
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdTorus(uv - vec3(_Frequency), vec2(5.0, 2.0));
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdBox(uv - vec3(1.5), vec3(1.5, 1.5, 1.5));

	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdSphere(uv - vec3(10.0), 10.0);
    _NormalResult[id.x + id.y * _Width + id.z * _Width * _Height] = vec4(0.0);
}

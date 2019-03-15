/*
https://www.shadertoy.com/view/MsS3W3
https://www.shadertoy.com/view/lldBDn
https://www.shadertoy.com/view/4lX3Rj
https://www.shadertoy.com/view/MsS3W3
https://www.shadertoy.com/view/4lXGRB
https://www.shadertoy.com/view/XsjBRy
https://www.shadertoy.com/view/4stGD2
https://www.shadertoy.com/view/MsXXWn
https://www.shadertoy.com/view/4t2BDD
https://www.shadertoy.com/view/WdsXDH
https://www.shadertoy.com/view/MscyW4
*/

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

vec3 fmod(vec3 x, float y)
{
	return (x - y * trunc(x/y));
}

vec3 fade(vec3 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

vec4 perm2d(vec2 uv)
{
	return textureLod(_PermTable2D, uv, 0);
}

float gradperm(float x, vec3 p)
{
	vec3 g = textureLod(_Gradient3D, vec2(x, 0), .0).rgb * 2.0 - 1.0;
	return dot(g, p);
}
						
float inoise(vec3 p)
{
	vec3 P = fmod(floor(p), 256.0);	// FIND UNIT CUBE THAT CONTAINS POINT
  	p -= floor(p);                  // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	vec3 f = fade(p);               // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

	P = P / 256.0;
	const float one = 1.0 / 256.0;
	
    // HASH COORDINATES OF THE 8 CUBE CORNERS
	vec4 AA = perm2d(P.xy) + P.z;
 
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
  	return mix( mix( mix( gradperm(AA.x, p ),  
                             gradperm(AA.z, p + vec3(-1, 0, 0) ), f.x),
                       mix( gradperm(AA.y, p + vec3(0, -1, 0) ),
                             gradperm(AA.w, p + vec3(-1, -1, 0) ), f.x), f.y),
                             
                 mix( mix( gradperm(AA.x+one, p + vec3(0, 0, -1) ),
                             gradperm(AA.z+one, p + vec3(-1, 0, -1) ), f.x),
                       mix( gradperm(AA.y+one, p + vec3(0, -1, -1) ),
                             gradperm(AA.w+one, p + vec3(-1, -1, -1) ), f.x), f.y), f.z);
}

// fractal sum, range -1.0 - 1.0
float fBm(vec3 p, int octaves)
{
	float freq = _Frequency, amp = 0.5;
	float sum = 0;	
	for(int i = 0; i < octaves; i++) 
	{
		sum += inoise(p * freq) * amp;
		freq *= _Lacunarity;
		amp *= _Gain;
	}
	return sum;
}

// fractal abs sum, range 0.0 - 1.0
float turbulence(vec3 p, int octaves)
{
	float sum = 0;
	float freq = _Frequency, amp = 1.0;
	for(int i = 0; i < octaves; i++) 
	{
		sum += abs(inoise(p*freq))*amp;
		freq *= _Lacunarity;
		amp *= _Gain;
	}
	return sum;
}

// Ridged multifractal, range 0.0 - 1.0
// See "Texturing & Modeling, A Procedural Approach", Chapter 12
float ridge(float h, float offset)
{
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(vec3 p, int octaves, float offset)
{
	float sum = 0;
	float freq = _Frequency, amp = 0.5;
	float prev = 1.0;
	for(int i = 0; i < octaves; i++) 
	{
		float n = ridge(inoise(p*freq), offset);
		sum += n*amp*prev;
		prev = n;
		freq *= _Lacunarity;
		amp *= _Gain;
	}
	return sum;
}

float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,d.z)),0.0); // remove this line for an only partially signed sdf 
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdPlane( vec3 p )
{
	return p.y;
}

/*
float map(vec3 p, vec4 c)
{
	p/10.0;
    vec3 z = p;
    float m = dot(z,z);

    vec4 trap = vec4(abs(z),m);
	float dz = 1.0;
    
    
	for( int i=0; i<4; i++ )
    {
		dz = 8.0*pow(m,3.5)*dz;
                
        float r = length(z);
        float b = 8.0*acos( clamp(z.y/r, -1.0, 1.0));
        float a = 8.0*atan( z.x, z.z );
        z = c + pow(r,8.0) * vec3( sin(b)*sin(a), cos(b), sin(b)*cos(a) );

        trap = min( trap, vec4(abs(z),m) );

        m = dot(z,z);
		if( m > 2.0 )
            break;
    }

    return 10.0*0.25*log(m)*sqrt(m)/dz;
}
*/

vec4 qsqr( in vec4 a ) // square a quaterion
{
    return vec4( a.x*a.x - a.y*a.y - a.z*a.z - a.w*a.w,
                 2.0*a.x*a.y,
                 2.0*a.x*a.z,
                 2.0*a.x*a.w );
}

float map1(vec3 p, vec4 c)
{
    vec4 z = vec4(p,0.0);
    float md2 = 1.0;
    float mz2 = dot(z,z);

    //vec4 trap = vec4(abs(z.xyz),dot(z,z));

    for( int i=0; i<11; i++ )
    {
        md2 *= 4.0*mz2;   // dz -> 2·z·dz, meaning |dz| -> 2·|z|·|dz| (can take the 4 out of the loop and do an exp2() afterwards)
        z = qsqr(z) + c;  // z  -> z^2 + c

        //trap = min( trap, vec4(abs(z.xyz),dot(z,z)) );

        mz2 = dot(z,z);
        if(mz2>4.0) break;
    }

    return 0.25*sqrt(mz2/md2)*log(mz2);  // d = 0.5·|z|·log|z| / |dz|
}

/*
float opScale( in vec3 p, in float s)
{
    return -map(p/s)*s;
}
*/

vec4 qSquare( vec4 a )
{
    return vec4( a.x*a.x - dot(a.yzw,a.yzw), 2.0*a.x*(a.yzw) );
}

vec4 qCube( vec4 a )
{
	return a * ( 4.0*a.x*a.x - dot(a,a)*vec4(3.0,1.0,1.0,1.0) );
}

//--------------------------------------------------------------------------------

float lengthSquared( vec4 z ) { return dot(z,z); }

// animation

float map2( vec3 p, vec4 c )
{
    vec4 z = vec4( p, 0.2 );
	
	float m2 = 0.0;
	vec2  t = vec2( 1e10 );

	float dz2 = 1.0;
	for( int i=0; i<4; i++ ) 
	{
        // |dz|² = |3z²|²
		dz2 *= 9.0*lengthSquared(qSquare(z));
        
		// z = z^3 + c		
		z = qCube( z ) + c;
		
        // stop under divergence		
        m2 = dot(z, z);		
        if( m2>10000.0 ) break;				 

        // orbit trapping ( |z|² and z_x  )
		//t = min( t, vec2( m2, abs(z.x)) );

	}

	// distance estimator: d(z) = 0.5·log|z|·|z|/|dz|   (see http://iquilezles.org/www/articles/distancefractals/distancefractals.htm)
	float d = 0.25 * log(m2) * sqrt(m2/dz2 );
	//float d = log(m2);

	return d;
}

float map3( in vec3 p)
{
    vec3 w = p;
    float m = dot(w,w);
	float dz = 1.0;
    
	for( int i=0; i<1; i++ )
    {
        dz = 8.0*pow(sqrt(m),7.0)*dz + 1.0;
		//dz = 8.0*pow(m,3.5)*dz + 1.0;
        
        float r = length(w);
        float b = 8.0*acos( w.y/r);
        float a = 8.0*atan( w.x, w.z );
        w = p + pow(r,8.0) * vec3( sin(b)*sin(a), cos(b), sin(b)*cos(a) );

        m = dot(w,w);
		if( m > 256.0 )
            break;
    }

    return 0.25*log(m)*sqrt(m)/dz;
}

#define PHI (sqrt(5.)*0.5 + 0.5)
const float PI = 3.14159265359;
float snoise(vec3 v);

//-------------------------------------------------------  PPRIMITIVES / OPERATIONS
vec3 orbit(float phi, float theta, float radius)
{
	return vec3(
		radius * sin( phi ) * cos( theta ),
		radius * cos( phi ),
		radius * sin( phi ) * sin( theta )
	);
}
float pyramid( vec3 p, float h) {
	vec3 q=abs(p);
	return max(-p.y, (q.x*2.1+q.y+q.z*2.1-h)/3.0 );
}

void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}
void pR45(inout vec2 p) {
	p = (p + vec2(p.y, -p.x))*sqrt(0.5);
}
float fOpUnionRound(float a, float b, float r) {
	vec2 u = max(vec2(r - a,r - b), vec2(0));
	return max(r, min (a, b)) - length(u);
}
float fCylinder(vec3 p, float r, float height) {
	float d = length(p.xz) - r;
	d = max(d, abs(p.y) - height);
	return d;
}
float fTorus(vec3 p, float smallRadius, float largeRadius) {
	return length(vec2(length(p.xz) - largeRadius, p.y)) - smallRadius;
}
float fSphere(vec3 p, float r) {
	return length(p) - r;
}
float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}
float fBox(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}
float fOpIntersectionChamfer(float a, float b, float r) {
	return max(max(a, b), (a + r + b)*sqrt(0.5));
}
float fOpIntersectionRound(float a, float b, float r) {
	vec2 u = max(vec2(r + a,r + b), vec2(0));
	return min(-r, max (a, b)) + length(u);
}
vec2 pModMirror2(inout vec2 p, vec2 size) {
	vec2 halfsize = size*0.5;
	vec2 c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	p *= mod(c,vec2(2.))*2. - vec2(1.);
	return c;
}
float fOpUnionStairs(float a, float b, float r, float n) {
	float s = r/n;
	float u = b-r;
	return min(min(a,b), 0.5 * (u + a + abs ((mod (u - a + s, 2. * s)) - s)));
}

//------------------------------------------------------------------ MAP
float cathedral( in vec3 pos ) {
    //arch
    vec3 q = pos;
    pModMirror2(q.xz,vec2(22.));
    q.y -= 40.;
	//q.y -= 10.;
    q.zx -= 21.;
    pR(q.yz,PI/2.);
    pR45(q.xy);
    float d = fTorus(q,6.3,44.);
	
    q = pos;
    pModMirror2(q.xz,vec2(500.));
    d = max(-fBox(q-vec3(0.,10.5,0.),vec3(500.,30.,500.)),d);
    
    //column
    q = pos;
    q.zx -= 22.;
    //q.zx += texture(iChannel0,pos.xy/50.).x*.5;
    vec2 idx = pModMirror2(q.xz,vec2(22.));
    q.xz -= 9.5;
    d = fOpUnionStairs(d, fCylinder(q,6.,40.),4.,10.);
    
    //ground
    //d = fOpUnionStairs(d, pos.y+40.1+texture(iChannel0,pos.xz/50.).x*.5, 5., 5.);
	d = fOpUnionStairs(d, pos.y+40.1, 5., 5.);
    
    //pyramid on top of arch
    q = pos;
    q.y -= 80.;
    q.zx += 11.;
    pModMirror2(q.xz,vec2(44.));
    pR45(q.xz);
    float d2 = pyramid( q, 32. );
    pR(q.xy,PI);
    d2 = min(d2,pyramid( q, 12. ));
    d = fOpUnionRound(d, d2,1.);
	
    return d;
}

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

// k should be negative. -4.0 works nicely.
float smin(float a, float b, float k)
{
	// I'm guessing that base 2 operations are the fastest.
	return log2(exp2(k*a)+exp2(k*b))/k;
}

vec3 RotateY(vec3 v, float rad)
{
	float cos = cos(rad);
	float sin = sin(rad);
	//if (RIGHT_HANDED_COORD)
	return vec3(cos * v.x - sin * v.z, v.y, sin * v.x + cos * v.z);
	//else return new float3(cos * x + sin * z, y, -sin * x + cos * z);
}

float dSphere(vec3 p, float rad)
{
	return length(p) - rad;
}

float dBox(vec3 pos, vec3 b)
{
	return length(max(abs(pos)-(b),0.0));
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
	vec3 pa = p-a, ba = b-a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h ) - r;
}

float dBoxSigned(vec3 p)
{
	float b = 1.0;
	vec3 b2 = vec3(6.0, 2.0, 2.0);
	vec3 center = vec3(0, -2.0, 0.0);
	vec3 d = abs(p - center) - b2;//*abs(cos(p.y + 0.5));
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float dBoxSlant(vec3 p, vec3 b)
{
	float size = b.x;
	float f = length(max(abs(p)-(b),0.0));
	//float f = length(max(abs(p.x)+abs(p.z)-(b),0.0));
	//f = max(f, p.y*0.5 + abs(p.x) + abs(p.z) - size * 0.995);
	return f;
}

float dFloor(vec3 p)
{
	return p.y + 1.0;
}

float sdColumn( vec3 p, vec3 c )
{
	float cyl = length(p.xz-c.xy)-c.z;// + abs(p.y);
	cyl -= cos(p.y*2.0)*0.045;
	float a = atan(p.x - c.x, p.z - c.y);
	a /= 2.0*PI;
	float subs = 48.0;
	a *= subs;
	//cyl *= pow(sin(a), 0.5) * 0.925 + 1.0;
	cyl += abs(sin(a)) * 0.015;

	cyl = max(cyl, p.y - 2.4);
	cyl = min(cyl, dBox(p + vec3(0.0, 1.0, 0.0), vec3(0.3, 0.2, 0.3)));
	cyl = smin(cyl, dBoxSlant(RotateY(p, PI/4.0) + vec3(0.0, -2.3, 0.0), vec3(0.3, 0.15, 0.3)), -24.0);
	//cyl = min(cyl, dBox(RotateY(p, PI/4.0) + vec3(0.0, -2.3, 0.0), vec3(0.3, 0.15, 0.3)));
	return cyl;
}

float length16(vec2 v)
{
	return pow(pow(abs(v.x),16.0) + pow(abs(v.y), 16.0), 1.0/16.0);
	//return pow((pow(v.x,16.0) + pow(v.y, 16.0)), (1.0/16.0));
}

float globalRadial;

float sdTorusBricks( vec3 p, vec2 t, vec3 center, float subs )
{
	p -= center;
	float a = globalRadial;// + PI/6.0;// atan(p.x, p.z);
	a = pow(abs(sin(a*subs)), 0.25);
	//a = mod(a,PI*2.0) - 0.5;
	//a = a *0.025 + 0.975;
	a = a *0.2 + 0.8;
	vec2 q = vec2(length(p.xz)-t.x,p.y);
	return length16(q)-t.y*a;
}
float sdTorusArch( vec3 p, vec2 t, vec3 center, float subs )
{
	p -= center;
	float a = atan(p.y, p.z);
	a = pow(abs(sin(a*subs)), 0.25);
	//a = mod(a,PI*2.0) - 0.5;
	//a = a *0.025 + 0.975;
	a = a *0.25 + 0.75;
	p.y -= cos(p.y)*0.4;
	vec2 q = vec2(length(p.yz)-t.x,p.x);
	return length16(q)-t.y*a;
}

// This makes the stained glass windows in the dome.
float dTiles(vec3 p)
{
	float subs = 16.0;
	float final = length(p) - 2.2;
	float a = globalRadial;// atan(p.x, p.z);
	a /= 2.0*PI;
	a *= subs;
	a = abs((fract(a) - 0.5))*2.0;	// triangle wave from 0.0 to 1.0
	a -= 0.15;
	a *= 6.0;
	a = max(0.0, a);
	a = min(0.75, a);

	float b = atan(length(p.xz), p.y);
	b /= 2.0*PI;
	b *= subs;
	b = abs((fract(b) - 0.5))*2.0;	// triangle wave from 0.0 to 1.0
	b -= 0.15;
	b *= 6.0;
	b = max(0.0, b);
	b = min(0.75, b);
	
	a = a*b;

	a = a *0.2 + 0.8;
	b = b *0.2 + 0.8;
	
	final = final - a;
	final = max(final, 0.5-p.y);
	final = min(final, length(p.xz) - 0.5);	// oculus - cylinder in middle
	return final/1.414;
}

float cathedral2(vec3 p)
{
	globalRadial = atan(p.x, p.z);
	// set up repeating spaces for pillars and arches
	vec3 c = vec3(1.0, 1.0, 1.0)* 4.0;
	float c2 = 5.2;
	vec3 q = mod(p,c)-0.5*c;
	float q2 = mod(p.x,c2)-0.5*c2;
	vec3 p2 = vec3(q.x, p.y, q.z);
	vec3 p3 = vec3(q2, p.y, p.z);

	float final = -sdCapsule(p, vec3(0.0,-0.5,0.0), vec3(0.0,2.25,0.0), 3.0);
	// This if condition is for a culling speedup and a cool bevel effect on the ceiling tiles.
	final = max(final, -dTiles(p + vec3(0.0, -2.75, 0.0)));
	final = min(final, sdTorusBricks(p, vec2(2.75, 0.25), vec3(0.0, -0.795, 0.0), 12.0));
	final = max(final, -sdCapsule(p, vec3(-6.0,0.0,0.0), vec3(6.0,0.0,0.0), 2.0));
	final = max(final, -sdCapsule(p, vec3(0.0,0.0,-16.0), vec3(0.0,0.0,16.0), 2.0));
	final = max(final, -dBoxSigned(p));
	final = max(final, -sdCapsule(p, vec3(0.0,0.0,0.0), vec3(0.0,5.5,0.0), 0.5));//oculus
	final = max(final, p.y - 5.3);	// open the sky
	//final = max(final, sdCapsule(p, vec3(0.0,-0.5,0.0), vec3(0.0,0.5,0.0), 3.05));
	final = max(final, -dSphere(p2, 0.08));
	//final = min(final, sdColumn(p2, vec3(0.0, 0.0, 0.25)));
	//final = max(final, -sdBox(p - vec3(0.0,0.5,0.0), vec3(0.5, 1.0, 3.5)));

	//final = min(final, sdTorusBricks(p, vec2(2.75, 0.25), vec3(0.0, 2.7, 0.0), 8.0));
	//final = min(final, sdTorusBricks(p, vec2(0.75, 0.25), vec3(0.0, -1.0, 0.0), 3.0));
	//final = min(final, sdTorusArch(p3, vec2(2.125, 0.3), vec3(0.0, 0.1, 0.0), 12.0));
	//final = min(final, sdTorusArch(p, vec2(2.125, 0.3), vec3(2.6, -0.1, 0.0), 6.0));
	//final = min(final, sdTorusArch(p, vec2(2.125, 0.3), vec3(-2.6, -0.1, 0.0), 6.0));
	final = min(final, dFloor(p));

	return final;
}

float mapScaled(vec3 p, vec4 c)
{
	//return map2(p/0.0025f, c) * 0.0025f;
	//return map3(p/0.0025f) * 0.0025f;
	//return cathedral(p/0.0025f) * 0.0025f;
	//return cathedral(p/100.0f) * 100.0f;
	//return cathedral(p/0.000025f) * 0.000025f;
	//return sponge1(p / 0.0025) * 0.0025;
	
	//return dist2nearest(p / 0.0025) * 0.0025;
	
	//return sponge2(p / 0.001) * 0.001;
	
	//return cathedral2(p / 0.0005) * 0.0005;
	
	return sdPlane(p);
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
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdBox(uv - vec3(10.0), vec3(10.0, 5.0, 5.0));
	//_Result[id.x + id.y * _Width + id.z * _Width * _Height] = -sdSphere(uv - vec3(10.0), 10.0);

}

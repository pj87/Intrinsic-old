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
//------------------------------------------------------------------------

float mapScaled(vec3 p, vec4 c)
{
	//return map2(p/0.0025f, c) * 0.0025f;
	//return map3(p/0.0025f) * 0.0025f;
	//return sponge1(p / 0.0025) * 0.0025;
	//return sponge2(p / 0.0025) * 0.0025;
	
	return cathedral(p/0.000025f) * 0.000025f;
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

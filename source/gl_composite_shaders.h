#ifndef RME_GL_COMPOSITE_SHADERS_H_
#define RME_GL_COMPOSITE_SHADERS_H_
// Generated from libretro/glsl-shaders: MDAPT v2.8 (Sp00kyFox), ScaleFX-Hybrid (Sp00kyFox) and sharpsmoother (Sp00kyFox).
// Each fragment shader shares a common vertex stage that emits TEX0/COL0.

static const char* const compositeVertexSrc = R"GLSL(
#version 330
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
uniform mat4 uProjection;
out vec4 TEX0;
out vec4 COL0;
void main(){
	gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
	TEX0 = vec4(aUV, 0.0, 1.0);
	COL0 = aColor;
}
)GLSL";

static const char* const compositeMdapt0Src = R"GLSL(#version 330
#define FRAGMENT
/*
   Merge Dithering and Pseudo Transparency Shader v2.8 - Pass 0
   by Sp00kyFox, 2014

   Neighbor analysis via color metric and dot product of the difference vectors.

*/

// Parameter lines go here:
#pragma parameter MODE "MDAPT Monochrome Analysis"	0.0 0.0 1.0 1.0
#pragma parameter PWR  "MDAPT Color Metric Exp"		2.0 0.0 10.0 0.1

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying
#define COMPAT_ATTRIBUTE attribute
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

vec4 _oPosition1;
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;

// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
// All parameter floats need to have COMPAT_PRECISION in front of them
uniform COMPAT_PRECISION float MODE;
uniform COMPAT_PRECISION float PWR;
#else
#define MODE 0.0
#define PWR 2.0
#endif

#define dotfix(x,y) clamp(dot(x,y), 0.0, 1.0)	// NVIDIA Fix
#define TEX(dx,dy) COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw)

// Reference: http://www.compuphase.com/cmetric.htm
float eq(vec3 A, vec3 B)
{
	vec3 diff = A-B;
	float  ravg = (A.x + B.x) * 0.5;

	diff *= diff * vec3(2.0 + ravg, 4.0, 3.0 - ravg);

	return pow( smoothstep(3.0, 0.0, sqrt(diff.x + diff.y + diff.z)), PWR );
}

float and(float a, float b, float c, float d, float e, float f){
	return min(a, min(b, min(c, min(d, min(e,f)))));
}

void main()
{
	/*
		  U
		L C R
		  D
	*/

	vec3 C = TEX( 0., 0.).xyz;
	vec3 L = TEX(-1., 0.).xyz;
	vec3 R = TEX( 1., 0.).xyz;
	vec3 U = TEX( 0.,-1.).xyz;
	vec3 D = TEX( 0., 1.).xyz;


	vec3 res = vec3(0.0);

	if(MODE > 0.5){
		res.x = float((L == R) && (C != L));
		res.y = float((U == D) && (C != U));
		res.z = float(bool(res.x) && bool(res.y) && (L == U));
	}
	else{
		vec3 dCL = normalize(C-L), dCR = normalize(C-R), dCD = normalize(C-D), dCU = normalize(C-U);

		res.x = dotfix(dCL, dCR) * eq(L,R);
		res.y = dotfix(dCU, dCD) * eq(U,D);
		res.z = and(res.x, res.y, dotfix(dCL, dCU) * eq(L,U), dotfix(dCL, dCD) * eq(L,D), dotfix(dCR, dCU) * eq(R,U), dotfix(dCR, dCD) * eq(R,D));
	}

   FragColor = vec4(res, 1.0);
}
#endif
)GLSL";

static const char* const compositeMdapt1Src = R"GLSL(#version 330
#define FRAGMENT
/*
   Merge Dithering and Pseudo Transparency Shader v2.8 - Pass 1
   by Sp00kyFox, 2014

   Preparing checkerboard patterns.

*/

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying
#define COMPAT_ATTRIBUTE attribute
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

vec4 _oPosition1;
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;

// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#define TEX(dx,dy) COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw)


float and(float a, float b){
	return min(a,b);
}

float and(float a, float b, float c){
	return min(a, min(b,c));
}

float or(float a, float b){
	return max(a,b);
}

float or(float a, float b, float c, float d, float e){
	return max(a, max(b, max(c, max(d,e))));
}



void main()
{
	/*
		UL U UR
		L  C R
		DL D DR
	*/

	vec3 C = TEX( 0., 0.).xyz;
	vec3 L = TEX(-1., 0.).xyz;
	vec3 R = TEX( 1., 0.).xyz;
	vec3 D = TEX( 0., 1.).xyz;
	vec3 U = TEX( 0.,-1.).xyz;

	float UL = TEX(-1.,-1.).z;
	float UR = TEX( 1.,-1.).z;
	float DL = TEX(-1., 1.).z;
	float DR = TEX( 1., 1.).z;

	// Checkerboard Pattern Completion
	float prCB = or(C.z,
		and(L.z, R.z, or(U.x, D.x)),
		and(U.z, D.z, or(L.y, R.y)),
		and(C.x, or(and(UL, UR), and(DL, DR))),
		and(C.y, or(and(UL, DL), and(UR, DR))));
   FragColor = vec4(C.x, prCB, 0.0, 0.0);
}
#endif
)GLSL";

static const char* const compositeMdapt2Src = R"GLSL(#version 330
#define FRAGMENT
/*
   Merge Dithering and Pseudo Transparency Shader v2.8 - Pass 2
   by Sp00kyFox, 2014

   Eliminating isolated detections.

*/

// Parameter lines go here:
#pragma parameter VL_LO "MDAPT VL LO Thresh" 1.25 0.0 10.0 0.05
#pragma parameter VL_HI "MDAPT VL HI Thresh" 1.75 0.0 10.0 0.05
#pragma parameter CB_LO "MDAPT CB LO Thresh" 5.25 0.0 25.0 0.05
#pragma parameter CB_HI "MDAPT CB HI Thresh" 5.75 0.0 25.0 0.05

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying
#define COMPAT_ATTRIBUTE attribute
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

vec4 _oPosition1;
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;

// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
// All parameter floats need to have COMPAT_PRECISION in front of them
uniform COMPAT_PRECISION float VL_LO;
uniform COMPAT_PRECISION float VL_HI;
uniform COMPAT_PRECISION float CB_LO;
uniform COMPAT_PRECISION float CB_HI;
#else
#define VL_LO 1.25
#define VL_HI 1.75
#define CB_LO 5.25
#define CB_HI 5.75
#endif

#define TEX(dx,dy) COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw)
#define and(x,y) min(x,y)
#define or(x,y)  max(x,y)

vec2 sigmoid(vec2 signal){
	return smoothstep(vec2(VL_LO, CB_LO), vec2(VL_HI, CB_HI), signal);
}

void main()
{
	/*
		NW  UUL U2 UUR NE
		ULL UL  U1 UR  URR
		L2  L1  C  R1  R2
		DLL DL  D1 DR  DRR
		SW  DDL D2 DDR SE
	*/

	vec2 C = TEX( 0., 0.).xy;


	vec2 hits = vec2(0.0);

	//phase 1
	vec2 L1 = TEX(-1., 0.).xy;
	vec2 R1 = TEX( 1., 0.).xy;
	vec2 U1 = TEX( 0.,-1.).xy;
	vec2 D1 = TEX( 0., 1.).xy;

	//phase 2
	vec2 L2 = and(TEX(-2., 0.).xy, L1);
	vec2 R2 = and(TEX( 2., 0.).xy, R1);
	vec2 U2 = and(TEX( 0.,-2.).xy, U1);
	vec2 D2 = and(TEX( 0., 2.).xy, D1);
	vec2 UL = and(TEX(-1.,-1.).xy, or(L1, U1));
	vec2 UR = and(TEX( 1.,-1.).xy, or(R1, U1));
	vec2 DL = and(TEX(-1., 1.).xy, or(L1, D1));
	vec2 DR = and(TEX( 1., 1.).xy, or(R1, D1));

	//phase 3
	vec2 ULL = and(TEX(-2.,-1.).xy, or(L2, UL));
	vec2 URR = and(TEX( 2.,-1.).xy, or(R2, UR));
	vec2 DRR = and(TEX( 2., 1.).xy, or(R2, DR));
	vec2 DLL = and(TEX(-2., 1.).xy, or(L2, DL));
	vec2 UUL = and(TEX(-1.,-2.).xy, or(U2, UL));
	vec2 UUR = and(TEX( 1.,-2.).xy, or(U2, UR));
	vec2 DDR = and(TEX( 1., 2.).xy, or(D2, DR));
	vec2 DDL = and(TEX(-1., 2.).xy, or(D2, DL));

	//phase 4
	hits += and(TEX(-2.,-2.).xy, or(UUL, ULL));
	hits += and(TEX( 2.,-2.).xy, or(UUR, URR));
	hits += and(TEX(-2., 2.).xy, or(DDL, DLL));
	hits += and(TEX( 2., 2.).xy, or(DDR, DRR));

	hits += (ULL + URR + DRR + DLL + L2 + R2) + vec2(0.0, 1.0) * (C + U1 + U2 + D1 + D2 + L1 + R1 + UL + UR + DL + DR + UUL + UUR + DDR + DDL);

   FragColor = vec4(C * sigmoid(hits), C);
}
#endif
)GLSL";

static const char* const compositeMdapt3Src = R"GLSL(#version 330
#define FRAGMENT
/*
   Merge Dithering and Pseudo Transparency Shader v2.8 - Pass 3
   by Sp00kyFox, 2014

   Backpropagation and checkerboard smoothing.

*/

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying
#define COMPAT_ATTRIBUTE attribute
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

vec4 _oPosition1;
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
uniform sampler2D OrigTexture;
#define Original OrigTexture
COMPAT_VARYING vec4 TEX0;

// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#define TEX(dx,dy)   COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw)
#define TEXt0(dx,dy) COMPAT_TEXTURE(Original, vTexCoord+vec2((dx),(dy))*SourceSize.zw)

bool eq(vec3 A, vec3 B){
	return (A == B);
}

float and(float a, float b){
	return min(a,b);
}

float or(float a, float b, float c, float d, float e, float f, float g, float h, float i){
	return max(a, max(b, max(c, max(d, max(e, max(f, max(g, max(h,i))))))));
}

vec2 and(vec2 a, vec2 b){
	return min(a,b);
}

vec2 or(vec2 a, vec2 b){
	return max(a,b);
}

vec2 or(vec2 a, vec2 b, vec2 c, vec2 d){
	return max(a, max(b, max(c,d)));
}

void main()
{
	/*
		UL U UR
		L  C R
		DL D DR
	*/

	vec4 C  = TEX( 0., 0.);		vec3 c  = TEXt0( 0., 0.).xyz;
	vec2 L  = TEX(-1., 0.).xy;	vec3 l  = TEXt0(-1., 0.).xyz;
	vec2 R  = TEX( 1., 0.).xy;	vec3 r  = TEXt0( 1., 0.).xyz;
	vec2 U  = TEX( 0.,-1.).xy;	vec3 u  = TEXt0( 0.,-1.).xyz;
	vec2 D  = TEX( 0., 1.).xy;	vec3 d  = TEXt0( 0., 1.).xyz;
	float UL = TEX(-1.,-1.).y;	vec3 ul = TEXt0(-1.,-1.).xyz;
	float UR = TEX( 1.,-1.).y;	vec3 ur = TEXt0( 1.,-1.).xyz;
	float DL = TEX(-1., 1.).y;	vec3 dl = TEXt0(-1., 1.).xyz;
	float DR = TEX( 1., 1.).y;	vec3 dr = TEXt0( 1., 1.).xyz;

	// Backpropagation
	C.xy = or(C.xy, and(C.zw, or(L, R, U, D)));

	// Checkerboard Smoothing
	C.y = or(C.y, min(U.y, float(eq(c,u))), min(D.y, float(eq(c,d))), min(L.y, float(eq(c,l))), min(R.y, float(eq(c,r))), min(UL, float(eq(c,ul))), min(UR, float(eq(c,ur))), min(DL, float(eq(c,dl))), min(DR, float(eq(c,dr))));

   FragColor = vec4(C);
}
#endif
)GLSL";

static const char* const compositeMdapt4Src = R"GLSL(#version 330
#define FRAGMENT
/*
   Merge Dithering and Pseudo Transparency Shader v2.8 - Pass 4
   by Sp00kyFox, 2014

   Blends pixels based on detected dithering patterns.

*/

// Parameter lines go here:
#pragma parameter VL    "MDAPT Vertical Lines"	0.0 0.0 1.0 1.0
#pragma parameter CB    "MDAPT Checkerboard"	1.0 0.0 1.0 1.0
#pragma parameter DEBUG "MDAPT Adjust View"	0.0 0.0 1.0 1.0
#pragma parameter linear_gamma "MDAPT Linear Gamma Blend"	0.0 0.0 1.0 1.0

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying
#define COMPAT_ATTRIBUTE attribute
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

vec4 _oPosition1;
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
uniform sampler2D OrigTexture;
#define Original OrigTexture
COMPAT_VARYING vec4 TEX0;

// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
uniform COMPAT_PRECISION float VL;
uniform COMPAT_PRECISION float CB;
uniform COMPAT_PRECISION float DEBUG;
uniform COMPAT_PRECISION float linear_gamma;
#else
#define VL 0.0
#define CB 1.0
#define DEBUG 0.0
#define linear_gamma 0.0
#endif

//#define TEX(dx,dy)   COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw)
//#define TEXt0(dx,dy) COMPAT_TEXTURE(Original, vTexCoord+vec2((dx),(dy))*SourceSize.zw)

vec4 TEX(float dx, float dy){
	if(linear_gamma > 0.5) return pow(COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw), vec4(2.2));
	else return COMPAT_TEXTURE(Source, vTexCoord+vec2((dx),(dy))*SourceSize.zw);
}

vec4 TEXt0(float dx, float dy){
	if(linear_gamma > 0.5) return pow(COMPAT_TEXTURE(Original, vTexCoord+vec2((dx),(dy))*SourceSize.zw), vec4(2.2));
	else return COMPAT_TEXTURE(Original, vTexCoord+vec2((dx),(dy))*SourceSize.zw);
}

bool eq(vec3 A, vec3 B){
	return (A == B);
}

float and(float a, float b){
	return min(a,b);
}

float or(float a, float b){
	return max(a,b);
}

float or(float a, float b, float c, float d, float e, float f, float g, float h, float i){
	return max(a, max(b, max(c, max(d, max(e, max(f, max(g, max(h,i))))))));
}

vec2 and(vec2 a, vec2 b){
	return min(a,b);
}

vec2 or(vec2 a, vec2 b){
	return max(a,b);
}

vec2 or(vec2 a, vec2 b, vec2 c, vec2 d){
	return max(a, max(b, max(c,d)));
}

void main()
{
	/*
		UL U UR
		L  C R
		DL D DR
	*/

	vec4 C = TEX( 0., 0.);		vec3 c = TEXt0( 0., 0.).xyz;
	vec2 L = TEX(-1., 0.).xy;	vec3 l = TEXt0(-1., 0.).xyz;
	vec2 R = TEX( 1., 0.).xy;	vec3 r = TEXt0( 1., 0.).xyz;
	vec2 U = TEX( 0.,-1.).xy;
	vec2 D = TEX( 0., 1.).xy;

	float  prVL = 0.0,		prCB = 0.0;
	vec3 fVL  = vec3(0.0),	fCB  = vec3(0.0);


	// Backpropagation
	C.xy = or(C.xy, and(C.zw, or(L.xy, R.xy, U.xy, D.xy)));


	if(VL > 0.5){
		float prSum = L.x + R.x;

		prVL = max(L.x, R.x);
		prVL = (prVL == 0.0) ? 1.0 : prSum/prVL;

		fVL  = (prVL*c + L.x*l + R.x*r)/(prVL + prSum);
		prVL = C.x;
	}


	if(CB > 0.5){
		vec3 u = TEXt0( 0.,-1.).xyz;
		vec3 d = TEXt0( 0., 1.).xyz;

		float eqCL = float(eq(c,l));
		float eqCR = float(eq(c,r));
		float eqCU = float(eq(c,u));
		float eqCD = float(eq(c,d));

		float prU = or(U.y, eqCU);
		float prD = or(D.y, eqCD);
		float prL = or(L.y, eqCL);
		float prR = or(R.y, eqCR);


		float prSum = prU  + prD  + prL  + prR;

		prCB = max(prL, max(prR, max(prU,prD)));
		prCB = (prCB == 0.0) ? 1.0 : prSum/prCB;

		//standard formula: C/2 + (L + R + D + U)/8
		fCB = (prCB*c + prU*u + prD*d + prL*l + prR*r)/(prCB + prSum);


		float UL = TEX(-1.,-1.).y;	vec3 ul = TEXt0(-1.,-1.).xyz;
		float UR = TEX( 1.,-1.).y;	vec3 ur = TEXt0( 1.,-1.).xyz;
		float DL = TEX(-1., 1.).y;	vec3 dl = TEXt0(-1., 1.).xyz;
		float DR = TEX( 1., 1.).y;	vec3 dr = TEXt0( 1., 1.).xyz;

		// Checkerboard Smoothing
		prCB = or(C.y, and(L.y, eqCL), and(R.y, eqCR), and(U.y, eqCU), and(D.y, eqCD), and(UL, float(eq(c,ul))), and(UR, float(eq(c,ur))), and(DL, float(eq(c,dl))), and(DR, float(eq(c,dr))));
	}


	if(DEBUG > 0.5)
		FragColor = vec4(prVL, prCB, 0.0, 0.0);

	vec4 final = (prCB >= prVL) ? vec4(mix(c, fCB, prCB), 1.0) : vec4(mix(c, fVL, prVL), 1.0);
	FragColor = (linear_gamma > 0.5) ? pow(final, vec4(1.0 / 2.2)) : final;
}
#endif
)GLSL";

static const char* const compositeScalefx0Src = R"GLSL(#version 330
#define FRAGMENT


/*
	ScaleFX - Pass 0
	by Sp00kyFox, 2017-03-01

Filter:	Nearest
Scale:	1x

ScaleFX is an edge interpolation algorithm specialized in pixel art. It was
originally intended as an improvement upon Scale3x but became a new filter in
its own right.
ScaleFX interpolates edges up to level 6 and makes smooth transitions between
different slopes. The filtered picture will only consist of colours present
in the original.

Pass 0 prepares metric data for the next pass.



Copyright (c) 2016 Sp00kyFox - ScaleFX@web.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;


uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    COL0 = COLOR;
    TEX0.xy = TexCoord.xy;
	float dx = SourceSize.z, dy = SourceSize.w;

	t1 = TEX0.xxxy + vec4(-dx, 0., dx, -dy);	// A, B, C
	t2 = TEX0.xxxy + vec4(-dx, 0., dx,   0.);	// D, E, F
}

#elif defined(FRAGMENT)

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out COMPAT_PRECISION vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;


// fragment compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

// Reference: http://www.compuphase.com/cmetric.htm
float dist(vec3 A, vec3 B)
{
	float r = 0.5 * (A.r + B.r);
	vec3 d = A - B;
	vec3 c = vec3(2. + r, 4., 3. - r);

	return sqrt(dot(c*d, d)) / 3.;
}

void main()
{
	float rmeDx = SourceSize.z, rmeDy = SourceSize.w;
	vec4 t1 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, -rmeDy);
	vec4 t2 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, 0.);

	/*	grid		metric

		A B C		x y z
		  E F		  o w
	*/

#ifdef GL_ES
#define TEX(x) COMPAT_TEXTURE(Source, x)
// read texels
	vec3 A = TEX(t1.xw).rgb;
	vec3 B = TEX(t1.yw).rgb;
	vec3 C = TEX(t1.zw).rgb;
	vec3 E = TEX(t2.yw).rgb;
	vec3 F = TEX(t2.zw).rgb;
#else
#define TEX(x, y) textureOffset(Source, vTexCoord, ivec2(x, y)).rgb
	// read texels
	vec3 A = TEX(-1,-1);
	vec3 B = TEX( 0,-1);
	vec3 C = TEX( 1,-1);
	vec3 E = TEX( 0, 0);
	vec3 F = TEX( 1, 0);
#endif
	// output
	FragColor = vec4(dist(E,A), dist(E,B), dist(E,C), dist(E,F));
} 
#endif
)GLSL";

static const char* const compositeScalefx1Src = R"GLSL(#version 330
#define FRAGMENT


/*
	ScaleFX - Pass 1
	by Sp00kyFox, 2017-03-01

Filter:	Nearest
Scale:	1x

ScaleFX is an edge interpolation algorithm specialized in pixel art. It was
originally intended as an improvement upon Scale3x but became a new filter in
its own right.
ScaleFX interpolates edges up to level 6 and makes smooth transitions between
different slopes. The filtered picture will only consist of colours present
in the original.

Pass 1 calculates the strength of interpolation candidates.



Copyright (c) 2016 Sp00kyFox - ScaleFX@web.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// Parameter lines go here:
#pragma parameter SFX_CLR "ScaleFX Threshold" 0.50 0.01 1.00 0.01
#pragma parameter SFX_SAA "ScaleFX Filter AA" 1.00 0.00 1.00 1.00

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;



uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
	gl_Position = MVPMatrix * VertexCoord;
	COL0 = COLOR;
	TEX0.xy = TexCoord.xy;
	float dx = SourceSize.z, dy = SourceSize.w;
    
	t1 = TEX0.xxxy + vec4(  -dx,   0., dx,  -dy);	// A, B, C
	t2 = TEX0.xxxy + vec4(  -dx,   0., dx,    0.);	// D, E, F
	t3 = TEX0.xxxy + vec4(  -dx,   0., dx,   dy);	// G, H, I
}

#elif defined(FRAGMENT)

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out COMPAT_PRECISION vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;



// fragment compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
uniform COMPAT_PRECISION float SFX_CLR;
uniform COMPAT_PRECISION float SFX_SAA;
#else
#define SFX_CLR 0.5
#define SFX_SAA 1.0
#endif

// corner strength
float str(float d, vec2 a, vec2 b){
	float diff = a.x - a.y;
	float wght1 = max(SFX_CLR - d, 0.) / SFX_CLR;
	float wght2 = clamp((1.-d) + (min(a.x, b.x) + a.x > min(a.y, b.y) + a.y ? diff : -diff), 0., 1.);
	return (SFX_SAA == 1. || 2.*d < a.x + a.y) ? (wght1 * wght2) * (a.x * a.y) : 0.;
}

void main()
{
	float rmeDx = SourceSize.z, rmeDy = SourceSize.w;
	vec4 t1 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, -rmeDy);
	vec4 t2 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, 0.);
	vec4 t3 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, rmeDy);

	/*	grid		metric		pattern

		A B		x y z		x y
		D E F		  o w		w z
		G H I
	*/

#ifdef GL_ES
#define TEX(x) COMPAT_TEXTURE(Source, x)

	// metric data
	vec4 A = TEX(t1.xw), B = TEX(t1.yw);
	vec4 D = TEX(t2.xw), E = TEX(t2.yw), F = TEX(t2.zw);
	vec4 G = TEX(t3.xw), H = TEX(t3.yw), I = TEX(t3.zw);
#else
#define TEX(x, y) textureOffset(Source, vTexCoord, ivec2(x, y))

	// metric data
	vec4 A = TEX(-1,-1), B = TEX( 0,-1);
	vec4 D = TEX(-1, 0), E = TEX( 0, 0), F = TEX( 1, 0);
	vec4 G = TEX(-1, 1), H = TEX( 0, 1), I = TEX( 1, 1);
#endif

	// corner strength
	vec4 res;
	res.x = str(D.z, vec2(D.w, E.y), vec2(A.w, D.y));
	res.y = str(F.x, vec2(E.w, E.y), vec2(B.w, F.y));
	res.z = str(H.z, vec2(E.w, H.y), vec2(H.w, I.y));
	res.w = str(H.x, vec2(D.w, H.y), vec2(G.w, G.y));
		
	FragColor = res;
} 
#endif
)GLSL";

static const char* const compositeScalefx2Src = R"GLSL(#version 330
#define FRAGMENT


/*
	ScaleFX - Pass 2
	by Sp00kyFox, 2017-03-01

Filter:	Nearest
Scale:	1x

ScaleFX is an edge interpolation algorithm specialized in pixel art. It was
originally intended as an improvement upon Scale3x but became a new filter in
its own right.
ScaleFX interpolates edges up to level 6 and makes smooth transitions between
different slopes. The filtered picture will only consist of colours present
in the original.

Pass 2 resolves ambiguous configurations of corner candidates at pixel junctions.



Copyright (c) 2016 Sp00kyFox - ScaleFX@web.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;



// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    COL0 = COLOR;
    TEX0.xy = TexCoord.xy;
	float dx = SourceSize.z, dy = SourceSize.w;
	
	t1 = TEX0.xxxy + vec4(  -dx,   0., dx,  -dy);	// A, B, C
	t2 = TEX0.xxxy + vec4(  -dx,   0., dx,    0.);	// D, E, F
	t3 = TEX0.xxxy + vec4(  -dx,   0., dx,   dy);	// G, H, I
}

#elif defined(FRAGMENT)

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out COMPAT_PRECISION vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
uniform sampler2D PassPrev2Texture;
COMPAT_VARYING vec4 TEX0;



// fragment compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

#define PassOutput0 PassPrev2Texture

#define LE(x, y) (1. - step(y, x))
#define GE(x, y) (1. - step(x, y))
#define LEQ(x, y) step(x, y)
#define GEQ(x, y) step(y, x)
#define NOT(x) (1. - (x))

// corner dominance at junctions
vec4 dom(vec3 x, vec3 y, vec3 z, vec3 w){
	return 2. * vec4(x.y, y.y, z.y, w.y) - (vec4(x.x, y.x, z.x, w.x) + vec4(x.z, y.z, z.z, w.z));
}

// necessary but not sufficient junction condition for orthogonal edges
float clear(vec2 crn, vec2 a, vec2 b){
	return (crn.x >= max(min(a.x, a.y), min(b.x, b.y))) && (crn.y >= max(min(a.x, b.y), min(b.x, a.y))) ? 1. : 0.;
}

void main()
{
	float rmeDx = SourceSize.z, rmeDy = SourceSize.w;
	vec4 t1 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, -rmeDy);
	vec4 t2 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, 0.);
	vec4 t3 = TEX0.xxxy + vec4(-rmeDx, 0., rmeDx, rmeDy);

	/*	grid		metric		pattern

		A B C		x y z		x y
		D E F		  o w		w z
		G H I
	*/

#ifdef GL_ES
	#define TEXm(x) COMPAT_TEXTURE(PassOutput0, x)
	#define TEXs(x) COMPAT_TEXTURE(Source, x)

	// metric data
	vec4 A = TEXm(t1.xw), B = TEXm(t1.yw);
	vec4 D = TEXm(t2.xw), E = TEXm(t2.yw), F = TEXm(t2.zw);
	vec4 G = TEXm(t3.xw), H = TEXm(t3.yw), I = TEXm(t3.zw);
	
	// strength data
	vec4 As = TEXs(t1.xw), Bs = TEXs(t1.yw), Cs = TEXs(t1.zw);
	vec4 Ds = TEXs(t2.xw), Es = TEXs(t2.yw), Fs = TEXs(t2.zw);
	vec4 Gs = TEXs(t3.xw), Hs = TEXs(t3.yw), Is = TEXs(t3.zw);
#else
	#define TEXm(x, y) textureOffset(PassOutput0, vTexCoord, ivec2(x, y))
	#define TEXs(x, y) textureOffset(Source, vTexCoord, ivec2(x, y))

	// metric data
	vec4 A = TEXm(-1,-1), B = TEXm( 0,-1);
	vec4 D = TEXm(-1, 0), E = TEXm( 0, 0), F = TEXm( 1, 0);
	vec4 G = TEXm(-1, 1), H = TEXm( 0, 1), I = TEXm( 1, 1);	

	// strength data
	vec4 As = TEXs(-1,-1), Bs = TEXs( 0,-1), Cs = TEXs( 1,-1);
	vec4 Ds = TEXs(-1, 0), Es = TEXs( 0, 0), Fs = TEXs( 1, 0);
	vec4 Gs = TEXs(-1, 1), Hs = TEXs( 0, 1), Is = TEXs( 1, 1);
#endif

	// strength & dominance junctions
	vec4 jSx = vec4(As.z, Bs.w, Es.x, Ds.y), jDx = dom(As.yzw, Bs.zwx, Es.wxy, Ds.xyz);
	vec4 jSy = vec4(Bs.z, Cs.w, Fs.x, Es.y), jDy = dom(Bs.yzw, Cs.zwx, Fs.wxy, Es.xyz);
	vec4 jSz = vec4(Es.z, Fs.w, Is.x, Hs.y), jDz = dom(Es.yzw, Fs.zwx, Is.wxy, Hs.xyz);
	vec4 jSw = vec4(Ds.z, Es.w, Hs.x, Gs.y), jDw = dom(Ds.yzw, Es.zwx, Hs.wxy, Gs.xyz);


	// majority vote for ambiguous dominance junctions
	vec4 zero4 = vec4(0.);
	vec4 jx = min(GE(jDx, zero4) * (LEQ(jDx.yzwx, zero4) * LEQ(jDx.wxyz, zero4) + GE(jDx + jDx.zwxy, jDx.yzwx + jDx.wxyz)), 1.);
	vec4 jy = min(GE(jDy, zero4) * (LEQ(jDy.yzwx, zero4) * LEQ(jDy.wxyz, zero4) + GE(jDy + jDy.zwxy, jDy.yzwx + jDy.wxyz)), 1.);
	vec4 jz = min(GE(jDz, zero4) * (LEQ(jDz.yzwx, zero4) * LEQ(jDz.wxyz, zero4) + GE(jDz + jDz.zwxy, jDz.yzwx + jDz.wxyz)), 1.);
	vec4 jw = min(GE(jDw, zero4) * (LEQ(jDw.yzwx, zero4) * LEQ(jDw.wxyz, zero4) + GE(jDw + jDw.zwxy, jDw.yzwx + jDw.wxyz)), 1.);


	// inject strength without creating new contradictions
	vec4 res;
	res.x = min(jx.z + NOT(jx.y) * NOT(jx.w) * GE(jSx.z, 0.) * (jx.x + GE(jSx.x + jSx.z, jSx.y + jSx.w)), 1.);
	res.y = min(jy.w + NOT(jy.z) * NOT(jy.x) * GE(jSy.w, 0.) * (jy.y + GE(jSy.y + jSy.w, jSy.x + jSy.z)), 1.);
	res.z = min(jz.x + NOT(jz.w) * NOT(jz.y) * GE(jSz.x, 0.) * (jz.z + GE(jSz.x + jSz.z, jSz.y + jSz.w)), 1.);
	res.w = min(jw.y + NOT(jw.x) * NOT(jw.z) * GE(jSw.y, 0.) * (jw.w + GE(jSw.y + jSw.w, jSw.x + jSw.z)), 1.);	


	// single pixel & end of line detection
	res = min(res * (vec4(jx.z, jy.w, jz.x, jw.y) + NOT(res.wxyz * res.yzwx)), 1.);


	// output

	vec4 clr;
	clr.x = clear(vec2(D.z, E.x), vec2(D.w, E.y), vec2(A.w, D.y));
	clr.y = clear(vec2(F.x, E.z), vec2(E.w, E.y), vec2(B.w, F.y));
	clr.z = clear(vec2(H.z, I.x), vec2(E.w, H.y), vec2(H.w, I.y));
	clr.w = clear(vec2(H.x, G.z), vec2(D.w, H.y), vec2(G.w, G.y));

	vec4 h = vec4(min(D.w, A.w), min(E.w, B.w), min(E.w, H.w), min(D.w, G.w));
	vec4 v = vec4(min(E.y, D.y), min(E.y, F.y), min(H.y, I.y), min(H.y, G.y));

	vec4 or   = GE(h + vec4(D.w, E.w, E.w, D.w), v + vec4(E.y, E.y, H.y, H.y));	// orientation
	vec4 hori = LE(h, v) * clr;	// horizontal edges
	vec4 vert = GE(h, v) * clr;	// vertical edges

	FragColor = (res + 2. * hori + 4. * vert + 8. * or) / 15.;
} 
#endif
)GLSL";

static const char* const compositeScalefx3Src = R"GLSL(#version 330
#define FRAGMENT


/*
	ScaleFX - Pass 3
	by Sp00kyFox, 2017-03-01

Filter:	Nearest
Scale:	1x

ScaleFX is an edge interpolation algorithm specialized in pixel art. It was
originally intended as an improvement upon Scale3x but became a new filter in
its own right.
ScaleFX interpolates edges up to level 6 and makes smooth transitions between
different slopes. The filtered picture will only consist of colours present
in the original.

Pass 3 determines which edge level is present and prepares tags for subpixel
output in the final pass.



Copyright (c) 2016 Sp00kyFox - ScaleFX@web.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// Parameter lines go here:
#pragma parameter SFX_SCN "ScaleFX Filter Corners" 1.0 0.0 1.0 1.0

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;




uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    COL0 = COLOR;
    TEX0.xy = TexCoord.xy;
	float dx = SourceSize.z, dy = SourceSize.w;
    
    t1 = TEX0.xxxy + vec4(-dx, -2.*dx, -3.*dx,     0.);	// D, D0, D1
	t2 = TEX0.xxxy + vec4( dx,  2.*dx,  3.*dx,     0.);	// F, F0, F1
	t3 = TEX0.xyyy + vec4(  0.,   -dy, -2.*dy, -3.*dy);	// B, B0, B1
	t4 = TEX0.xyyy + vec4(  0.,    dy,  2.*dy,  3.*dy);	// H, H0, H1
}

#elif defined(FRAGMENT)

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out COMPAT_PRECISION vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;




// fragment compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
uniform COMPAT_PRECISION float SFX_SCN;
#else
#define SFX_SCN 1.0
#endif

// extract first bool4 from float4 - corners
bvec4 loadCorn(vec4 x){
	return bvec4(floor(mod(x*15. + 0.5, 2.)));
}

// extract second bool4 from float4 - horizontal edges
bvec4 loadHori(vec4 x){
	return bvec4(floor(mod(x*7.5 + 0.25, 2.)));
}

// extract third bool4 from float4 - vertical edges
bvec4 loadVert(vec4 x){
	return bvec4(floor(mod(x*3.75 + 0.125, 2.)));
}

// extract fourth bool4 from float4 - orientation
bvec4 loadOr(vec4 x){
	return bvec4(floor(mod(x*1.875 + 0.0625, 2.)));
}

void main()
{
	float rmeDx = SourceSize.z, rmeDy = SourceSize.w;
	vec4 t1 = TEX0.xxxy + vec4(-rmeDx, -2.*rmeDx, -3.*rmeDx, 0.);
	vec4 t2 = TEX0.xxxy + vec4(rmeDx, 2.*rmeDx, 3.*rmeDx, 0.);
	vec4 t3 = TEX0.xyyy + vec4(0., -rmeDy, -2.*rmeDy, -3.*rmeDy);
	vec4 t4 = TEX0.xyyy + vec4(0., rmeDy, 2.*rmeDy, 3.*rmeDy);

	/*	grid		corners		mids		

		  B		x   y	  	  x
		D E F				w   y
		  H		w   z	  	  z
	*/
#ifdef GL_ES
#define TEX(x) COMPAT_TEXTURE(Source, x)

	// read data
	vec4 E = TEX(vTexCoord);
	vec4 D = TEX(t1.xw), D0 = TEX(t1.yw), D1 = TEX(t1.zw);
	vec4 F = TEX(t2.xw), F0 = TEX(t2.yw), F1 = TEX(t2.zw);
	vec4 B = TEX(t3.xy), B0 = TEX(t3.xz), B1 = TEX(t3.xw);
	vec4 H = TEX(t4.xy), H0 = TEX(t4.xz), H1 = TEX(t4.xw);
#else
#define TEX(x, y) textureOffset(Source, vTexCoord, ivec2(x, y))

	// read data
	vec4 E = TEX( 0, 0);
	vec4 D = TEX(-1, 0), D0 = TEX(-2, 0), D1 = TEX(-3, 0);
	vec4 F = TEX( 1, 0), F0 = TEX( 2, 0), F1 = TEX( 3, 0);
	vec4 B = TEX( 0,-1), B0 = TEX( 0,-2), B1 = TEX( 0,-3);
	vec4 H = TEX( 0, 1), H0 = TEX( 0, 2), H1 = TEX( 0, 3);
#endif
	// extract data
	bvec4 Ec = loadCorn(E), Eh = loadHori(E), Ev = loadVert(E), Eo = loadOr(E);
	bvec4 Dc = loadCorn(D),	Dh = loadHori(D), Do = loadOr(D), D0c = loadCorn(D0), D0h = loadHori(D0), D1h = loadHori(D1);
	bvec4 Fc = loadCorn(F),	Fh = loadHori(F), Fo = loadOr(F), F0c = loadCorn(F0), F0h = loadHori(F0), F1h = loadHori(F1);
	bvec4 Bc = loadCorn(B),	Bv = loadVert(B), Bo = loadOr(B), B0c = loadCorn(B0), B0v = loadVert(B0), B1v = loadVert(B1);
	bvec4 Hc = loadCorn(H),	Hv = loadVert(H), Ho = loadOr(H), H0c = loadCorn(H0), H0v = loadVert(H0), H1v = loadVert(H1);

	
	// lvl1 corners (hori, vert)
	bool lvl1x = Ec.x && (Dc.z || Bc.z || SFX_SCN == 1.);
	bool lvl1y = Ec.y && (Fc.w || Bc.w || SFX_SCN == 1.);
	bool lvl1z = Ec.z && (Fc.x || Hc.x || SFX_SCN == 1.);
	bool lvl1w = Ec.w && (Dc.y || Hc.y || SFX_SCN == 1.);

	// lvl2 mid (left, right / up, down)
	bvec2 lvl2x = bvec2((Ec.x && Eh.y) && Dc.z, (Ec.y && Eh.x) && Fc.w);
	bvec2 lvl2y = bvec2((Ec.y && Ev.z) && Bc.w, (Ec.z && Ev.y) && Hc.x);
	bvec2 lvl2z = bvec2((Ec.w && Eh.z) && Dc.y, (Ec.z && Eh.w) && Fc.x);
	bvec2 lvl2w = bvec2((Ec.x && Ev.w) && Bc.z, (Ec.w && Ev.x) && Hc.y);

	// lvl3 corners (hori, vert)
	bvec2 lvl3x = bvec2(lvl2x.y && (Dh.y && Dh.x) && Fh.z, lvl2w.y && (Bv.w && Bv.x) && Hv.z);
	bvec2 lvl3y = bvec2(lvl2x.x && (Fh.x && Fh.y) && Dh.w, lvl2y.y && (Bv.z && Bv.y) && Hv.w);
	bvec2 lvl3z = bvec2(lvl2z.x && (Fh.w && Fh.z) && Dh.x, lvl2y.x && (Hv.y && Hv.z) && Bv.x);
	bvec2 lvl3w = bvec2(lvl2z.y && (Dh.z && Dh.w) && Fh.y, lvl2w.x && (Hv.x && Hv.w) && Bv.y);

	// lvl4 corners (hori, vert)
	bvec2 lvl4x = bvec2((Dc.x && Dh.y && Eh.x && Eh.y && Fh.x && Fh.y) && (D0c.z && D0h.w), (Bc.x && Bv.w && Ev.x && Ev.w && Hv.x && Hv.w) && (B0c.z && B0v.y));
	bvec2 lvl4y = bvec2((Fc.y && Fh.x && Eh.y && Eh.x && Dh.y && Dh.x) && (F0c.w && F0h.z), (Bc.y && Bv.z && Ev.y && Ev.z && Hv.y && Hv.z) && (B0c.w && B0v.x));
	bvec2 lvl4z = bvec2((Fc.z && Fh.w && Eh.z && Eh.w && Dh.z && Dh.w) && (F0c.x && F0h.y), (Hc.z && Hv.y && Ev.z && Ev.y && Bv.z && Bv.y) && (H0c.x && H0v.w));
	bvec2 lvl4w = bvec2((Dc.w && Dh.z && Eh.w && Eh.z && Fh.w && Fh.z) && (D0c.y && D0h.x), (Hc.w && Hv.x && Ev.w && Ev.x && Bv.w && Bv.x) && (H0c.y && H0v.z));

	// lvl5 mid (left, right / up, down)
	bvec2 lvl5x = bvec2(lvl4x.x && (F0h.x && F0h.y) && (D1h.z && D1h.w), lvl4y.x && (D0h.y && D0h.x) && (F1h.w && F1h.z));
	bvec2 lvl5y = bvec2(lvl4y.y && (H0v.y && H0v.z) && (B1v.w && B1v.x), lvl4z.y && (B0v.z && B0v.y) && (H1v.x && H1v.w));
	bvec2 lvl5z = bvec2(lvl4w.x && (F0h.w && F0h.z) && (D1h.y && D1h.x), lvl4z.x && (D0h.z && D0h.w) && (F1h.x && F1h.y));
	bvec2 lvl5w = bvec2(lvl4x.y && (H0v.x && H0v.w) && (B1v.z && B1v.y), lvl4w.y && (B0v.w && B0v.x) && (H1v.y && H1v.z));

	// lvl6 corners (hori, vert)
	bvec2 lvl6x = bvec2(lvl5x.y && (D1h.y && D1h.x), lvl5w.y && (B1v.w && B1v.x));
	bvec2 lvl6y = bvec2(lvl5x.x && (F1h.x && F1h.y), lvl5y.y && (B1v.z && B1v.y));
	bvec2 lvl6z = bvec2(lvl5z.x && (F1h.w && F1h.z), lvl5y.x && (H1v.y && H1v.z));
	bvec2 lvl6w = bvec2(lvl5z.y && (D1h.z && D1h.w), lvl5w.x && (H1v.x && H1v.w));

	
	// subpixels - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0

	vec4 crn;
	crn.x = (lvl1x && Eo.x || lvl3x.x && Eo.y || lvl4x.x && Do.x || lvl6x.x && Fo.y) ? 5. : (lvl1x || lvl3x.y && !Eo.w || lvl4x.y && !Bo.x || lvl6x.y && !Ho.w) ? 1. : lvl3x.x ? 3. : lvl3x.y ? 7. : lvl4x.x ? 2. : lvl4x.y ? 6. : lvl6x.x ? 4. : lvl6x.y ? 8. : 0.;
	crn.y = (lvl1y && Eo.y || lvl3y.x && Eo.x || lvl4y.x && Fo.y || lvl6y.x && Do.x) ? 5. : (lvl1y || lvl3y.y && !Eo.z || lvl4y.y && !Bo.y || lvl6y.y && !Ho.z) ? 3. : lvl3y.x ? 1. : lvl3y.y ? 7. : lvl4y.x ? 4. : lvl4y.y ? 6. : lvl6y.x ? 2. : lvl6y.y ? 8. : 0.;
	crn.z = (lvl1z && Eo.z || lvl3z.x && Eo.w || lvl4z.x && Fo.z || lvl6z.x && Do.w) ? 7. : (lvl1z || lvl3z.y && !Eo.y || lvl4z.y && !Ho.z || lvl6z.y && !Bo.y) ? 3. : lvl3z.x ? 1. : lvl3z.y ? 5. : lvl4z.x ? 4. : lvl4z.y ? 8. : lvl6z.x ? 2. : lvl6z.y ? 6. : 0.;
	crn.w = (lvl1w && Eo.w || lvl3w.x && Eo.z || lvl4w.x && Do.w || lvl6w.x && Fo.z) ? 7. : (lvl1w || lvl3w.y && !Eo.x || lvl4w.y && !Ho.w || lvl6w.y && !Bo.x) ? 1. : lvl3w.x ? 3. : lvl3w.y ? 5. : lvl4w.x ? 2. : lvl4w.y ? 8. : lvl6w.x ? 4. : lvl6w.y ? 6. : 0.;

	vec4 mid;
	mid.x = (lvl2x.x &&  Eo.x || lvl2x.y &&  Eo.y || lvl5x.x &&  Do.x || lvl5x.y &&  Fo.y) ? 5. : lvl2x.x ? 1. : lvl2x.y ? 3. : lvl5x.x ? 2. : lvl5x.y ? 4. : (Ec.x && Dc.z && Ec.y && Fc.w) ? ( Eo.x ?  Eo.y ? 5. : 3. : 1.) : 0.;
	mid.y = (lvl2y.x && !Eo.y || lvl2y.y && !Eo.z || lvl5y.x && !Bo.y || lvl5y.y && !Ho.z) ? 3. : lvl2y.x ? 5. : lvl2y.y ? 7. : lvl5y.x ? 6. : lvl5y.y ? 8. : (Ec.y && Bc.w && Ec.z && Hc.x) ? (!Eo.y ? !Eo.z ? 3. : 7. : 5.) : 0.;
	mid.z = (lvl2z.x &&  Eo.w || lvl2z.y &&  Eo.z || lvl5z.x &&  Do.w || lvl5z.y &&  Fo.z) ? 7. : lvl2z.x ? 1. : lvl2z.y ? 3. : lvl5z.x ? 2. : lvl5z.y ? 4. : (Ec.z && Fc.x && Ec.w && Dc.y) ? ( Eo.z ?  Eo.w ? 7. : 1. : 3.) : 0.;
	mid.w = (lvl2w.x && !Eo.x || lvl2w.y && !Eo.w || lvl5w.x && !Bo.x || lvl5w.y && !Ho.w) ? 1. : lvl2w.x ? 5. : lvl2w.y ? 7. : lvl5w.x ? 6. : lvl5w.y ? 8. : (Ec.w && Hc.y && Ec.x && Bc.z) ? (!Eo.w ? !Eo.x ? 1. : 5. : 7.) : 0.;


	// ouput
	FragColor = (crn + 9. * mid) / 80.;
} 
#endif
)GLSL";

static const char* const compositeScalefx4Src = R"GLSL(#version 330
#define FRAGMENT


/*
	ScaleFX - Pass 4
	by Sp00kyFox, 2017-03-01

Filter:	Nearest
Scale:	3x

ScaleFX is an edge interpolation algorithm specialized in pixel art. It was
originally intended as an improvement upon Scale3x but became a new filter in
its own right.
ScaleFX interpolates edges up to level 6 and makes smooth transitions between
different slopes. The filtered picture will only consist of colours present
in the original.

Pass 4 outputs subpixels based on previously calculated tags.



Copyright (c) 2016 Sp00kyFox - ScaleFX@web.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// Parameter lines go here:
#pragma parameter SFX_RAA "ScaleFX rAA Sharpness" 2.0 0.0 10.0 0.05

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;

uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// vertex compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    COL0 = COLOR;
    TEX0.xy = TexCoord.xy;
}

#elif defined(FRAGMENT)

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out COMPAT_PRECISION vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
uniform sampler2D PassPrev5Texture;
COMPAT_VARYING vec4 TEX0;

// fragment compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define outsize vec4(OutputSize, 1.0 / OutputSize)
#define Original PassPrev5Texture

#ifdef PARAMETER_UNIFORM
// All parameter floats need to have COMPAT_PRECISION in front of them
uniform COMPAT_PRECISION float SFX_RAA;
#else
#define SFX_RAA 2.0
#endif

// extract corners
vec4 loadCrn(vec4 x){
	return floor(mod(x*80. + 0.5, 9.));
}

// extract mids
vec4 loadMid(vec4 x){
	return floor(mod(x*8.888888 + 0.055555, 9.));
}

vec3 res2x(vec3 pre2, vec3 pre1, vec3 px, vec3 pos1, vec3 pos2)
{
	vec3 t, m;
	mat4x3 pre = mat4x3(pre2, pre1,   px, pos1);
	mat4x3 pos = mat4x3(pre1,   px, pos1, pos2);
	mat4x3  df = pos - pre;
	
	m = mix(px, 1.-px, step(px, vec3(0.5)));	
	m = SFX_RAA * min(m, min(abs(df[1]), abs(df[2])));
	t = (7. * (df[1] + df[2]) - 3. * (df[0] + df[3])) / 16.;
	t = clamp(t, -m, m);
   
	return t;
}

void main()
{
	/*	grid		corners		mids

		  B		x   y	  	  x
		D E F				w   y
		  H		w   z	  	  z
	*/


	// read data
	vec4 E = COMPAT_TEXTURE(Source, vTexCoord);

	// determine subpixel
	vec2 fc = fract(vTexCoord * SourceSize.xy);
	vec2 fp = floor(3.0 * fc);
	
	// check adjacent pixels to prevent artifacts
	vec4 hn = COMPAT_TEXTURE(Source, vTexCoord + vec2(fp.x - 1., 0.) / SourceSize.xy);
	vec4 vn = COMPAT_TEXTURE(Source, vTexCoord + vec2(0., fp.y - 1.) / SourceSize.xy);

	// extract data
	vec4 crn = loadCrn(E), hc = loadCrn(hn), vc = loadCrn(vn);
	vec4 mid = loadMid(E), hm = loadMid(hn), vm = loadMid(vn);

	vec3 res = fp.y == 0. ? (fp.x == 0. ? vec3(crn.x, hc.y, vc.w) : fp.x == 1. ? vec3(mid.x, 0., vm.z) : vec3(crn.y, hc.x, vc.z)) : (fp.y == 1. ? (fp.x == 0. ? vec3(mid.w, hm.y, 0.) : fp.x == 1. ? vec3(0.) : vec3(mid.y, hm.w, 0.)) : (fp.x == 0. ? vec3(crn.w, hc.z, vc.x) : fp.x == 1. ? vec3(mid.z, 0., vm.x) : vec3(crn.z, hc.w, vc.y)));	
	

#define TEX(x, y) textureOffset(Original, vTexCoord, ivec2(x, y)).rgb

	// reverseAA
	vec3 E0 = TEX( 0, 0);
	vec3 B0 = TEX( 0,-1), B1 = TEX( 0,-2), H0 = TEX( 0, 1), H1 = TEX( 0, 2);
	vec3 D0 = TEX(-1, 0), D1 = TEX(-2, 0), F0 = TEX( 1, 0), F1 = TEX( 2, 0);

	// output coordinate - 0 = E0, 1 = D0, 2 = D1, 3 = F0, 4 = F1, 5 = B0, 6 = B1, 7 = H0, 8 = H1
	vec3 sfx = res.x == 1. ? D0 : res.x == 2. ? D1 : res.x == 3. ? F0 : res.x == 4. ? F1 : res.x == 5. ? B0 : res.x == 6. ? B1 : res.x == 7. ? H0 : H1;

	// rAA weight
	vec2 w = 2. * fc - 1.;
	w.x = res.y == 0. ? w.x : 0.;
	w.y = res.z == 0. ? w.y : 0.;

	// rAA filter
	vec3 t1 = res2x(D1, D0, E0, F0, F1);
	vec3 t2 = res2x(B1, B0, E0, H0, H1);

	vec3 a = min(min(min(min(B0,D0),E0),F0),H0);
	vec3 b = max(max(max(max(B0,D0),E0),F0),H0);
	vec3 raa = clamp(E0 + w.x*t1 + w.y*t2, a, b);

	// hybrid output
	FragColor = vec4((res.x != 0.) ? sfx : raa, 0.);	
} 
#endif
)GLSL";

static const char* const compositeSharpsmootherSrc = R"GLSL(#version 330
#define FRAGMENT
uniform sampler2D AlphaSource;
/*
   Sharpsmoother shader
   
   Copyright (C) 2005-2017 guest(r) - guest.r@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma parameter max_w   "Max filter weight"  0.10  0.00 0.20 0.01
#pragma parameter min_w   "Min filter weight" -0.07 -0.15 0.05 0.01
#pragma parameter smoot   "Smoothing strength" 0.55  0.00 1.50 0.01
#pragma parameter lumad   "Effects smoothing"  0.30  0.10 5.00 0.10
#pragma parameter mtric   "The metric we use"  0.70  0.10 2.00 0.10

#if defined(VERTEX)

#if __VERSION__ >= 130
#define COMPAT_VARYING out
#define COMPAT_ATTRIBUTE in
#define COMPAT_TEXTURE texture
#else
#define COMPAT_VARYING varying 
#define COMPAT_ATTRIBUTE attribute 
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

COMPAT_ATTRIBUTE vec4 VertexCoord;
COMPAT_ATTRIBUTE vec4 COLOR;
COMPAT_ATTRIBUTE vec4 TexCoord;
COMPAT_VARYING vec4 COL0;
COMPAT_VARYING vec4 TEX0;




vec4 _oPosition1; 
uniform mat4 MVPMatrix;
uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;

// compatibility #defines
#define vTexCoord TEX0.xy
#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

void main()
{
    gl_Position = MVPMatrix * VertexCoord;
    TEX0.xy = TexCoord.xy;
   float x = 1.0 * (1.0 / SourceSize.x);
   float y = 1.0 * (1.0 / SourceSize.y);
   vec2 dg1 = vec2( x, y);
   vec2 dg2 = vec2(-x, y);
   vec2 dx = vec2(x, 0.0);
   vec2 dy = vec2(0.0, y);
   t1 = vec4(vTexCoord.xy-dg1,vTexCoord.xy-dy);
   t2 = vec4(vTexCoord.xy-dg2,vTexCoord.xy+dx);
   t3 = vec4(vTexCoord.xy+dg1,vTexCoord.xy+dy);
   t4 = vec4(vTexCoord.xy+dg2,vTexCoord.xy-dx);
}

#elif defined(FRAGMENT)

#if __VERSION__ >= 130
#define COMPAT_VARYING in
#define COMPAT_TEXTURE texture
out vec4 FragColor;
#else
#define COMPAT_VARYING varying
#define FragColor gl_FragColor
#define COMPAT_TEXTURE texture2D
#endif

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define COMPAT_PRECISION mediump
#else
#define COMPAT_PRECISION
#endif

uniform COMPAT_PRECISION int FrameDirection;
uniform COMPAT_PRECISION int FrameCount;
uniform COMPAT_PRECISION vec2 OutputSize;
uniform COMPAT_PRECISION vec2 TextureSize;
uniform COMPAT_PRECISION vec2 InputSize;
uniform sampler2D Texture;
COMPAT_VARYING vec4 TEX0;




// compatibility #defines
#define Source Texture
#define vTexCoord TEX0.xy

#define SourceSize vec4(TextureSize, 1.0 / TextureSize) //either TextureSize or InputSize
#define OutSize vec4(OutputSize, 1.0 / OutputSize)

#ifdef PARAMETER_UNIFORM
uniform COMPAT_PRECISION float max_w;
uniform COMPAT_PRECISION float min_w;
uniform COMPAT_PRECISION float smoot;
uniform COMPAT_PRECISION float lumad;
uniform COMPAT_PRECISION float mtric;
#else 
#define max_w    0.10
#define min_w   -0.07
#define smoot    0.55
#define lumad    0.30
#define mtric    0.70
#endif

vec3 dt = vec3(1.0, 1.0, 1.0);


float wt(vec3 A, vec3 B)
{	
	return clamp(smoot - ((6.0+lumad)/pow(3.0,mtric))*pow(dot(pow(abs(A-B),vec3(1.0/mtric)),dt),mtric)/(dot(A+B,dt)+lumad), min_w, max_w);
}

void main()
{
	float rmeX = SourceSize.z, rmeY = SourceSize.w;
	vec2 rmeDg1 = vec2(rmeX, rmeY), rmeDg2 = vec2(-rmeX, rmeY);
	vec2 rmeTx = vec2(rmeX, 0.0), rmeTy = vec2(0.0, rmeY);
	vec4 t1 = vec4(vTexCoord.xy - rmeDg1, vTexCoord.xy - rmeTy);
	vec4 t2 = vec4(vTexCoord.xy - rmeDg2, vTexCoord.xy + rmeTx);
	vec4 t3 = vec4(vTexCoord.xy + rmeDg1, vTexCoord.xy + rmeTy);
	vec4 t4 = vec4(vTexCoord.xy + rmeDg2, vTexCoord.xy - rmeTx);

   vec3 c00 = COMPAT_TEXTURE(Source, t1.xy).xyz; 
   vec3 c10 = COMPAT_TEXTURE(Source, t1.zw).xyz; 
   vec3 c20 = COMPAT_TEXTURE(Source, t2.xy).xyz; 
   vec3 c01 = COMPAT_TEXTURE(Source, t4.zw).xyz; 
   vec3 c11 = COMPAT_TEXTURE(Source, vTexCoord.xy).xyz; 
   vec3 c21 = COMPAT_TEXTURE(Source, t2.zw).xyz; 
   vec3 c02 = COMPAT_TEXTURE(Source, t4.xy).xyz; 
   vec3 c12 = COMPAT_TEXTURE(Source, t3.zw).xyz; 
   vec3 c22 = COMPAT_TEXTURE(Source, t3.xy).xyz;
       
   float w10 = wt(c11,c10);
   float w21 = wt(c11,c21);
   float w12 = wt(c11,c12);
   float w01 = wt(c11,c01);
   float w00 = wt(c11,c00)*0.75;
   float w22 = wt(c11,c22)*0.75;
   float w20 = wt(c11,c20)*0.75;
   float w02 = wt(c11,c02)*0.75;

   FragColor = vec4(w10*c10+w21*c21+w12*c12+w01*c01+w00*c00+w22*c22+w20*c20+w02*c02+(1.0-w10-w21-w12-w01-w00-w22-w20-w02)*c11, texture(AlphaSource, vTexCoord).a);
} 
#endif
)GLSL";

static const int compositePassCount = 11;
static const char* const compositePassSrc[compositePassCount] = {
	compositeMdapt0Src,
	compositeMdapt1Src,
	compositeMdapt2Src,
	compositeMdapt3Src,
	compositeMdapt4Src,
	compositeScalefx0Src,
	compositeScalefx1Src,
	compositeScalefx2Src,
	compositeScalefx3Src,
	compositeScalefx4Src,
	compositeSharpsmootherSrc,
};

#endif

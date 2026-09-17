#include <glad/glad.h>

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <Windows.h>
#elif defined(__APPLE__)
	#include <dlfcn.h>
#else
	#include <dlfcn.h>
#endif

#include "main.h"
#include "settings.h"
#include "gl_renderer.h"
#include "gl_composite_shaders.h"
#include <array>
#include <cstring>
#include <cmath>
#include <numbers>
#include <fstream>
#include <stb_truetype.h>

#ifdef _WIN32
static void* rmeGetGLProc(const char* name) {
	auto p = (void*)wglGetProcAddress(name);
	if (p == nullptr || p == (void*)0x1 || p == (void*)0x2 || p == (void*)0x3 || p == (void*)-1) {
		static HMODULE gl = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(gl, name);
	}
	return p;
}
#elif defined(__APPLE__)
static void* rmeGetGLProc(const char* name) {
	static void* lib = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
	return lib ? dlsym(lib, name) : nullptr;
}
#else
typedef void (*__GLXextFuncPtr)(void);
extern "C" __GLXextFuncPtr glXGetProcAddressARB(const unsigned char*);
static void* rmeGetGLProc(const char* name) {
	void* p = (void*)glXGetProcAddressARB((const GLubyte*)name);
	if (!p) {
		static void* lib = dlopen("libGL.so.1", RTLD_LAZY);
		if (lib) {
			p = dlsym(lib, name);
		}
	}
	return p;
}
#endif

std::vector<GLRenderer*> GLRenderer::s_instances;

static const char* const vertSrc = R"(
#version 330
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
uniform mat4 uProjection;
out vec2 vUV;
out vec4 vColor;
void main(){
	gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
	vUV = aUV;
	vColor = aColor;
}
)";

static const char* const fragSrc = R"(  
#version 330  
in vec2 vUV;  
in vec4 vColor;  
uniform sampler2D uTexture;  
uniform int uStipple;  
out vec4 FragColor;  
void main() {  
    if (uStipple != 0) {  
        float p = gl_FragCoord.x + gl_FragCoord.y;  
        if (mod(p, 4.0) < 2.0) discard;  
    }  
    FragColor = texture(uTexture, vUV) * vColor;  
}  
)";

static const char* const fragRetroSrc = R"(
#version 330
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform vec2 uTexSize;
uniform float uCellSize;
out vec4 FragColor;

float rLum(vec3 c) {
	return dot(c, vec3(0.299, 0.587, 0.114));
}

float rSmooth(float e0, float e1, float x) {
	float t = clamp((x - e0) / max(e1 - e0, 1e-5), 0.0, 1.0);
	return t * t * (3.0 - 2.0 * t);
}

void main() {
	// No magnification: plain nearest sampling (same look as the plain FBO blit)
	if (uCellSize <= 1.0f) {
		FragColor = texture(uTexture, vUV) * vColor;
		return;
	}

	ivec2 lim = ivec2(uTexSize) - 1;
	int cs = max(1, int(uCellSize + 0.5f));
	vec2 p = gl_FragCoord.xy / float(cs);
	ivec2 c = ivec2(int(floor(p.x)), int(floor(p.y)));
	vec2 f = p - vec2(c);
	// The scene is already zoomed inside the FBO (1 map unit = uCellSize texels),
	// so each cell's base TEXEL is c * uCellSize, not the cell index itself.
	ivec2 base = c * cs;

	vec4 s00 = texelFetch(uTexture, clamp(base, ivec2(0, 0), lim), 0);
	vec4 s10 = texelFetch(uTexture, clamp(base + ivec2(cs, 0), ivec2(0, 0), lim), 0);
	vec4 s01 = texelFetch(uTexture, clamp(base + ivec2(0, cs), ivec2(0, 0), lim), 0);
	vec4 s11 = texelFetch(uTexture, clamp(base + ivec2(cs, cs), ivec2(0, 0), lim), 0);
	s00.rgb *= s00.a;
	s10.rgb *= s10.a;
	s01.rgb *= s01.a;
	s11.rgb *= s11.a;

	vec4 cL = texelFetch(uTexture, clamp(base + ivec2(-cs, 0), ivec2(0, 0), lim), 0);
	vec4 cR = texelFetch(uTexture, clamp(base + ivec2( cs, 0), ivec2(0, 0), lim), 0);
	vec4 cU = texelFetch(uTexture, clamp(base + ivec2(0, -cs), ivec2(0, 0), lim), 0);
	vec4 cD = texelFetch(uTexture, clamp(base + ivec2(0, cs), ivec2(0, 0), lim), 0);
	cL.rgb *= cL.a;
	cR.rgb *= cR.a;
	cU.rgb *= cU.a;
	cD.rgb *= cD.a;

	// Edge-gated blend: only soften borders where pixels actually differ
	float l00 = rLum(s00.rgb);
	float maxDiff = max(max(abs(l00 - rLum(cL.rgb)), abs(l00 - rLum(cR.rgb))),
	                    max(abs(l00 - rLum(cU.rgb)), abs(l00 - rLum(cD.rgb))));
	float edgeGain = rSmooth(0.02, 0.12, maxDiff);

	// Blend only in the outer fringe of each cell so the apparent border stays
	// glued to the pick grid (perceived edge = real grid line)
	float interior = min(min(f.x, 1.0 - f.x), min(f.y, 1.0 - f.y));
	float border = 1.0 - rSmooth(0.30, 0.50, clamp(interior, 0.0, 0.5));

	float mixAmt = 0.6f * edgeGain * border;

	vec4 bil = s00 * (1.0 - f.x) * (1.0 - f.y)
	         + s10 * f.x * (1.0 - f.y)
	         + s01 * (1.0 - f.x) * f.y
	         + s11 * f.x * f.y;
	vec4 outC = mix(s00, bil, mixAmt);
	outC.rgb = outC.a > 1e-4 ? outC.rgb / outC.a : vec3(0.0);
	FragColor = outC * vColor;
}
)";

// xBRZ pixel-art scaler used when the map is zoomed in. It operates on the FBO
// scene rebuilt as logical sprite-pixels: each sprite pixel occupies `cs`
// texels, so the neighbourhood is sampled at `base + offset * cs` and the
// intra-pixel fraction picks the 4x4 (xBRZ) output pattern.
// Port: Zenju's xBRZ (4x), via the libretro 4xbrz shader.
static const char* const fragPixelArtSrc = R"(
#version 330
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform vec2 uTexSize;
uniform int uSourceCellSize;
uniform float uOutputCellSize;
out vec4 FragColor;

vec4 texS(ivec2 p) {
	p = clamp(p, ivec2(0), ivec2(uTexSize) - ivec2(1));
	return texelFetch(uTexture, p, 0);
}

// xBRZ 4x. Zenju's xBRZ, as ported to GLSL by the libretro project
// (xbrz/shaders/4xbrz.glsl). It evaluates the 16 sub-pixels of the 4x output
// block and `f` (the position inside the current sprite pixel, in output space)
// picks the blended result. Input mapping (5x5, centre = index 0):
//   20|21|22|23|24
//   19|06|07|08|09
//   18|05|00|01|10
//   17|04|03|02|11
//   16|15|14|13|12
float xbrzReduce(vec3 color) {
	return dot(color, vec3(65536.0, 256.0, 1.0));
}

float xbrzDist(vec3 a, vec3 b) {
	const vec3 w = vec3(0.2627, 0.6780, 0.0593);
	const float scaleB = 0.5 / (1.0 - w.b);
	const float scaleR = 0.5 / (1.0 - w.r);
	vec3 diff = a - b;
	float Y = dot(diff, w);
	float Cb = scaleB * (diff.b - Y);
	float Cr = scaleR * (diff.r - Y);
	return sqrt(Y * Y + Cb * Cb + Cr * Cr);
}

bool xbrzEq(vec3 a, vec3 b) {
	return xbrzDist(a, b) < (30.0 / 255.0);
}

vec3 xbrzTap(ivec2 base, int cs, int x, int y) {
	return texS(base + ivec2(cs * x, cs * y)).rgb;
}

vec3 xbrzScale(ivec2 base, int cs, vec2 f) {
	const int BLEND_NONE = 0;
	const int BLEND_NORMAL = 1;
	const int BLEND_DOMINANT = 2;
	const float STEEP = 2.2;
	const float DOMINANT = 3.6;

	vec3 src[25];
	src[ 0] = xbrzTap(base, cs,  0,  0);
	src[ 1] = xbrzTap(base, cs,  1,  0);
	src[ 2] = xbrzTap(base, cs,  1,  1);
	src[ 3] = xbrzTap(base, cs,  0,  1);
	src[ 4] = xbrzTap(base, cs, -1,  1);
	src[ 5] = xbrzTap(base, cs, -1,  0);
	src[ 6] = xbrzTap(base, cs, -1, -1);
	src[ 7] = xbrzTap(base, cs,  0, -1);
	src[ 8] = xbrzTap(base, cs,  1, -1);
	src[ 9] = xbrzTap(base, cs,  2, -1);
	src[10] = xbrzTap(base, cs,  2,  0);
	src[11] = xbrzTap(base, cs,  2,  1);
	src[13] = xbrzTap(base, cs,  1,  2);
	src[14] = xbrzTap(base, cs,  0,  2);
	src[15] = xbrzTap(base, cs, -1,  2);
	src[17] = xbrzTap(base, cs, -2,  1);
	src[18] = xbrzTap(base, cs, -2,  0);
	src[19] = xbrzTap(base, cs, -2, -1);
	src[21] = xbrzTap(base, cs, -1, -2);
	src[22] = xbrzTap(base, cs,  0, -2);
	src[23] = xbrzTap(base, cs,  1, -2);

	float v[9];
	v[0] = xbrzReduce(src[0]);
	v[1] = xbrzReduce(src[1]);
	v[2] = xbrzReduce(src[2]);
	v[3] = xbrzReduce(src[3]);
	v[4] = xbrzReduce(src[4]);
	v[5] = xbrzReduce(src[5]);
	v[6] = xbrzReduce(src[6]);
	v[7] = xbrzReduce(src[7]);
	v[8] = xbrzReduce(src[8]);

	ivec4 blendResult = ivec4(BLEND_NONE);

	// Preprocess the four corners around the centre pixel.
	if (!((v[0] == v[1] && v[3] == v[2]) || (v[0] == v[3] && v[1] == v[2]))) {
		float dist_03_01 = xbrzDist(src[4], src[0]) + xbrzDist(src[0], src[8]) + xbrzDist(src[14], src[2]) + xbrzDist(src[2], src[10]) + (4.0 * xbrzDist(src[3], src[1]));
		float dist_00_02 = xbrzDist(src[5], src[3]) + xbrzDist(src[3], src[13]) + xbrzDist(src[7], src[1]) + xbrzDist(src[1], src[11]) + (4.0 * xbrzDist(src[0], src[2]));
		bool dominantGradient = (DOMINANT * dist_03_01) < dist_00_02;
		blendResult[2] = ((dist_03_01 < dist_00_02) && (v[0] != v[1]) && (v[0] != v[3])) ? (dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}

	if (!((v[5] == v[0] && v[4] == v[3]) || (v[5] == v[4] && v[0] == v[3]))) {
		float dist_04_00 = xbrzDist(src[17], src[5]) + xbrzDist(src[5], src[7]) + xbrzDist(src[15], src[3]) + xbrzDist(src[3], src[1]) + (4.0 * xbrzDist(src[4], src[0]));
		float dist_05_03 = xbrzDist(src[18], src[4]) + xbrzDist(src[4], src[14]) + xbrzDist(src[6], src[0]) + xbrzDist(src[0], src[2]) + (4.0 * xbrzDist(src[5], src[3]));
		bool dominantGradient = (DOMINANT * dist_05_03) < dist_04_00;
		blendResult[3] = ((dist_04_00 > dist_05_03) && (v[0] != v[5]) && (v[0] != v[3])) ? (dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}

	if (!((v[7] == v[8] && v[0] == v[1]) || (v[7] == v[0] && v[8] == v[1]))) {
		float dist_00_08 = xbrzDist(src[5], src[7]) + xbrzDist(src[7], src[23]) + xbrzDist(src[3], src[1]) + xbrzDist(src[1], src[9]) + (4.0 * xbrzDist(src[0], src[8]));
		float dist_07_01 = xbrzDist(src[6], src[0]) + xbrzDist(src[0], src[2]) + xbrzDist(src[22], src[8]) + xbrzDist(src[8], src[10]) + (4.0 * xbrzDist(src[7], src[1]));
		bool dominantGradient = (DOMINANT * dist_07_01) < dist_00_08;
		blendResult[1] = ((dist_00_08 > dist_07_01) && (v[0] != v[7]) && (v[0] != v[1])) ? (dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}

	if (!((v[6] == v[7] && v[5] == v[0]) || (v[6] == v[5] && v[7] == v[0]))) {
		float dist_05_07 = xbrzDist(src[18], src[6]) + xbrzDist(src[6], src[22]) + xbrzDist(src[4], src[0]) + xbrzDist(src[0], src[8]) + (4.0 * xbrzDist(src[5], src[7]));
		float dist_06_00 = xbrzDist(src[19], src[5]) + xbrzDist(src[5], src[3]) + xbrzDist(src[21], src[7]) + xbrzDist(src[7], src[1]) + (4.0 * xbrzDist(src[6], src[0]));
		bool dominantGradient = (DOMINANT * dist_05_07) < dist_06_00;
		blendResult[0] = ((dist_05_07 < dist_06_00) && (v[0] != v[5]) && (v[0] != v[7])) ? (dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
	}

	vec3 dst[16];
	dst[ 0] = src[0]; dst[ 1] = src[0]; dst[ 2] = src[0]; dst[ 3] = src[0];
	dst[ 4] = src[0]; dst[ 5] = src[0]; dst[ 6] = src[0]; dst[ 7] = src[0];
	dst[ 8] = src[0]; dst[ 9] = src[0]; dst[10] = src[0]; dst[11] = src[0];
	dst[12] = src[0]; dst[13] = src[0]; dst[14] = src[0]; dst[15] = src[0];

	if (any(notEqual(blendResult, ivec4(BLEND_NONE)))) {
		float dist_01_04;
		float dist_03_08;
		bool haveShallowLine;
		bool haveSteepLine;
		bool needBlend;
		bool doLineBlend;
		vec3 blendPix;

		// Corner (1, 1)
		dist_01_04 = xbrzDist(src[1], src[4]);
		dist_03_08 = xbrzDist(src[3], src[8]);
		haveShallowLine = (STEEP * dist_01_04 <= dist_03_08) && (v[0] != v[4]) && (v[5] != v[4]);
		haveSteepLine   = (STEEP * dist_03_08 <= dist_01_04) && (v[0] != v[8]) && (v[7] != v[8]);
		needBlend = (blendResult[2] != BLEND_NONE);
		doLineBlend = (blendResult[2] >= BLEND_DOMINANT ||
			!((blendResult[1] != BLEND_NONE && !xbrzEq(src[0], src[4])) ||
			  (blendResult[3] != BLEND_NONE && !xbrzEq(src[0], src[8])) ||
			  (xbrzEq(src[4], src[3]) && xbrzEq(src[3], src[2]) && xbrzEq(src[2], src[1]) && xbrzEq(src[1], src[8]) && !xbrzEq(src[0], src[2]))));

		blendPix = (xbrzDist(src[0], src[1]) <= xbrzDist(src[0], src[3])) ? src[1] : src[3];
		dst[ 2] = mix(dst[ 2], blendPix, (needBlend && doLineBlend) ? (haveShallowLine ? (haveSteepLine ? 1.0 / 3.0 : 0.25) : (haveSteepLine ? 0.25 : 0.00)) : 0.00);
		dst[ 9] = mix(dst[ 9], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[10] = mix(dst[10], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.75 : 0.00);
		dst[11] = mix(dst[11], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.00 : ((haveShallowLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[12] = mix(dst[12], blendPix, (needBlend) ? ((doLineBlend) ? 1.00 : 0.6848532563) : 0.00);
		dst[13] = mix(dst[13], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.00 : ((haveSteepLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[14] = mix(dst[14], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.75 : 0.00);
		dst[15] = mix(dst[15], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);

		// Corner (1, 0)
		dist_01_04 = xbrzDist(src[7], src[2]);
		dist_03_08 = xbrzDist(src[1], src[6]);
		haveShallowLine = (STEEP * dist_01_04 <= dist_03_08) && (v[0] != v[2]) && (v[3] != v[2]);
		haveSteepLine   = (STEEP * dist_03_08 <= dist_01_04) && (v[0] != v[6]) && (v[5] != v[6]);
		needBlend = (blendResult[1] != BLEND_NONE);
		doLineBlend = (blendResult[1] >= BLEND_DOMINANT ||
			!((blendResult[0] != BLEND_NONE && !xbrzEq(src[0], src[2])) ||
			  (blendResult[2] != BLEND_NONE && !xbrzEq(src[0], src[6])) ||
			  (xbrzEq(src[2], src[1]) && xbrzEq(src[1], src[8]) && xbrzEq(src[8], src[7]) && xbrzEq(src[7], src[6]) && !xbrzEq(src[0], src[8]))));

		blendPix = (xbrzDist(src[0], src[7]) <= xbrzDist(src[0], src[1])) ? src[7] : src[1];
		dst[ 1] = mix(dst[ 1], blendPix, (needBlend && doLineBlend) ? (haveShallowLine ? (haveSteepLine ? 1.0 / 3.0 : 0.25) : (haveSteepLine ? 0.25 : 0.00)) : 0.00);
		dst[ 6] = mix(dst[ 6], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[ 7] = mix(dst[ 7], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.75 : 0.00);
		dst[ 8] = mix(dst[ 8], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.00 : ((haveShallowLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[ 9] = mix(dst[ 9], blendPix, (needBlend) ? ((doLineBlend) ? 1.00 : 0.6848532563) : 0.00);
		dst[10] = mix(dst[10], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.00 : ((haveSteepLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[11] = mix(dst[11], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.75 : 0.00);
		dst[12] = mix(dst[12], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);

		// Corner (0, 0)
		dist_01_04 = xbrzDist(src[5], src[8]);
		dist_03_08 = xbrzDist(src[7], src[4]);
		haveShallowLine = (STEEP * dist_01_04 <= dist_03_08) && (v[0] != v[8]) && (v[1] != v[8]);
		haveSteepLine   = (STEEP * dist_03_08 <= dist_01_04) && (v[0] != v[4]) && (v[3] != v[4]);
		needBlend = (blendResult[0] != BLEND_NONE);
		doLineBlend = (blendResult[0] >= BLEND_DOMINANT ||
			!((blendResult[3] != BLEND_NONE && !xbrzEq(src[0], src[8])) ||
			  (blendResult[1] != BLEND_NONE && !xbrzEq(src[0], src[4])) ||
			  (xbrzEq(src[8], src[7]) && xbrzEq(src[7], src[6]) && xbrzEq(src[6], src[5]) && xbrzEq(src[5], src[4]) && !xbrzEq(src[0], src[6]))));

		blendPix = (xbrzDist(src[0], src[5]) <= xbrzDist(src[0], src[7])) ? src[5] : src[7];
		dst[ 0] = mix(dst[ 0], blendPix, (needBlend && doLineBlend) ? (haveShallowLine ? (haveSteepLine ? 1.0 / 3.0 : 0.25) : (haveSteepLine ? 0.25 : 0.00)) : 0.00);
		dst[15] = mix(dst[15], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[ 4] = mix(dst[ 4], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.75 : 0.00);
		dst[ 5] = mix(dst[ 5], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.00 : ((haveShallowLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[ 6] = mix(dst[ 6], blendPix, (needBlend) ? ((doLineBlend) ? 1.00 : 0.6848532563) : 0.00);
		dst[ 7] = mix(dst[ 7], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.00 : ((haveSteepLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[ 8] = mix(dst[ 8], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.75 : 0.00);
		dst[ 9] = mix(dst[ 9], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);

		// Corner (0, 1)
		dist_01_04 = xbrzDist(src[3], src[6]);
		dist_03_08 = xbrzDist(src[5], src[2]);
		haveShallowLine = (STEEP * dist_01_04 <= dist_03_08) && (v[0] != v[6]) && (v[7] != v[6]);
		haveSteepLine   = (STEEP * dist_03_08 <= dist_01_04) && (v[0] != v[2]) && (v[1] != v[2]);
		needBlend = (blendResult[3] != BLEND_NONE);
		doLineBlend = (blendResult[3] >= BLEND_DOMINANT ||
			!((blendResult[2] != BLEND_NONE && !xbrzEq(src[0], src[6])) ||
			  (blendResult[0] != BLEND_NONE && !xbrzEq(src[0], src[2])) ||
			  (xbrzEq(src[6], src[5]) && xbrzEq(src[5], src[4]) && xbrzEq(src[4], src[3]) && xbrzEq(src[3], src[2]) && !xbrzEq(src[0], src[4]))));

		blendPix = (xbrzDist(src[0], src[3]) <= xbrzDist(src[0], src[5])) ? src[3] : src[5];
		dst[ 3] = mix(dst[ 3], blendPix, (needBlend && doLineBlend) ? (haveShallowLine ? (haveSteepLine ? 1.0 / 3.0 : 0.25) : (haveSteepLine ? 0.25 : 0.00)) : 0.00);
		dst[12] = mix(dst[12], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.25 : 0.00);
		dst[13] = mix(dst[13], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.75 : 0.00);
		dst[14] = mix(dst[14], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.00 : ((haveShallowLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[15] = mix(dst[15], blendPix, (needBlend) ? ((doLineBlend) ? 1.00 : 0.6848532563) : 0.00);
		dst[ 4] = mix(dst[ 4], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.00 : ((haveSteepLine) ? 0.75 : 0.50)) : 0.08677704501) : 0.00);
		dst[ 5] = mix(dst[ 5], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.75 : 0.00);
		dst[ 6] = mix(dst[ 6], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.25 : 0.00);
	}

	// 16 sub-pixels (4x4) selected by the intra-pixel position.
	return mix(
		mix(mix(mix(dst[ 6], dst[ 7], step(0.25, f.x)), mix(dst[ 8], dst[ 9], step(0.75, f.x)), step(0.50, f.x)),
			mix(mix(dst[ 5], dst[ 0], step(0.25, f.x)), mix(dst[ 1], dst[10], step(0.75, f.x)), step(0.50, f.x)), step(0.25, f.y)),
		mix(mix(mix(dst[ 4], dst[ 3], step(0.25, f.x)), mix(dst[ 2], dst[11], step(0.75, f.x)), step(0.50, f.x)),
			mix(mix(dst[15], dst[14], step(0.25, f.x)), mix(dst[13], dst[12], step(0.75, f.x)), step(0.50, f.x)), step(0.75, f.y)),
		step(0.50, f.y));
}

void main() {
	int sourceCs = max(1, uSourceCellSize);
	float outputCs = max(1.0f, uOutputCellSize);

	// Below one screen pixel per sprite pixel the scaler would have to
	// minify; leave that to the plain (nearest / smooth) blit.
	if (outputCs <= 1.0f) {
		FragColor = texture(uTexture, vUV) * vColor;
		return;
	}

	// `outputCs` maps screen pixels to sprite pixels, while `sourceCs` is the
	// texel density of one sprite pixel in the supersampled FBO. The
	// neighbourhood is therefore fetched at `base + offset * sourceCs`, but
	// the intra-pixel fraction that selects the pattern lives in output space.
	vec2 p = gl_FragCoord.xy / outputCs;
	ivec2 c = ivec2(int(floor(p.x)), int(floor(p.y)));
	vec2 f = p - vec2(c);
	ivec2 base = c * sourceCs;

	vec3 color = xbrzScale(base, sourceCs, f);
	float a = texS(base).a;
	FragColor = vec4(color, a) * vColor;
}
)";

struct RetroVertex {
	float x;
	float y;
	float u;
	float v;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

static GLuint rmeCompileProgramWithVertex(const char* vtxSrc, const char* fragSrc) {
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vtxSrc, nullptr);
	glCompileShader(vs);
	{
		GLint ok = 0;
		glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetShaderInfoLog(vs, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::rmeCompileProgram — vertex shader compile error: %s", log.data());
			glDeleteShader(vs);
			return 0;
		}
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragSrc, nullptr);
	glCompileShader(fs);
	{
		GLint ok = 0;
		glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetShaderInfoLog(fs, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::rmeCompileProgram — fragment shader compile error: %s", log.data());
			glDeleteShader(vs);
			glDeleteShader(fs);
			return 0;
		}
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	{
		GLint ok = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetProgramInfoLog(prog, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::rmeCompileProgram — program link error: %s", log.data());
			glDeleteProgram(prog);
			prog = 0;
		}
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}

static GLuint rmeCompileProgram(const char* fragSrc) {
	return rmeCompileProgramWithVertex(vertSrc, fragSrc);
}

void GLRenderer::initFontAtlas() {
	// Load TTF font
	const float fontSize = 14.0f;
	std::string fontPath;

#ifdef _WIN32
	fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
#elif defined(__APPLE__)
	fontPath = "/System/Library/Fonts/Helvetica.ttc";
#else
	fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif

	// Try bundled font first
	std::string bundledPath = "data/fonts/DejaVuSans.ttf";
	std::ifstream testFile(bundledPath, std::ios::binary);
	if (testFile.good()) {
		fontPath = bundledPath;
	}
	testFile.close();

	std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		// Fallback to old wxBitmap method
		initFontAtlasFallback();
		return;
	}

	auto fileSize = file.tellg();
	file.seekg(0);
	std::vector<uint8_t> ttfData(fileSize);
	file.read(reinterpret_cast<char*>(ttfData.data()), fileSize);
	file.close();

	stbtt_fontinfo stbFont;
	if (!stbtt_InitFont(&stbFont, ttfData.data(), 0)) {
		initFontAtlasFallback();
		return;
	}

	float scale = stbtt_ScaleForPixelHeight(&stbFont, fontSize);

	int ascent;
	int descent;
	int lineGap;
	stbtt_GetFontVMetrics(&stbFont, &ascent, &descent, &lineGap);
	font.ascent = ascent * scale;
	font.lineHeight = (ascent - descent + lineGap) * scale;

	// Bake glyphs into atlas
	const int texW = 512;
	const int texH = 512;
	std::vector<uint8_t> bitmap(texW * texH, 0);

	int penX = 1;
	int penY = 1;
	int rowH = 0;

	for (int i = 0; i < 96; i++) {
		const auto ch = 32 + i;
		int x0;
		int y0;
		int x1;
		int y1;
		stbtt_GetCodepointBitmapBox(&stbFont, ch, scale, scale, &x0, &y0, &x1, &y1);

		int gw = x1 - x0;
		int gh = y1 - y0;

		if (penX + gw + 1 >= texW) {
			penX = 1;
			penY += rowH + 1;
			rowH = 0;
		}

		if (penY + gh + 1 >= texH) {
			break; // atlas full
		}

		stbtt_MakeCodepointBitmap(&stbFont, &bitmap[penY * texW + penX], gw, gh, texW, scale, scale, ch);

		font.glyphs[i].u0 = static_cast<float>(penX) / texW;
		font.glyphs[i].v0 = static_cast<float>(penY) / texH;
		font.glyphs[i].u1 = static_cast<float>(penX + gw) / texW;
		font.glyphs[i].v1 = static_cast<float>(penY + gh) / texH;
		font.glyphs[i].xoff = static_cast<float>(x0);
		font.glyphs[i].yoff = static_cast<float>(y0);
		font.glyphs[i].w = static_cast<float>(gw);
		font.glyphs[i].h = static_cast<float>(gh);

		int advW;
		int lsb;
		stbtt_GetCodepointHMetrics(&stbFont, ch, &advW, &lsb);
		font.glyphs[i].advance = advW * scale;
		font.advances[i] = advW * scale;

		penX += gw + 1;
		if (gh + 1 > rowH) {
			rowH = gh + 1;
		}
	}

	// Upload as RGBA (white + alpha)
	std::vector<uint8_t> pixels(texW * texH * 4);
	for (int i = 0; i < texW * texH; i++) {
		pixels[i * 4] = 255;
		pixels[i * 4 + 1] = 255;
		pixels[i * 4 + 2] = 255;
		pixels[i * 4 + 3] = bitmap[i];
	}

	glGenTextures(1, &font.texture);
	glBindTexture(GL_TEXTURE_2D, font.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	font.texW = texW;
	font.texH = texH;
	font.loaded = true;
}

void GLRenderer::initFontAtlasFallback() {
	const int glyphW = 10;
	const int glyphH = 16;
	const int cols = 16;
	const int rows = 6;
	const int texW = cols * glyphW;
	const int texH = rows * glyphH;

	wxBitmap bmp(texW, texH, 24);
	wxMemoryDC dc(bmp);
	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();
	dc.SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	dc.SetTextForeground(*wxWHITE);
	for (int i = 0; i < 96; i++) {
		int col = i % cols;
		int row = i / cols;
		const auto ch = static_cast<char>(32 + i);
		dc.DrawText(wxString(ch), col * glyphW, row * glyphH);
	}
	dc.SelectObject(wxNullBitmap);
	wxImage img = bmp.ConvertToImage();
	std::vector<uint8_t> pixels(texW * texH * 4);
	for (int y = 0; y < texH; y++) {
		for (int x = 0; x < texW; x++) {
			int si = (y * texW + x) * 3;
			int di = (y * texW + x) * 4;
			uint8_t lum = img.GetData()[si];
			pixels[di] = 255;
			pixels[di + 1] = 255;
			pixels[di + 2] = 255;
			pixels[di + 3] = lum;
		}
	}

	glGenTextures(1, &font.texture);
	glBindTexture(GL_TEXTURE_2D, font.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	font.ascent = 12.0f;
	font.lineHeight = 16.0f;
	for (int i = 0; i < 96; i++) {
		int col = i % cols;
		int row = i / cols;
		font.glyphs[i].u0 = static_cast<float>(col * glyphW) / texW;
		font.glyphs[i].v0 = static_cast<float>(row * glyphH) / texH;
		font.glyphs[i].u1 = static_cast<float>((col + 1) * glyphW) / texW;
		font.glyphs[i].v1 = static_cast<float>((row + 1) * glyphH) / texH;
		font.glyphs[i].xoff = 0;
		font.glyphs[i].yoff = -12.0f;
		font.glyphs[i].w = static_cast<float>(glyphW);
		font.glyphs[i].h = static_cast<float>(glyphH);
		font.glyphs[i].advance = static_cast<float>(glyphW);
		font.advances[i] = static_cast<float>(glyphW);
	}

	font.texW = texW;
	font.texH = texH;
	font.loaded = true;
}

void GLRenderer::init() {
	if (std::find(s_instances.begin(), s_instances.end(), this) == s_instances.end()) {
		s_instances.push_back(this);
	}
	if (initialized) {
		return;
	}

	if (!gladLoadGLLoader((GLADloadproc)rmeGetGLProc)) {
		wxLogError("GLRenderer::init — gladLoadGLLoader failed");
		return;
	}

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertSrc, nullptr);
	glCompileShader(vs);
	{
		GLint ok = 0;
		glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetShaderInfoLog(vs, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::init — vertex shader compile error: %s", log.data());
			glDeleteShader(vs);
			return;
		}
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragSrc, nullptr);
	glCompileShader(fs);
	{
		GLint ok = 0;
		glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetShaderInfoLog(fs, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::init — fragment shader compile error: %s", log.data());
			glDeleteShader(vs);
			glDeleteShader(fs);
			return;
		}
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	{
		GLint ok = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &ok);
		if (!ok) {
			std::array<char, 512> log {};
			glGetProgramInfoLog(program, log.size(), nullptr, log.data());
			wxLogError("GLRenderer::init — program link error: %s", log.data());
			glDeleteProgram(program);
			program = 0;
			glDeleteShader(vs);
			glDeleteShader(fs);
			return;
		}
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	loc_projection = glGetUniformLocation(program, "uProjection");
	loc_texture = glGetUniformLocation(program, "uTexture");
	loc_stipple = glGetUniformLocation(program, "uStipple");

	retroProgram = rmeCompileProgram(fragRetroSrc);
	if (retroProgram != 0) {
		retr_loc_projection = glGetUniformLocation(retroProgram, "uProjection");
		retr_loc_texture = glGetUniformLocation(retroProgram, "uTexture");
		retr_loc_texSize = glGetUniformLocation(retroProgram, "uTexSize");
		retr_loc_cellSize = glGetUniformLocation(retroProgram, "uCellSize");
	}

	scalProgram = rmeCompileProgram(fragPixelArtSrc);
	if (scalProgram != 0) {
		scal_loc_projection = glGetUniformLocation(scalProgram, "uProjection");
		scal_loc_texture = glGetUniformLocation(scalProgram, "uTexture");
		scal_loc_texSize = glGetUniformLocation(scalProgram, "uTexSize");
		scal_loc_sourceCellSize = glGetUniformLocation(scalProgram, "uSourceCellSize");
		scal_loc_outputCellSize = glGetUniformLocation(scalProgram, "uOutputCellSize");
	}

	for (int i = 0; i < COMPOSITE_PASS_COUNT; ++i) {
		auto &cp = compositePrograms[i];
		cp.program = rmeCompileProgramWithVertex(compositeVertexSrc, compositePassSrc[i]);
		if (cp.program == 0) {
			continue;
		}
		cp.loc_projection = glGetUniformLocation(cp.program, "uProjection");
		cp.loc_texture = glGetUniformLocation(cp.program, "Texture");
		cp.loc_orig = glGetUniformLocation(cp.program, "OrigTexture");
		cp.loc_prev2 = glGetUniformLocation(cp.program, "PassPrev2Texture");
		cp.loc_prev5 = glGetUniformLocation(cp.program, "PassPrev5Texture");
		cp.loc_alpha = glGetUniformLocation(cp.program, "AlphaSource");
		cp.loc_texSize = glGetUniformLocation(cp.program, "TextureSize");
		cp.loc_outSize = glGetUniformLocation(cp.program, "OutputSize");
		cp.loc_inputSize = glGetUniformLocation(cp.program, "InputSize");
	}

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, STREAM_VBO_CAPACITY * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, STREAM_EBO_CAPACITY * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, r));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (retroProgram != 0) {
		glGenVertexArrays(1, &retroVao);
		glGenBuffers(1, &retroVbo);
		glBindVertexArray(retroVao);
		glBindBuffer(GL_ARRAY_BUFFER, retroVbo);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(RetroVertex), nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, x));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, u));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, r));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (compositePrograms[0].program != 0 && compositePrograms[COMPOSITE_PASS_COUNT - 1].program != 0) {
		glGenFramebuffers(1, &compositeFbo);
		glGenVertexArrays(1, &compositeVao);
		glGenBuffers(1, &compositeVbo);
		glBindVertexArray(compositeVao);
		glBindBuffer(GL_ARRAY_BUFFER, compositeVbo);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(RetroVertex), nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, x));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, u));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(RetroVertex), (void*)offsetof(RetroVertex, r));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	initFontAtlas();

	std::array<uint8_t, 4> white = { 255, 255, 255, 255 };
	glGenTextures(1, &whitePixelTexture);
	glBindTexture(GL_TEXTURE_2D, whitePixelTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	initialized = true;
}

void GLRenderer::shutdown() {
	current_texture = 0;
	std::erase(s_instances, this);
	if (!initialized) {
		return;
	}
	destroyFBO();
	if (program) {
		glDeleteProgram(program);
		program = 0;
	}
	if (retroProgram) {
		glDeleteProgram(retroProgram);
		retroProgram = 0;
	}
	if (scalProgram) {
		glDeleteProgram(scalProgram);
		scalProgram = 0;
	}
	for (auto &cp : compositePrograms) {
		if (cp.program) {
			glDeleteProgram(cp.program);
			cp.program = 0;
		}
	}
	destroyCompositeTargets();
	if (compositeFbo) {
		glDeleteFramebuffers(1, &compositeFbo);
		compositeFbo = 0;
	}
	if (compositeVao) {
		glDeleteVertexArrays(1, &compositeVao);
		compositeVao = 0;
	}
	if (compositeVbo) {
		glDeleteBuffers(1, &compositeVbo);
		compositeVbo = 0;
	}
	if (retroVao) {
		glDeleteVertexArrays(1, &retroVao);
		retroVao = 0;
	}
	if (retroVbo) {
		glDeleteBuffers(1, &retroVbo);
		retroVbo = 0;
	}
	if (vbo) {
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
	if (ebo) {
		glDeleteBuffers(1, &ebo);
		ebo = 0;
	}
	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	if (whitePixelTexture) {
		glDeleteTextures(1, &whitePixelTexture);
		whitePixelTexture = 0;
	}
	if (font.texture) {
		glDeleteTextures(1, &font.texture);
		font.texture = 0;
	}
	initialized = false;
}

void GLRenderer::setOrtho(float left, float right, float bottom, float top) {
	std::array<float, 16> m {};
	m[0] = 2.0f / (right - left);
	m[5] = 2.0f / (top - bottom);
	m[10] = -1.0f;
	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[15] = 1.0f;

	projection = m;

	glUseProgram(program);
	glUniformMatrix4fv(loc_projection, 1, GL_FALSE, m.data());
	if (retroProgram != 0) {
		glUseProgram(retroProgram);
		glUniformMatrix4fv(retr_loc_projection, 1, GL_FALSE, m.data());
	}
	if (scalProgram != 0) {
		glUseProgram(scalProgram);
		glUniformMatrix4fv(scal_loc_projection, 1, GL_FALSE, m.data());
	}
	glUseProgram(0);
}

void GLRenderer::flushBatch() {
	if (batch.empty() || !initialized) {
		return;
	}

	size_t vertexCount = batch.size();
	size_t indexCount = indexBatch.size();
	size_t vertexBytes = vertexCount * sizeof(Vertex);
	size_t indexBytes = indexCount * sizeof(GLuint);

	glUseProgram(program);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	if (vertexCount > STREAM_VBO_CAPACITY || indexCount > STREAM_EBO_CAPACITY) {
		batch.clear();
		indexBatch.clear();
		glBindVertexArray(0);
		glUseProgram(0);
		return;
	}

	if (vboOffset + vertexCount > STREAM_VBO_CAPACITY || eboOffset + indexCount > STREAM_EBO_CAPACITY) {
		glBufferData(GL_ARRAY_BUFFER, STREAM_VBO_CAPACITY * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, STREAM_EBO_CAPACITY * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
		vboOffset = 0;
		eboOffset = 0;
	}

	void* vboPtr = glMapBufferRange(GL_ARRAY_BUFFER, vboOffset * sizeof(Vertex), vertexBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	if (!vboPtr) {
		batch.clear();
		indexBatch.clear();
		glBindVertexArray(0);
		glUseProgram(0);
		return;
	}
	std::memcpy(vboPtr, batch.data(), vertexBytes);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	void* eboPtr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, eboOffset * sizeof(GLuint), indexBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	if (!eboPtr) {
		batch.clear();
		indexBatch.clear();
		glBindVertexArray(0);
		glUseProgram(0);
		return;
	}
	std::memcpy(eboPtr, indexBatch.data(), indexBytes);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, current_texture);
	glUniform1i(loc_texture, 0);

	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(eboOffset * sizeof(GLuint)), (GLint)vboOffset);

	vboOffset += vertexCount;
	eboOffset += indexCount;

	glBindVertexArray(0);
	glUseProgram(0);
	batch.clear();
	indexBatch.clear();
}

void GLRenderer::drawTexturedQuad(float x, float y, float w, float h, GLuint textureId, const GLColor &color, float u0, float v0_, float u1, float v1_) {
	DrawCommand cmd;
	cmd.state.textureId = textureId;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = true;
	cmd.vertices = {
		{ x, y, u0, v0_, color.r, color.g, color.b, color.a },
		{ x + w, y, u1, v0_, color.r, color.g, color.b, color.a },
		{ x + w, y + h, u1, v1_, color.r, color.g, color.b, color.a },
		{ x, y + h, u0, v1_, color.r, color.g, color.b, color.a },
	};
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawColoredQuad(float x, float y, float w, float h, const GLColor &color) {
	DrawCommand cmd;
	cmd.state.textureId = whitePixelTexture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = true;
	cmd.vertices = {
		{ x, y, 0, 0, color.r, color.g, color.b, color.a },
		{ x + w, y, 0, 0, color.r, color.g, color.b, color.a },
		{ x + w, y + h, 0, 0, color.r, color.g, color.b, color.a },
		{ x, y + h, 0, 0, color.r, color.g, color.b, color.a },
	};
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawThickLineSegment(float x1, float y1, float x2, float y2, float width, const GLColor &color) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 1e-6f) {
		return;
	}
	float nx = (-dy / len) * (width * 0.5f);
	float ny = (dx / len) * (width * 0.5f);

	DrawCommand cmd;
	cmd.state.textureId = whitePixelTexture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = true;
	cmd.vertices = {
		{ x1 + nx, y1 + ny, 0, 0, color.r, color.g, color.b, color.a },
		{ x1 - nx, y1 - ny, 0, 0, color.r, color.g, color.b, color.a },
		{ x2 - nx, y2 - ny, 0, 0, color.r, color.g, color.b, color.a },
		{ x2 + nx, y2 + ny, 0, 0, color.r, color.g, color.b, color.a },
	};
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawRect(float x, float y, float w, float h, const GLColor &color, float lineWidth) {
	drawThickLineSegment(x, y, x + w, y, lineWidth, color);
	drawThickLineSegment(x + w, y, x + w, y + h, lineWidth, color);
	drawThickLineSegment(x + w, y + h, x, y + h, lineWidth, color);
	drawThickLineSegment(x, y + h, x, y, lineWidth, color);
}

void GLRenderer::drawRoundedRect(float x, float y, float w, float h, float radius, const GLColor &fill) {
	const int segments = 8;

	float cx = x + w * 0.5f;
	float cy = y + h * 0.5f;
	Vertex center = { cx, cy, 0, 0, fill.r, fill.g, fill.b, fill.a };

	std::array<std::array<float, 2>, 4> corners = { {
		{ x + radius, y + radius },
		{ x + w - radius, y + radius },
		{ x + w - radius, y + h - radius },
		{ x + radius, y + h - radius },
	} };

	constexpr float pi = std::numbers::pi_v<float>;
	std::array<float, 4> startAngle = { pi, 1.5f * pi, 0.0f, 0.5f * pi };

	std::vector<Vertex> perimeter;
	for (int c = 0; c < 4; ++c) {
		for (int s = 0; s <= segments; ++s) {
			float angle = startAngle[c] + (s / static_cast<float>(segments)) * (pi * 0.5f);
			float px = corners[c][0] + cosf(angle) * radius;
			float py = corners[c][1] + sinf(angle) * radius;
			perimeter.push_back({ px, py, 0, 0, fill.r, fill.g, fill.b, fill.a });
		}
	}

	DrawCommand cmd;
	cmd.state.textureId = whitePixelTexture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = false;
	for (size_t i = 0; i < perimeter.size(); ++i) {
		size_t next = (i + 1) % perimeter.size();
		cmd.vertices.push_back(center);
		cmd.vertices.push_back(perimeter[i]);
		cmd.vertices.push_back(perimeter[next]);
	}
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawRoundedRectOutline(float x, float y, float w, float h, float radius, const GLColor &color, float lineWidth) {
	const int segments = 8;

	std::array<std::array<float, 2>, 4> corners = { {
		{ x + radius, y + radius },
		{ x + w - radius, y + radius },
		{ x + w - radius, y + h - radius },
		{ x + radius, y + h - radius },
	} };

	constexpr float pi = std::numbers::pi_v<float>;
	std::array<float, 4> startAngle = { pi, 1.5f * pi, 0.0f, 0.5f * pi };

	std::vector<std::array<float, 2>> perimeter;
	for (int c = 0; c < 4; ++c) {
		for (int s = 0; s <= segments; ++s) {
			float angle = startAngle[c] + (s / static_cast<float>(segments)) * (pi * 0.5f);
			float px = corners[c][0] + cosf(angle) * radius;
			float py = corners[c][1] + sinf(angle) * radius;
			perimeter.push_back({ px, py });
		}
	}

	for (size_t i = 0; i < perimeter.size(); ++i) {
		size_t next = (i + 1) % perimeter.size();
		drawThickLineSegment(perimeter[i][0], perimeter[i][1], perimeter[next][0], perimeter[next][1], lineWidth, color);
	}
}

void GLRenderer::drawLine(float x1, float y1, float x2, float y2, const GLColor &color, float width) {
	drawThickLineSegment(x1, y1, x2, y2, width, color);
}

void GLRenderer::drawLines(const float* vertices, int pairCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float width) {
	GLColor c = { r, g, b, a };
	for (int i = 0; i < pairCount; ++i) {
		float x1 = vertices[i * 4];
		float y1 = vertices[i * 4 + 1];
		float x2 = vertices[i * 4 + 2];
		float y2 = vertices[i * 4 + 3];
		drawThickLineSegment(x1, y1, x2, y2, width, c);
	}
}

void GLRenderer::drawStippledLines(const float* vertices, int pairCount, const GLColor &color, float width, int factor, uint16_t pattern) {
	for (int i = 0; i < pairCount; ++i) {
		float x1 = vertices[i * 4];
		float y1 = vertices[i * 4 + 1];
		float x2 = vertices[i * 4 + 2];
		float y2 = vertices[i * 4 + 3];

		float dx = x2 - x1;
		float dy = y2 - y1;
		float len = sqrtf(dx * dx + dy * dy);
		if (len < 1e-6f) {
			continue;
		}

		float dirX = dx / len;
		float dirY = dy / len;
		auto step = static_cast<float>(factor);
		int bit = 0;
		float pos = 0.0f;

		while (pos < len) {
			float segEnd = pos + step;
			if (segEnd > len) {
				segEnd = len;
			}

			if (pattern & (1 << (bit & 15))) {
				float sx = x1 + dirX * pos;
				float sy = y1 + dirY * pos;
				float ex = x1 + dirX * segEnd;
				float ey = y1 + dirY * segEnd;
				drawThickLineSegment(sx, sy, ex, ey, width, color);
			}

			pos = segEnd;
			bit++;
		}
	}
}

void GLRenderer::drawPolygon(const float* vertices, int vertexCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if (vertexCount < 3) {
		return;
	}
	DrawCommand cmd;
	cmd.state.textureId = whitePixelTexture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = false;
	for (int i = 1; i < vertexCount - 1; ++i) {
		cmd.vertices.push_back({ vertices[0], vertices[1], 0, 0, r, g, b, a });
		cmd.vertices.push_back({ vertices[i * 2], vertices[i * 2 + 1], 0, 0, r, g, b, a });
		cmd.vertices.push_back({ vertices[(i + 1) * 2], vertices[(i + 1) * 2 + 1], 0, 0, r, g, b, a });
	}
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawTriangleFan(const float* vertices, int vertexCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if (vertexCount < 3) {
		return;
	}
	DrawCommand cmd;
	cmd.state.textureId = whitePixelTexture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = false;
	for (int i = 1; i < vertexCount - 1; ++i) {
		cmd.vertices.push_back({ vertices[0], vertices[1], 0, 0, r, g, b, a });
		cmd.vertices.push_back({ vertices[i * 2], vertices[i * 2 + 1], 0, 0, r, g, b, a });
		cmd.vertices.push_back({ vertices[(i + 1) * 2], vertices[(i + 1) * 2 + 1], 0, 0, r, g, b, a });
	}
	commandList.push_back(std::move(cmd));
}

void GLRenderer::drawText(float x, float y, const std::string &text, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	font.textColor = { r, g, b, a };
	setRasterPos(x, y);
	for (char c : text) {
		drawBitmapChar(c);
	}
}

float GLRenderer::getCharWidth(char c) {
	if (c < 32 || c > 127) {
		return 0.0f;
	}
	return font.advances[c - 32];
}

float GLRenderer::getLineHeight() const {
	return font.lineHeight;
}

float GLRenderer::getAscent() const {
	return font.ascent;
}

void GLRenderer::setRasterPos(float x, float y) {
	font.cursorX = x;
	font.cursorY = y;
}

void GLRenderer::drawBitmapChar(char c) {
	if (c < 32 || c > 127) {
		return;
	}
	int idx = c - 32;
	const auto &g = font.glyphs[idx];
	float qx = font.cursorX + g.xoff;
	float qy = font.cursorY + g.yoff;
	float qw = g.w;
	float qh = g.h;
	DrawCommand cmd;
	cmd.state.textureId = font.texture;
	cmd.state.blendSrc = activeBlendSrc;
	cmd.state.blendDst = activeBlendDst;
	cmd.isQuadBatch = true;
	cmd.vertices = {
		{ qx, qy, g.u0, g.v0, font.textColor.r, font.textColor.g, font.textColor.b, font.textColor.a },
		{ qx + qw, qy, g.u1, g.v0, font.textColor.r, font.textColor.g, font.textColor.b, font.textColor.a },
		{ qx + qw, qy + qh, g.u1, g.v1, font.textColor.r, font.textColor.g, font.textColor.b, font.textColor.a },
		{ qx, qy + qh, g.u0, g.v1, font.textColor.r, font.textColor.g, font.textColor.b, font.textColor.a },
	};
	commandList.push_back(std::move(cmd));
	font.cursorX += g.advance;
}

void GLRenderer::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	font.textColor = { r, g, b, a };
}

void GLRenderer::mergeCommands() {
	if (commandList.size() <= 1) {
		return;
	}
	size_t write = 0;
	for (size_t read = 1; read < commandList.size(); ++read) {
		if (commandList[write].state == commandList[read].state && commandList[write].isQuadBatch == commandList[read].isQuadBatch) {
			auto &src = commandList[read].vertices;
			auto &dst = commandList[write].vertices;
			dst.insert(dst.end(), src.begin(), src.end());
		} else {
			++write;
			if (write != read) {
				commandList[write] = std::move(commandList[read]);
			}
		}
	}
	commandList.resize(write + 1);
}

void GLRenderer::flushCommands() {
	mergeCommands();

	unsigned int currentBlendSrc = 0;
	unsigned int currentBlendDst = 0;

	for (auto &cmd : commandList) {
		bool textureChanged = current_texture != cmd.state.textureId;
		bool blendChanged = cmd.state.blendSrc != currentBlendSrc || cmd.state.blendDst != currentBlendDst;

		if ((textureChanged || blendChanged) && !batch.empty()) {
			flushBatch();
		}

		if (blendChanged) {
			currentBlendSrc = cmd.state.blendSrc;
			currentBlendDst = cmd.state.blendDst;
			if (currentBlendSrc != 0) {
				glBlendFunc(currentBlendSrc, currentBlendDst);
			} else {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}

		current_texture = cmd.state.textureId;

		// Processes command vertices in slices that fit into the buffer
		const auto &verts = cmd.vertices;
		size_t vertStep = cmd.isQuadBatch ? 4 : 3;
		size_t idxPerStep = cmd.isQuadBatch ? 6 : 3;
		size_t i = 0;

		while (i < verts.size()) {
			size_t remaining = verts.size() - i;
			size_t batchFree = STREAM_VBO_CAPACITY - batch.size();
			// Number of vertices that still fit (rounded to a multiple of vertStep)
			size_t canTake = (batchFree / vertStep) * vertStep;

			if (canTake == 0) {
				flushBatch();
				canTake = (STREAM_VBO_CAPACITY / vertStep) * vertStep;
			}

			size_t take = std::min(remaining, canTake);

			if (cmd.isQuadBatch) {
				auto base = (GLuint)batch.size();
				batch.insert(batch.end(), verts.begin() + i, verts.begin() + i + take);
				for (size_t q = 0; q < take; q += 4) {
					GLuint b = base + (GLuint)q;
					indexBatch.push_back(b);
					indexBatch.push_back(b + 1);
					indexBatch.push_back(b + 2);
					indexBatch.push_back(b);
					indexBatch.push_back(b + 2);
					indexBatch.push_back(b + 3);
				}
			} else {
				auto base = (GLuint)batch.size();
				batch.insert(batch.end(), verts.begin() + i, verts.begin() + i + take);
				for (GLuint j = 0; j < (GLuint)take; ++j) {
					indexBatch.push_back(base + j);
				}
			}

			i += take;
		}
	}
	commandList.clear();
	flushBatch();

	if (currentBlendSrc != 0) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void GLRenderer::setBlendMode(unsigned int src, unsigned int dst) {
	activeBlendSrc = src;
	activeBlendDst = dst;
}

void GLRenderer::resetBlendMode() {
	activeBlendSrc = 0;
	activeBlendDst = 0;
}

void GLRenderer::flush() {
	flushCommands();
}

void GLRenderer::invalidateTexture(GLuint id) {
	for (auto* inst : s_instances) {
		if (inst->current_texture == id) {
			inst->current_texture = 0;
		}
	}
}

void GLRenderer::ensureFBO(int w, int h, bool smooth) {
	if (fboData.fbo != 0 && fboData.width == w && fboData.height == h && fboData.smooth == smooth) {
		return;
	}
	destroyFBO();

	glGenFramebuffers(1, &fboData.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fboData.fbo);

	glGenTextures(1, &fboData.texture);
	glBindTexture(GL_TEXTURE_2D, fboData.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	const GLint fboFilter = smooth ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fboFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fboFilter);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboData.texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		spdlog::error("[GLRenderer::ensureFBO] Framebuffer incomplete");
		glDeleteTextures(1, &fboData.texture);
		glDeleteFramebuffers(1, &fboData.fbo);
		fboData.fbo = 0;
		fboData.texture = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	fboData.width = w;
	fboData.height = h;
	fboData.smooth = smooth;
}

void GLRenderer::destroyFBO() {
	if (fboData.texture != 0) {
		glDeleteTextures(1, &fboData.texture);
		fboData.texture = 0;
	}
	if (fboData.fbo != 0) {
		glDeleteFramebuffers(1, &fboData.fbo);
		fboData.fbo = 0;
	}
	fboData.width = 0;
	fboData.height = 0;
	fboData.smooth = false;
}

void GLRenderer::beginFBO() {
	if (fboData.fbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboData.fbo);
		glViewport(0, 0, fboData.width, fboData.height);
	}
}

void GLRenderer::endFBO() {
	if (fboData.fbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void GLRenderer::blitFBO(float w, float h, int sourceCellSize, float outputCellSize, int outputWidth, int outputHeight) {
	if (fboData.fbo == 0) {
		return;
	}
	glViewport(0, 0, outputWidth, outputHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Spatial upscale: 0 = nearest, 1 = Smooth Retro, 2 = composite chain, 3 = xBRZ (4x).
	// The xBRZ scaler only magnifies, and needs the scene in
	// integer "source cells" (sourceCellSize FBO texels per sprite pixel) plus
	// the exact screen-pixel size of a cell (outputCellSize).
	const int scaleFilter = g_settings.getInteger(Config::SCALE_FILTER);
	if (scaleFilter == 3 && scalProgram != 0 && sourceCellSize >= 1 && outputCellSize > 1.0f) {
		const RetroVertex verts[6] = {
			{ 0.0f, 0.0f, 0.0f, 1.0f, 255, 255, 255, 255 },
			{ w, 0.0f, 1.0f, 1.0f, 255, 255, 255, 255 },
			{ w, h, 1.0f, 0.0f, 255, 255, 255, 255 },
			{ 0.0f, 0.0f, 0.0f, 1.0f, 255, 255, 255, 255 },
			{ w, h, 1.0f, 0.0f, 255, 255, 255, 255 },
			{ 0.0f, h, 0.0f, 0.0f, 255, 255, 255, 255 },
		};

		glUseProgram(scalProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboData.texture);
		glUniformMatrix4fv(scal_loc_projection, 1, GL_FALSE, projection.data());
		glUniform1i(scal_loc_texture, 0);
		glUniform2f(scal_loc_texSize, static_cast<float>(fboData.width), static_cast<float>(fboData.height));
		glUniform1i(scal_loc_sourceCellSize, sourceCellSize);
		glUniform1f(scal_loc_outputCellSize, outputCellSize);

		glBindVertexArray(retroVao);
		glBindBuffer(GL_ARRAY_BUFFER, retroVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		current_texture = 0;
		return;
	}

	// Nearest and Smooth Retro resolve the scene with a single filtered blit.
	// Smooth Retro renders into a supersampled FBO (see MapDrawer::Draw) so the
	// GL_LINEAR texture filter does the antialiasing, with no scaling shader.
	drawTexturedQuad(0, 0, w, h, fboData.texture, { 255, 255, 255, 255 }, 0.f, 1.f, 1.f, 0.f);
	flush();
}

void GLRenderer::ensureCompositeTarget(int index, int w, int h, bool linear, bool highPrecision) {
	auto &t = compositeTargets[index];
	if (t.texture != 0 && t.width == w && t.height == h && t.linear == linear && t.highPrecision == highPrecision) {
		return;
	}
	if (t.texture != 0) {
		glDeleteTextures(1, &t.texture);
		t.texture = 0;
	}
	glGenTextures(1, &t.texture);
	glBindTexture(GL_TEXTURE_2D, t.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, highPrecision ? GL_RGBA16F : GL_RGBA, w, h, 0, GL_RGBA, highPrecision ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
	const GLint filter = linear ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	t.width = w;
	t.height = h;
	t.linear = linear;
	t.highPrecision = highPrecision;
}

void GLRenderer::destroyCompositeTargets() {
	for (auto &t : compositeTargets) {
		if (t.texture != 0) {
			glDeleteTextures(1, &t.texture);
		}
		t = CompositeTarget {};
	}
}

void GLRenderer::runCompositePass(int pass, GLuint inputTex, int inputW, int inputH, GLuint origTex, GLuint prev2Tex, GLuint prev5Tex, int targetIndex, int outW, int outH, GLuint alphaTex) {
	auto &p = compositePrograms[pass];
	if (p.program == 0) {
		return;
	}

	if (targetIndex < 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, compositeFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositeTargets[targetIndex].texture, 0);
	}
	glViewport(0, 0, outW, outH);

	if (targetIndex < 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}

	static const float kIdentity[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	glUseProgram(p.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTex);
	if (p.loc_texture >= 0) {
		glUniform1i(p.loc_texture, 0);
	}
	if (p.loc_orig >= 0) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, origTex != 0 ? origTex : inputTex);
		glUniform1i(p.loc_orig, 1);
	}
	if (p.loc_prev2 >= 0) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prev2Tex != 0 ? prev2Tex : inputTex);
		glUniform1i(p.loc_prev2, 2);
	}
	if (p.loc_prev5 >= 0) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prev5Tex != 0 ? prev5Tex : inputTex);
		glUniform1i(p.loc_prev5, 2);
	}
	if (p.loc_alpha >= 0) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, alphaTex != 0 ? alphaTex : inputTex);
		glUniform1i(p.loc_alpha, 3);
	}
	if (p.loc_projection >= 0) {
		glUniformMatrix4fv(p.loc_projection, 1, GL_FALSE, kIdentity);
	}
	if (p.loc_texSize >= 0) {
		glUniform2f(p.loc_texSize, static_cast<float>(inputW), static_cast<float>(inputH));
	}
	if (p.loc_outSize >= 0) {
		glUniform2f(p.loc_outSize, static_cast<float>(outW), static_cast<float>(outH));
	}
	if (p.loc_inputSize >= 0) {
		glUniform2f(p.loc_inputSize, static_cast<float>(fboData.width), static_cast<float>(fboData.height));
	}

	const RetroVertex verts[6] = {
		{ -1.0f, -1.0f, 0.0f, 0.0f, 255, 255, 255, 255 },
		{ 1.0f, -1.0f, 1.0f, 0.0f, 255, 255, 255, 255 },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 255, 255, 255, 255 },
		{ -1.0f, -1.0f, 0.0f, 0.0f, 255, 255, 255, 255 },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 255, 255, 255, 255 },
		{ -1.0f, 1.0f, 0.0f, 1.0f, 255, 255, 255, 255 },
	};
	glBindVertexArray(compositeVao);
	glBindBuffer(GL_ARRAY_BUFFER, compositeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderer::presentComposite(int outputWidth, int outputHeight, bool rebuild) {
	if (fboData.fbo == 0 || !hasComposite()) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, outputWidth, outputHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const int nativeW = fboData.width;
	const int nativeH = fboData.height;
	const int tripleW = nativeW * 3;
	const int tripleH = nativeH * 3;

	if (rebuild || compositeTargets[5].texture == 0) {
		ensureCompositeTarget(0, nativeW, nativeH, false, true);
		ensureCompositeTarget(1, nativeW, nativeH, false, true);
		ensureCompositeTarget(2, nativeW, nativeH, false, true);
		ensureCompositeTarget(3, nativeW, nativeH, false, true);
		ensureCompositeTarget(4, nativeW, nativeH, false, true);
		// Nearest keeps the final 3x -> screen resolve crisp (no bilinear softening).
		ensureCompositeTarget(5, tripleW, tripleH, false, false);
		// Screen-sized target for the sharpsmoother resolve feeding the bloom.
		ensureCompositeTarget(6, outputWidth, outputHeight, true, false);

		for (int i = 0; i < COMPOSITE_TARGET_COUNT; ++i) {
			if (compositeTargets[i].texture == 0) {
				return;
			}
		}

		const GLuint scene = fboData.texture;
		const GLuint t0 = compositeTargets[0].texture;
		const GLuint t1 = compositeTargets[1].texture;
		const GLuint t2 = compositeTargets[2].texture;
		const GLuint t3 = compositeTargets[3].texture;
		const GLuint t4 = compositeTargets[4].texture;

		// MDAPT (native resolution): scene -> t0 -> t1 -> t2 -> t3 -> t0 (mdapt output).
		runCompositePass(0, scene, nativeW, nativeH, 0, 0, 0, 0, nativeW, nativeH, 0);
		runCompositePass(1, t0, nativeW, nativeH, 0, 0, 0, 1, nativeW, nativeH, 0);
		runCompositePass(2, t1, nativeW, nativeH, 0, 0, 0, 2, nativeW, nativeH, 0);
		runCompositePass(3, t2, nativeW, nativeH, scene, 0, 0, 3, nativeW, nativeH, 0);
		runCompositePass(4, t3, nativeW, nativeH, scene, 0, 0, 0, nativeW, nativeH, 0);

		// ScaleFX-Hybrid: mdapt output -> t1 -> t2 -> t3 (+t1 as prev2) -> t4 -> 3x target.
		runCompositePass(5, t0, nativeW, nativeH, 0, 0, 0, 1, nativeW, nativeH, 0);
		runCompositePass(6, t1, nativeW, nativeH, 0, 0, 0, 2, nativeW, nativeH, 0);
		runCompositePass(7, t2, nativeW, nativeH, 0, t1, 0, 3, nativeW, nativeH, 0);
		runCompositePass(8, t3, nativeW, nativeH, 0, 0, 0, 4, nativeW, nativeH, 0);
		runCompositePass(9, t4, nativeW, nativeH, t0, 0, t0, 5, tripleW, tripleH, 0);

		// sharpsmoother resolve into the screen-sized target, restoring the map
		// alpha so the editor background shows through where the scene is
		// transparent. Cached with the rest of the chain.
		runCompositePass(10, compositeTargets[5].texture, tripleW, tripleH, 0, 0, 0, 6, outputWidth, outputHeight, fboData.texture);
	}

	if (compositeTargets[6].texture == 0) {
		return;
	}

	// CRT phosphor bloom: per-channel glow with a P22-ish tint, to the screen.
	runCompositePass(11, compositeTargets[6].texture, outputWidth, outputHeight, 0, 0, 0, -1, outputWidth, outputHeight, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, outputWidth, outputHeight);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	current_texture = 0;
}

bool GLRenderer::compositeFits(int width, int height) {
	if (!hasComposite() || width <= 0 || height <= 0) {
		return false;
	}
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	if (maxTextureSize > 0 && (width > maxTextureSize / 3 || height > maxTextureSize / 3)) {
		return false;
	}
	return true;
}

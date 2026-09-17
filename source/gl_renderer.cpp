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

// Pixel-art scalers used when the map is zoomed in. Both operate on the FBO
// scene rebuilt as logical sprite-pixels: each sprite pixel occupies `cs`
// texels, so the neighbourhood is sampled at `base + offset * cs` and the
// intra-pixel fraction picks the 2x2 (2xSaI) or xBR output pattern.
// Ports: DOSBox render_templates_sai.h (_2xSaI) and Hyllian's xBR-lv2 shader.
static const char* const fragPixelArtSrc = R"(
#version 330
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform vec2 uTexSize;
uniform float uCellSize;
uniform int uMode; // 1 = 2xSaI, 2 = xBR
out vec4 FragColor;

vec4 texS(ivec2 p) {
	p = clamp(p, ivec2(0), ivec2(uTexSize) - ivec2(1));
	return texelFetch(uTexture, p, 0);
}

vec3 colOf(ivec2 p) {
	vec4 t = texS(p);
	return t.a > (0.5 / 255.0) ? t.rgb : vec3(0.0);
}

bool eqv(vec3 a, vec3 b) {
	return all(equal(a, b));
}

vec3 avg2(vec3 a, vec3 b) {
	return (a + b) * 0.5;
}

vec3 avg4(vec3 a, vec3 b, vec3 c, vec3 d) {
	return (a + b + c + d) * 0.25;
}

int getResult(vec3 A, vec3 B, vec3 C, vec3 D) {
	bool ac = eqv(A, C);
	bool bc = eqv(B, C);
	bool ad = eqv(A, D);
	bool bd = eqv(B, D);
	int x = (ac ? 1 : 0) + (ad ? 1 : 0);
	int y = (bc && !ac ? 1 : 0) + (bd && !ad ? 1 : 0);
	// rmap[3][3] = { {0,0,-1},{0,0,-1},{1,1,0} }
	return x == 2 ? (y == 2 ? 0 : -1) : (y == 2 ? 1 : 0);
}

vec3 saiScale(ivec2 base, int cs, vec2 f) {
	vec3 C0 = colOf(base + ivec2(-cs, -cs));
	vec3 C1 = colOf(base + ivec2(0, -cs));
	vec3 C2 = colOf(base + ivec2(cs, -cs));
	vec3 C3 = colOf(base + ivec2(-cs, 0));
	vec3 C4 = colOf(base);
	vec3 C5 = colOf(base + ivec2(cs, 0));
	vec3 C6 = colOf(base + ivec2(-cs, cs));
	vec3 C7 = colOf(base + ivec2(0, cs));
	vec3 C8 = colOf(base + ivec2(cs, cs));
	vec3 D0 = colOf(base + ivec2(-cs, 2 * cs));
	vec3 D1 = colOf(base + ivec2(0, 2 * cs));
	vec3 D2 = colOf(base + ivec2(cs, 2 * cs));
	vec3 D3 = colOf(base + ivec2(2 * cs, -cs));
	vec3 D4 = colOf(base + ivec2(2 * cs, 0));
	vec3 D5 = colOf(base + ivec2(2 * cs, cs));

	vec3 tl = C4;
	vec3 tr;
	vec3 bl;
	vec3 br;
	if (eqv(C4, C8) && !eqv(C5, C7)) {
		if (((eqv(C4, C1) && eqv(C5, D5)) ||
			(eqv(C4, C7) && eqv(C4, C2) && !eqv(C5, C1) && eqv(C5, D3)))) {
			tr = C4;
		} else {
			tr = avg2(C4, C5);
		}
		if (((eqv(C4, C3) && eqv(C7, D2)) ||
			(eqv(C4, C5) && eqv(C4, C6) && !eqv(C3, C7) && eqv(C7, D0)))) {
			bl = C4;
		} else {
			bl = avg2(C4, C7);
		}
		br = C4;
	} else if (eqv(C5, C7) && !eqv(C4, C8)) {
		if (((eqv(C5, C2) && eqv(C4, C6)) ||
			(eqv(C5, C1) && eqv(C5, C8) && !eqv(C4, C2) && eqv(C4, C0)))) {
			tr = C5;
		} else {
			tr = avg2(C4, C5);
		}
		if (((eqv(C7, C6) && eqv(C4, C2)) ||
			(eqv(C7, C3) && eqv(C7, C8) && !eqv(C4, C6) && eqv(C4, C0)))) {
			bl = C7;
		} else {
			bl = avg2(C4, C7);
		}
		br = C5;
	} else if (eqv(C4, C8) && eqv(C5, C7)) {
		if (eqv(C4, C5)) {
			tr = C4;
			bl = C4;
			br = C4;
		} else {
			int r = 0;
			r += getResult(C4, C5, C3, C1);
			r -= getResult(C5, C4, D4, C2);
			r -= getResult(C5, C4, C6, D1);
			r += getResult(C4, C5, D5, D2);
			if (r > 0) {
				br = C4;
			} else if (r < 0) {
				br = C5;
			} else {
				br = avg4(C4, C5, C7, C8);
			}
			bl = avg2(C4, C7);
			tr = avg2(C4, C5);
		}
	} else {
		br = avg4(C4, C5, C7, C8);
		if ((eqv(C4, C7) && eqv(C4, C2) && !eqv(C5, C1) && eqv(C5, D3))) {
			tr = C4;
		} else if ((eqv(C5, C1) && eqv(C5, C8) && !eqv(C4, C2) && eqv(C4, C0))) {
			tr = C5;
		} else {
			tr = avg2(C4, C5);
		}
		if ((eqv(C4, C5) && eqv(C4, C6) && !eqv(C3, C7) && eqv(C7, D0))) {
			bl = C4;
		} else if ((eqv(C7, C3) && eqv(C7, C8) && !eqv(C4, C6) && eqv(C4, C0))) {
			bl = C7;
		} else {
			bl = avg2(C4, C7);
		}
	}
	return f.x >= 0.5 ? (f.y >= 0.5 ? br : tr) : (f.y >= 0.5 ? bl : tl);
}

vec4 xbrDiff(vec4 a, vec4 b) {
	return abs(a - b);
}

vec4 xbrEq(vec4 a, vec4 b) {
	return step(xbrDiff(a, b), vec4(15.0));
}

vec4 xbrNeq(vec4 a, vec4 b) {
	return vec4(1.0) - xbrEq(a, b);
}

vec4 xbrNotEqual(vec4 a, vec4 b) {
	return vec4(notEqual(a, b));
}

vec4 xbrWd(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h) {
	return xbrDiff(a, b) + xbrDiff(a, c) + xbrDiff(d, e) + xbrDiff(d, f) + 4.0 * xbrDiff(g, h);
}

float xbrCdf(vec3 a, vec3 b) {
	vec3 d = abs(a - b);
	return d.r + d.g + d.b;
}

vec3 xbrScale(ivec2 base, int cs, vec2 fp) {
	const vec3 rgbw = vec3(14.352, 28.176, 5.472);
	float scl = clamp(float(cs), 1.0, 4.0);
	vec4 delta = vec4(1.0 / scl);
	vec4 delta_l = vec4(0.5 / scl, 1.0 / scl, 0.5 / scl, 1.0 / scl);
	vec4 delta_u = delta_l.yxwz;

	const vec4 Ao = vec4(1.0, -1.0, -1.0, 1.0);
	const vec4 Bo = vec4(1.0, 1.0, -1.0, -1.0);
	const vec4 Co = vec4(1.5, 0.5, -0.5, 0.5);
	const vec4 Ax = vec4(1.0, -1.0, -1.0, 1.0);
	const vec4 Bx = vec4(0.5, 2.0, -0.5, -2.0);
	const vec4 Cx = vec4(1.0, 1.0, -0.5, 0.0);
	const vec4 Ay = vec4(1.0, -1.0, -1.0, 1.0);
	const vec4 By = vec4(2.0, 0.5, -2.0, -0.5);
	const vec4 Cy = vec4(2.0, 0.0, -1.0, 0.5);
	const vec4 Ci = vec4(0.25, 0.25, 0.25, 0.25);

	vec3 a1 = colOf(base + ivec2(-cs, -2 * cs));
	vec3 b1 = colOf(base + ivec2(0, -2 * cs));
	vec3 c1 = colOf(base + ivec2(cs, -2 * cs));
	vec3 a2 = colOf(base + ivec2(-cs, -cs));
	vec3 b2 = colOf(base + ivec2(0, -cs));
	vec3 c2 = colOf(base + ivec2(cs, -cs));
	vec3 d2 = colOf(base + ivec2(-cs, 0));
	vec3 e2 = colOf(base);
	vec3 f2 = colOf(base + ivec2(cs, 0));
	vec3 g2 = colOf(base + ivec2(-cs, cs));
	vec3 h2 = colOf(base + ivec2(0, cs));
	vec3 i2 = colOf(base + ivec2(cs, cs));
	vec3 g5 = colOf(base + ivec2(-cs, 2 * cs));
	vec3 h5 = colOf(base + ivec2(0, 2 * cs));
	vec3 i5 = colOf(base + ivec2(cs, 2 * cs));
	vec3 a0 = colOf(base + ivec2(-2 * cs, -cs));
	vec3 d0 = colOf(base + ivec2(-2 * cs, 0));
	vec3 g0 = colOf(base + ivec2(-2 * cs, cs));
	vec3 c4 = colOf(base + ivec2(2 * cs, -cs));
	vec3 f4 = colOf(base + ivec2(2 * cs, 0));
	vec3 i4 = colOf(base + ivec2(2 * cs, cs));

	vec4 bv = vec4(dot(b2, rgbw), dot(d2, rgbw), dot(h2, rgbw), dot(f2, rgbw));
	vec4 cv = vec4(dot(c2, rgbw), dot(a2, rgbw), dot(g2, rgbw), dot(i2, rgbw));
	vec4 dv = bv.yzwx;
	vec4 ev = vec4(dot(e2, rgbw));
	vec4 fv = bv.wxyz;
	vec4 gv = cv.zwxy;
	vec4 hv = bv.zwxy;
	vec4 iv = cv.wxyz;

	vec4 i4v = vec4(dot(i4, rgbw), dot(c1, rgbw), dot(a0, rgbw), dot(g5, rgbw));
	vec4 i5v = vec4(dot(i5, rgbw), dot(c4, rgbw), dot(a1, rgbw), dot(g0, rgbw));
	vec4 h5v = vec4(dot(h5, rgbw), dot(f4, rgbw), dot(b1, rgbw), dot(d0, rgbw));
	vec4 f4v = vec4(dot(f4, rgbw));

	vec4 fx = (Ao * fp.y + Bo * fp.x);
	vec4 fx_l = (Ax * fp.y + Bx * fp.x);
	vec4 fx_u = (Ay * fp.y + By * fp.x);

	vec4 irlv0 = xbrNotEqual(ev, fv) * xbrNotEqual(ev, hv);
	vec4 irlv1 = irlv0 * (
		xbrNeq(fv, bv) * xbrNeq(fv, cv) +
		xbrNeq(hv, dv) * xbrNeq(hv, gv) +
		xbrEq(ev, iv) * (xbrNeq(fv, f4v) * xbrNeq(fv, i4v) + xbrNeq(hv, h5v) * xbrNeq(hv, i5v)) +
		xbrEq(ev, gv) + xbrEq(ev, cv));
	vec4 irlv2l = xbrNotEqual(ev, gv) * xbrNotEqual(dv, gv);
	vec4 irlv2u = xbrNotEqual(ev, cv) * xbrNotEqual(bv, cv);

	vec4 fx45i = clamp((fx + delta - Co - Ci) / (2.0 * delta), 0.0, 1.0);
	vec4 fx45 = clamp((fx + delta - Co) / (2.0 * delta), 0.0, 1.0);
	vec4 fx30 = clamp((fx_l + delta_l - Cx) / (2.0 * delta_l), 0.0, 1.0);
	vec4 fx60 = clamp((fx_u + delta_u - Cy) / (2.0 * delta_u), 0.0, 1.0);

	vec4 wd1 = xbrWd(ev, cv, gv, iv, h5v, f4v, hv, fv);
	vec4 wd2 = xbrWd(hv, dv, i5v, fv, i4v, bv, ev, iv);

	vec4 edri = step(wd1, wd2) * irlv0;
	vec4 edr = step(wd1 + vec4(0.1), wd2) * step(vec4(0.5), irlv1);
	vec4 edr_l = step(2.0 * xbrDiff(fv, gv), xbrDiff(hv, cv)) * irlv2l * edr;
	vec4 edr_u = step(2.0 * xbrDiff(hv, cv), xbrDiff(fv, gv)) * irlv2u * edr;

	fx45 = edr * fx45;
	fx30 = edr_l * fx30;
	fx60 = edr_u * fx60;
	fx45i = edri * fx45i;

	vec4 px = step(xbrDiff(ev, fv), xbrDiff(ev, hv));

	vec4 maximos = max(max(fx30, fx60), max(fx45, fx45i));

	vec3 res1 = e2;
	res1 = mix(res1, mix(h2, f2, px.x), maximos.x);
	res1 = mix(res1, mix(b2, d2, px.z), maximos.z);

	vec3 res2 = e2;
	res2 = mix(res2, mix(f2, b2, px.y), maximos.y);
	res2 = mix(res2, mix(d2, h2, px.w), maximos.w);

	return mix(res1, res2, step(xbrCdf(e2, res1), xbrCdf(e2, res2)));
}

void main() {
	int cs = max(1, int(uCellSize + 0.5f));
	if (cs < 2) {
		FragColor = texture(uTexture, vUV) * vColor;
		return;
	}

	vec2 p = gl_FragCoord.xy / float(cs);
	ivec2 c = ivec2(int(floor(p.x)), int(floor(p.y)));
	vec2 f = p - vec2(c);
	ivec2 base = c * cs;

	vec3 color = (uMode == 1) ? saiScale(base, cs, f) : xbrScale(base, cs, f);
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

static GLuint rmeCompileProgram(const char* fragSrc) {
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertSrc, nullptr);
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
		scal_loc_cellSize = glGetUniformLocation(scalProgram, "uCellSize");
		scal_loc_mode = glGetUniformLocation(scalProgram, "uMode");
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

void GLRenderer::ensureFBO(int w, int h) {
	if (fboData.fbo != 0 && fboData.width == w && fboData.height == h) {
		return;
	}
	destroyFBO();

	glGenFramebuffers(1, &fboData.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fboData.fbo);

	glGenTextures(1, &fboData.texture);
	glBindTexture(GL_TEXTURE_2D, fboData.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
}

void GLRenderer::beginFBO() {
	if (fboData.fbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboData.fbo);
	}
}

void GLRenderer::endFBO() {
	if (fboData.fbo != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void GLRenderer::blitFBO(float w, float h, float cellScale) {
	if (fboData.fbo == 0) {
		return;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Spatial upscale: pick the legend-activated algorithm (see View -> Scaling
	// Filter). 0 = nearest, 1 = Smooth Retro, 2 = 2xSaI, 3 = xBR (4x).
	const int scaleFilter = g_settings.getInteger(Config::SCALE_FILTER);
	GLuint useScaleProgram = 0;
	int scaleMode = 0;
	if (scaleFilter == 1 && retroProgram != 0) {
		useScaleProgram = retroProgram;
	} else if ((scaleFilter == 2 || scaleFilter == 3) && scalProgram != 0) {
		useScaleProgram = scalProgram;
		scaleMode = scaleFilter == 2 ? 1 : 2;
	}

	if (cellScale >= 1.0f && useScaleProgram != 0) {
		const RetroVertex verts[6] = {
			{ 0.0f, 0.0f, 0.0f, 1.0f, 255, 255, 255, 255 },
			{ w, 0.0f, 1.0f, 1.0f, 255, 255, 255, 255 },
			{ w, h, 1.0f, 0.0f, 255, 255, 255, 255 },
			{ 0.0f, 0.0f, 0.0f, 1.0f, 255, 255, 255, 255 },
			{ w, h, 1.0f, 0.0f, 255, 255, 255, 255 },
			{ 0.0f, h, 0.0f, 0.0f, 255, 255, 255, 255 },
		};

		glUseProgram(useScaleProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboData.texture);
		if (useScaleProgram == retroProgram) {
			glUniformMatrix4fv(retr_loc_projection, 1, GL_FALSE, projection.data());
			glUniform1i(retr_loc_texture, 0);
			glUniform2f(retr_loc_texSize, static_cast<float>(fboData.width), static_cast<float>(fboData.height));
			glUniform1f(retr_loc_cellSize, cellScale);
		} else {
			glUniformMatrix4fv(scal_loc_projection, 1, GL_FALSE, projection.data());
			glUniform1i(scal_loc_texture, 0);
			glUniform2f(scal_loc_texSize, static_cast<float>(fboData.width), static_cast<float>(fboData.height));
			glUniform1f(scal_loc_cellSize, cellScale);
			glUniform1i(scal_loc_mode, scaleMode);
		}

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

	drawTexturedQuad(0, 0, w, h, fboData.texture, { 255, 255, 255, 255 }, 0.f, 1.f, 1.f, 0.f);
	flush();
}

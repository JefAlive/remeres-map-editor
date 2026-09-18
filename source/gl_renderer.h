#ifndef RME_GL_RENDERER_H_
#define RME_GL_RENDERER_H_

#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <array>

// Minimal GL type forward declarations — full GL comes from glad in gl_renderer.cpp
using GLuint = unsigned int;
using GLint = int;

struct GLColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

class GLRenderer {
public:
	void init();
	void shutdown();

	void drawTexturedQuad(float x, float y, float w, float h, GLuint textureId, const GLColor &color, float u0 = 0.f, float v0 = 0.f, float u1 = 1.f, float v1 = 1.f);
	void drawColoredQuad(float x, float y, float w, float h, const GLColor &color);

	void drawRect(float x, float y, float w, float h, const GLColor &color, float lineWidth = 1.0f);
	void drawRoundedRect(float x, float y, float w, float h, float radius, const GLColor &fill);
	void drawRoundedRectOutline(float x, float y, float w, float h, float radius, const GLColor &color, float lineWidth = 1.0f);

	void drawLine(float x1, float y1, float x2, float y2, const GLColor &color, float width = 1.0f);
	void drawLines(const float* vertices, int pairCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float width = 1.0f);
	void drawStippledLines(const float* vertices, int pairCount, const GLColor &color, float width = 1.0f, int factor = 2, uint16_t pattern = 0xAAAA);

	void drawPolygon(const float* vertices, int vertexCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void drawTriangleFan(const float* vertices, int vertexCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	void drawText(float x, float y, const std::string &text, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	float getCharWidth(char c);
	float getLineHeight() const;
	float getAscent() const;
	void setRasterPos(float x, float y);
	void drawBitmapChar(char c);
	void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	void setOrtho(float left, float right, float bottom, float top);

	void setBlendMode(unsigned int src, unsigned int dst);
	void resetBlendMode();

	void ensureFBO(int w, int h, bool smooth = false);
	void destroyFBO();
	void beginFBO();
	void endFBO();
	void blitFBO(float w, float h, int sourceCellSize, float outputCellSize, int outputWidth, int outputHeight);
	void presentComposite(int outputWidth, int outputHeight, bool rebuild, float sourceScaleX, float sourceScaleY);
	bool compositeFits(int width, int height);
	bool hasComposite() const {
		return compositeFbo != 0 && compositePrograms[0].program != 0;
	}
	bool hasFBO() const {
		return fboData.fbo != 0;
	}

	void flush();
	static void invalidateTexture(GLuint id);

private:
	static std::vector<GLRenderer*> s_instances;
	bool initialized = false;
	static constexpr size_t STREAM_VBO_CAPACITY = 64 * 1024;
	static constexpr size_t STREAM_EBO_CAPACITY = 96 * 1024;

	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;
	size_t vboOffset = 0;
	size_t eboOffset = 0;
	GLuint whitePixelTexture = 0;
	GLuint program = 0;
	GLint loc_projection = -1;
	GLint loc_texture = -1;
	GLint loc_stipple = -1;

	// Smooth Retro post-process pass (scene upscale in screen space)
	GLuint retroProgram = 0;
	GLint retr_loc_projection = -1;
	GLint retr_loc_texture = -1;
	GLint retr_loc_texSize = -1;
	GLint retr_loc_cellSize = -1;
	GLuint retroVao = 0;
	GLuint retroVbo = 0;

	// xBRZ (4x) pixel-art scaler post-process pass; reuses retroVao/Vbo
	GLuint scalProgram = 0;
	GLint scal_loc_projection = -1;
	GLint scal_loc_texture = -1;
	GLint scal_loc_texSize = -1;
	GLint scal_loc_sourceCellSize = -1;
	GLint scal_loc_outputCellSize = -1;
	std::array<float, 16> projection {};

	// "CRT Blend" composite chain (SCALE_FILTER == 2):
	//   passes 0-4  MDAPT v2.8 checkerboard de-dither, native resolution.
	//   pass  5     Super 2xSaI, re-run as many 2x steps as needed to cover the
	//               output (2^steps, capped by GL_MAX_TEXTURE_SIZE).
	//   pass  6     AMD FidelityFX CAS sharpen (5-tap cross), at the 2^steps
	//               Super 2xSaI resolution.
	//   pass  7     nearest downscale of the 2^steps image to the output size.
	//   pass  8     crt-hyllian-glow source threshold (quarter res).
	//   pass  9/10  separable glow blur (quarter res).
	//   pass  11    glow resolve (P22 halation + gamma) to the screen.
	static constexpr int COMPOSITE_PASS_COUNT = 12;
	// Targets 0-3 are the MDAPT ping-pong buffers, 4..(4+steps-1) the Super 2xSaI
	// steps, then CAS/sharp/threshold/blur-H/blur-V.
	static constexpr int COMPOSITE_TARGET_COUNT = 16;
	static constexpr int COMPOSITE_MAX_SCALE_STEPS = 6;
	static constexpr int COMPOSITE_PASS_SCALE = 5;
	static constexpr int COMPOSITE_PASS_CAS = 6;
	static constexpr int COMPOSITE_PASS_DOWNSCALE = 7;
	static constexpr int COMPOSITE_PASS_THRESHOLD = 8;
	static constexpr int COMPOSITE_PASS_BLUR_H = 9;
	static constexpr int COMPOSITE_PASS_BLUR_V = 10;
	static constexpr int COMPOSITE_PASS_RESOLVE = 11;
	static constexpr int COMPOSITE_TARGET_SCALE_BASE = 4;
	static constexpr int COMPOSITE_TARGET_CAS = 10;
	static constexpr int COMPOSITE_TARGET_SHARP = 11;
	static constexpr int COMPOSITE_TARGET_THRESHOLD = 12;
	static constexpr int COMPOSITE_TARGET_BLUR_H = 13;
	static constexpr int COMPOSITE_TARGET_BLUR_V = 14;
	// CAS sharpness knob: 0.0 is the least ringing, 1.0 the maximum ringing.
	static constexpr float COMPOSITE_CAS_SHARPNESS = 0.35f;
	struct CompositeProgram {
		GLuint program = 0;
		GLint loc_projection = -1;
		GLint loc_texture = -1;
		GLint loc_orig = -1;
		GLint loc_prev2 = -1;
		GLint loc_prev5 = -1;
		GLint loc_alpha = -1;
		GLint loc_texSize = -1;
		GLint loc_outSize = -1;
		GLint loc_inputSize = -1;
		GLint loc_sharpness = -1;
	};
	std::array<CompositeProgram, COMPOSITE_PASS_COUNT> compositePrograms {};
	struct CompositeTarget {
		GLuint texture = 0;
		int width = 0;
		int height = 0;
		bool linear = false;
		bool highPrecision = false;
	};
	std::array<CompositeTarget, COMPOSITE_TARGET_COUNT> compositeTargets {};
	GLuint compositeFbo = 0;
	GLuint compositeVao = 0;
	GLuint compositeVbo = 0;
	// Cached dimensions of the last built composite chain, so the expensive
	// upscale/glow passes only re-run when the scene or the output size changes.
	bool compositeCacheValid = false;
	int compositeCacheW = 0;
	int compositeCacheH = 0;
	int compositeCacheSteps = 0;
	void ensureCompositeTarget(int index, int w, int h, bool linear, bool highPrecision);
	void destroyCompositeTargets();
	void runCompositePass(int pass, GLuint inputTex, int inputW, int inputH, GLuint origTex, GLuint prev2Tex, GLuint prev5Tex, int targetIndex, int outW, int outH, GLuint alphaTex, float sourceScaleX = 1.0f, float sourceScaleY = 1.0f);

	struct Vertex {
		float x;
		float y;
		float u;
		float v;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	struct DrawState {
		GLuint textureId = 0;
		unsigned int blendSrc = 0;
		unsigned int blendDst = 0;
		bool operator==(const DrawState &o) const = default;
	};

	struct DrawCommand {
		DrawState state;
		std::vector<Vertex> vertices;
		bool isQuadBatch = true;
	};

	std::vector<Vertex> batch;
	std::vector<GLuint> indexBatch;
	GLuint current_texture = 0;
	std::vector<DrawCommand> commandList;
	unsigned int activeBlendSrc = 0;
	unsigned int activeBlendDst = 0;

	struct FBOData {
		GLuint fbo = 0;
		GLuint texture = 0;
		int width = 0;
		int height = 0;
		bool smooth = false;
	};
	FBOData fboData;

	void flushBatch();
	void mergeCommands();
	void flushCommands();
	void drawThickLineSegment(float x1, float y1, float x2, float y2, float width, const GLColor &color);

	struct GlyphInfo {
		float u0;
		float v0;
		float u1;
		float v1; // UV coords in texture (normalized)
		float xoff;
		float yoff; // offset from cursor pos (pixels)
		float w;
		float h; // glyph size (pixels)
		float advance; // horizontal advance (pixels)
	};

	struct FontData {
		GLuint texture = 0;
		int texW = 0;
		int texH = 0;
		float fontSize = 0;
		float ascent = 0;
		float descent = 0;
		float lineHeight = 0;
		bool loaded = false;
		std::array<GlyphInfo, 96> glyphs {};
		std::array<float, 96> advances {};
		float cursorX = 0;
		float cursorY = 0;
		GLColor textColor { 255, 255, 255, 255 };
	};

	FontData font;
	void initFontAtlas();
	void initFontAtlasFallback();
};

#endif

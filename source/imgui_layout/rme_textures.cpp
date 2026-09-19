// ImRad runtime support library implementation for RME.
// Defines IMRAD_H_IMPLEMENTATION to emit the ImRAD support code from imrad.h,
// plus a custom LoadTextureFromFile backed by RME's sprite system. The header
// only provides LoadTextureFromFile when IMRAD_WITH_LOAD_TEXTURE is defined,
// so this definition does not clash with the header implementation.

#include "main.h"
#include "gui.h"

#define IMRAD_H_IMPLEMENTATION
#include "imrad/imrad.h"

#include "graphics.h"
#include "sprites.h"
#include "brush.h"
#include "materials.h"

namespace ImRad {

Texture LoadTextureFromFile(const std::string& filename, bool, bool, bool, bool) {
	Texture tex;
	tex.id = 0;
	tex.w = 32;
	tex.h = 32;

	// TODO: Map filename to actual RME sprite/brush.
	// The generated layout uses hardcoded paths like
	// "C:/Users/T-Gamer/Downloads/mapeditor.png" which must be mapped to RME assets:
	// - "mapeditor.png" -> minimap texture
	// - "minimap.png" -> minimap texture
	// - "Spike_Sword.gif" -> item sprite
	// - "Fire_Field.gif" -> effect sprite
	// - "Grass.gif" -> ground sprite
	// - TibiaFankit/Emotes/* -> outfit sprites

	// Placeholder until the filename mapping is implemented: render from the
	// first available item sprite so the layout shows something concrete.
	Sprite* baseSprite = g_gui.gfx.getSprite(g_gui.gfx.getItemSpriteMinID());
	if (baseSprite) {
		GameSprite* sprite = dynamic_cast<GameSprite*>(baseSprite);
		if (sprite) {
			tex.id = (ImTextureID)(uintptr_t)sprite->getHardwareID(0, 0, 0, 0, 0, 0);
			tex.w = sprite->getWidth();
			tex.h = sprite->getHeight();
		}
	}

	return tex;
}

} // namespace ImRad
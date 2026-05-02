#ifndef FONT_H
#define FONT_H

#include "types.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

// Returns pointer to 16-byte bitmap for given ASCII character
const uint8_t* font_get_glyph(char c);

#endif // FONT_H

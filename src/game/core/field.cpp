// ============================================================================
// field.cpp - console text field (core.o)
// ============================================================================

#include "game/cl/cl_console.h"

#include <string.h>

// ea: 0x004BBEB0
void Field_Clear(field_t* edit)
{
    memset(edit->buffer, 0, sizeof(edit->buffer));
    edit->cursor = 0;
    edit->scroll = 0;
    edit->drawWidth = 256;
}

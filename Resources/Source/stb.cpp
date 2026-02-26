// ============================================================
// stb.cpp — single translation unit for stb single-header libs
//
// STB_IMAGE_IMPLEMENTATION must be defined in EXACTLY ONE .cpp
// file. All other files that include <stb_image.h> must NOT
// define it, or the linker will see duplicate symbols.
// ============================================================
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
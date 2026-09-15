// ============================================================
// Stb.cpp - single translation unit for stb single-header libs
//
// STB_IMAGE_IMPLEMENTATION must be defined in EXACTLY ONE .cpp
// file. All other files that include <stb_image.h> must NOT
// define it, or the linker will see duplicate symbols.
// ============================================================


#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

// ====================================

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

// ====================================

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stb_image_resize2.h>

// ====================================

#define STB_DXT_IMPLEMENTATION

#include <stb_dxt.h>

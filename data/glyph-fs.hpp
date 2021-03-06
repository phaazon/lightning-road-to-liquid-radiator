#ifndef __GLYPH_FS_HPP
#define __GLYPH_FS_HPP

char const *SHD_GLYPH_FS =
"#version 330 core\n"
"in vec2 uv;"
"out vec4 frag;"
"uniform vec4 gres;" /* glyph resolution */
"uniform sampler2D tex;" /* texture of the glyph */
"void main(){"
  "float f=texture2D(tex,vec2(uv.x,(1.-uv.y))).r;"
  "frag=vec4(f,f,f,1.);"
"}";

#endif /* guard */


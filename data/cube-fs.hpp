#ifndef __CUBE_FS_HPP
#define __CUBE_FS_HPP

char const *SHD_CUBE_FS =
"#version 330 core\n"
"uniform float time;"
"in vec3 pos;"
"out vec4 frag;"
"vec3 tex(vec3 uv){"
  "float v=sin(length(uv)*4.*time);"
  "return vec3(v*cos(time),0.5+v/2.,1.-v*sin(time));"
"}"
"void main(){"
  "vec3 lpos=vec3(cos(time)*8.,sin(time)*8.,20.);"
  "vec3 no=normalize(pos);"
  "vec3 ldir=normalize(lpos-pos);"
  "float d=max(0.,dot(ldir,no));"
  "frag=vec4(tex(pos),1.)*d;"
"}";

#endif /* guard */


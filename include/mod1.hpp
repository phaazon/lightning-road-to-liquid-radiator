#ifndef __MOD1_HPP
#define __MOD1_HPP

#include "shader.hpp"
#include "text_writer.hpp"

class mod1_c {
  /* text writer */
  text_writer_c &_textWriter;

  /* tunnel program */
  shader_c _tunFS;
  shader_c _thunVS;
  program_c _tunP;

  /* thunders field program */
  shader_c _thunTCS;
  shader_c _thunTES;
  shader_c _thunGS;
  shader_c _thunFS;
  program_c _thunP;
  GLuint _thunders;
  GLint _tunTimeIndex;
  GLint _thunTimeIndex;
  void _init_thunders(void);

  /* thunders blur program */
  shader_c _thunHBlurFS;
  program_c _thunHBlurP;
  shader_c _thunVBlurFS;
  program_c _thunVBlurP;

  /* offscreen */ 
  GLuint _offtex[3];
  GLuint _rdbf;
  GLuint _fbo[3];
  void _setup_offscreen(float width, float height);

  /* swap lines */
  shader_c _swapLinesFS;
  program_c _swapLinesP;
  GLint _swapLinesTimeIndex;

  void _init_uniforms(float width, float height);

public :
  mod1_c(float width, float height, text_writer_c &writer);
  ~mod1_c(void);

  void render(float time);
};

#endif /* guard */


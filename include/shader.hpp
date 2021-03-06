#ifndef __SHADER_HPP
#define __SHADER_HPP

#include <string>
#include "gl.hpp"

class shader_c {
  GLuint _;

public :
  shader_c(GLenum type);
  ~shader_c();

  GLuint id() const;
  void source(char const *src);
  void compile(void);
  bool compiled(void) const;
  std::string compile_log(void) const;
};

class program_c {
  GLuint _;

public :
  program_c(void);
  ~program_c(void);

  GLuint id() const;
  void attach(shader_c const &sh);
  void link(void);
  bool linked(void) const;
  std::string link_log(void) const;

  /* uniforms mapping */
  GLint map_uniform(char const *name) const;
};

#endif /* guard */


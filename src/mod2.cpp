#include <iostream>
#include "common.hpp"
#include "mod2.hpp"

using namespace std;

mod2_c::mod2_c(program_c &cubeP, GLuint cube) :
    _stdFS(GL_FRAGMENT_SHADER)
  , _cubeP(cubeP)
  , _cube(cube)
  , _rgbaFS(GL_FRAGMENT_SHADER) {
  _stdFS.source(load_source(STD_LAVA_FS_PATH).c_str());
  _stdFS.compile();
  if (!_stdFS.compiled()) {
    cerr << "STD Lava fragment shader failed to compile:\n" << _stdFS.compile_log() << endl;
    exit(1);
  }
  _stdP.attach(_stdFS);
  _stdP.link();
  if (!_stdP.linked()) {
    cerr << "STD Lava shader program failed to link:\n" << _stdP.link_log() << endl;
    exit(2);
  }
  _rgbaFS.source(load_source(LAVA_RGBA_FS_PATH).c_str());
  _rgbaFS.compile();
  if (!_rgbaFS.compiled()) {
    cerr << "Lava RGBA shader program failed to link:\n" << _rgbaFS.compile_log() << endl;
    exit(1);
  }
  _rgbaP.attach(_rgbaFS);
  _rgbaP.link();
  if (!_rgbaP.linked()) {
    cerr << "Lava RGBA program failed to link:\n" << _rgbaP.link_log() << endl;
    exit(2);
  }

  _init_uniforms();
  _setup_offscreen();
}

mod2_c::~mod2_c() {
}

void mod2_c::_setup_offscreen() {
  /* prepare the offscreen texture */
  glGenTextures(1, &_offtex);
  glBindTexture(GL_TEXTURE_2D, _offtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* prepare the renderbuffer */
  glGenRenderbuffers(1, &_rdbf);
  glBindRenderbuffer(GL_RENDERBUFFER, _rdbf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  /* prepare the FBO */
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rdbf);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _offtex, 0);

  auto ok = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  switch (ok) {
    case GL_FRAMEBUFFER_COMPLETE :
      cout << "framebuffer complete" << endl;
      break;

    default :
      cerr << "framebuffer incomplete" << endl;
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void mod2_c::_init_uniforms() {
  /* lava */
  glUseProgram(_stdP.id());
  _stdTimeIndex = _stdP.map_uniform("time");
  _stdRes = _stdP.map_uniform("res");
  _stdFovy = _stdP.map_uniform("fovy");
  glUniform4f(_stdRes, WIDTH, HEIGHT, IWIDTH, IHEIGHT);
  glUniform1f(_stdFovy, FOVY);

  /* cube */
  glUseProgram(_cubeP.id());
  _cubeTimeIndex = _cubeP.map_uniform("time");

  /* offscreen */
  glUseProgram(_rgbaP.id());
  auto rgbaOfftexIndex = _rgbaP.map_uniform("offtex");
  auto rgbaResIndex = _rgbaP.map_uniform("res");
  _rgbaTimeIndex = _rgbaP.map_uniform("time");
  glUniform1i(rgbaOfftexIndex, 0);
  glUniform4f(rgbaResIndex, WIDTH, HEIGHT, IWIDTH, IHEIGHT);
}

void mod2_c::render(float time) {
  glDisable(GL_BLEND);

  /* offscreen render */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

  glUseProgram(_stdP.id());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUniform1f(_stdTimeIndex, time);
  glRectf(-1.f, 1.f, 1.f, -1.f);

  if (time >= 109.7) { /* WOOUH THAT'S DIRTY! DO YOU THINK SO, WELL I BETTER NOT SHOW YOU WHERE’S THE LIMONADE IS MADE, SWEET LIMONADE */
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glUseProgram(_cubeP.id());
    glUniform1f(_cubeTimeIndex, time);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(_cube);
    glDrawElements(GL_PATCHES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  /* RGBA distor */
  glUseProgram(_rgbaP.id());
  glBindTexture(GL_TEXTURE_2D, _offtex);
  glUniform1f(_rgbaTimeIndex, time);
  glRectf(-1.f, 1.f, 1.f, -1.f);
}

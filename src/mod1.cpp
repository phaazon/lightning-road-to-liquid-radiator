#include <cstdlib>
#include <iostream>
#include "common.hpp"
#include "matrix.hpp"
#include "mod1.hpp"

/* shaders sources */
#include "data/tunnel-fs.hpp"
#include "data/thun-vs.hpp"
#include "data/thun-tcs.hpp"
#include "data/thun-tes.hpp"
#include "data/thun-fs.hpp"
#include "data/thun-hblur-fs.hpp"
#include "data/thun-vblur-fs.hpp"
#include "data/swap_lines-fs.hpp"

using namespace std;

namespace {
  int const THUNDERS_NB = 50;
  int const THUNDERS_VERTICES_NB = THUNDERS_NB*2; 
  int const BLUR_PASSES = 3;
}

mod1_c::mod1_c(float width, float height, text_writer_c &writer) :
    _textWriter(writer)
  , _tunFS(GL_FRAGMENT_SHADER)
  , _thunVS(GL_VERTEX_SHADER)
  , _thunTCS(GL_TESS_CONTROL_SHADER)
  , _thunTES(GL_TESS_EVALUATION_SHADER)
  , _thunGS(GL_GEOMETRY_SHADER)
  , _thunFS(GL_FRAGMENT_SHADER)
  , _thunHBlurFS(GL_FRAGMENT_SHADER)
  , _thunVBlurFS(GL_FRAGMENT_SHADER)
  , _swapLinesFS(GL_FRAGMENT_SHADER) {
  /* tunnel setup */
#if 0
  _tunFS.source(load_source(TUNNEL_FS_PATH).c_str());
#endif
  _tunFS.source(SHD_TUNNEL_FS);
  _tunFS.compile();
  if (!_tunFS.compiled()) {
    cerr << "Tunnel fragment shader failed to compile:\n" << _tunFS.compile_log() << endl;
    exit(1);
  }
  _tunP.attach(_tunFS);
  _tunP.link();
  if (!_tunP.linked()) {
    cerr << "Tunnel shader program failed to link:\n" << _tunP.link_log() << endl;
    exit(2);
  }

  /* thunders field setup */
#if 0
  _thunVS.source(load_source(THUN_VS_PATH).c_str());
#endif
  _thunVS.source(SHD_THUN_VS);
  _thunVS.compile();
  if (!_thunVS.compiled()) {
    cerr << "Thunder vertex shader failed to compile:\n" << _thunVS.compile_log() << endl;
    exit(1);
  }
#if 0
  _thunTCS.source(load_source(THUN_TCS_PATH).c_str());
#endif
  _thunTCS.source(SHD_THUN_TCS);
  _thunTCS.compile();
  if (!_thunTCS.compiled()) {
    cerr << "Thunder tessellation control shader failed to compile:\n" << _thunTCS.compile_log() << endl;
    exit(1);
  }
#if 0
  _thunTES.source(load_source(THUN_TES_PATH).c_str());
#endif
  _thunTES.source(SHD_THUN_TES);
  _thunTES.compile();
  if (!_thunTES.compiled()) {
    cerr << "Thunder tessellation evaluation shader failed to compile:\n" << _thunTES.compile_log() << endl;
    exit(1);
  }
#if 0
  _thunFS.source(load_source(THUN_FS_PATH).c_str());
#endif
  _thunFS.source(SHD_THUN_FS);
  _thunFS.compile();
  if (!_thunFS.compiled()) {
    cerr << "Thunder fragment shader failed to compile:\n" << _thunFS.compile_log() << endl;
    exit(1);
  }
  _thunP.attach(_thunVS);
  _thunP.attach(_thunTCS);
  _thunP.attach(_thunTES);
  _thunP.attach(_thunFS);
  _thunP.link();
  if (!_thunP.linked()) {
    cerr << "Thunder program failed to link:\n" << _thunP.link_log() << endl;
    exit(2);
  }

  /* thunders blur */
#if 0
  _thunBlurFS.source(load_source(THUN_BLUR_FS_PATH).c_str());
#endif
  _thunHBlurFS.source(SHD_THUN_HBLUR_FS);
  _thunHBlurFS.compile();
  if (!_thunHBlurFS.compiled()) {
    cerr << "Thunder horizontal blur fragment shader failed to compile:\n" << _thunHBlurFS.compile_log() << endl;
    exit(1);
  }
  _thunHBlurP.attach(_thunHBlurFS);
  _thunHBlurP.link();
  if (!_thunHBlurP.linked()) {
    cerr << "Thunder horizontal blur program failed to link:\n" << _thunHBlurP.link_log() << endl;
    exit(2);
  }
  _thunVBlurFS.source(SHD_THUN_VBLUR_FS);
  _thunVBlurFS.compile();
  if (!_thunVBlurFS.compiled()) {
    cerr << "Thunder vertical blur shader failed to compile:\n" << _thunVBlurFS.compile_log() << endl;
    exit(1);
  }
  _thunVBlurP.attach(_thunVBlurFS);
  _thunVBlurP.link();
  if (!_thunVBlurP.linked()) {
    cerr << "Thunder vertical blur program failed to link:\n" << _thunVBlurP.link_log() << endl;
    exit(2);
  }

  /* swap lines */
#if 0
  _swapLinesFS.source(load_source(SWAP_LINES_PATH_FS).c_str());
#endif
  _swapLinesFS.source(SHD_SWAP_LINES_FS);
  _swapLinesFS.compile();
  if (!_swapLinesFS.compiled()) {
    cerr << "Swap lines fragment shader failed to compile:\n" << _swapLinesFS.compile_log() << endl;
    exit(1);
  }
  _swapLinesP.attach(_swapLinesFS);
  _swapLinesP.link();
  if (!_swapLinesP.linked()) {
    cerr << "Swap lines program shader failed to link:\n" << _swapLinesP.link_log() << endl;
    exit(2);
  }

  _init_uniforms(width, height);
  _init_thunders();
  _setup_offscreen(width, height);
}

mod1_c::~mod1_c() {
}

void mod1_c::_setup_offscreen(float width, float height) {
  /* prepare the offscreen texture */
  glGenTextures(3, _offtex);
  for (int i = 0; i < 3; ++i) {
    glBindTexture(GL_TEXTURE_2D, _offtex[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  /* prepare the renderbuffer */
  glGenRenderbuffers(1, &_rdbf);
  glBindRenderbuffer(GL_RENDERBUFFER, _rdbf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  /* prepare the FBO */
  glGenFramebuffers(3, _fbo);
  for (int i = 0; i < 3; ++i) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[i]);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rdbf);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _offtex[i], 0);

#if DEBUG
    auto ok = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (ok) {
      case GL_FRAMEBUFFER_COMPLETE :
        cout << "framebuffer complete" << endl;
        break;

      default :
        cerr << "framebuffer incomplete" << endl;
    }
#endif
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void mod1_c::_init_thunders() {
  glGenVertexArrays(1, &_thunders);
  glBindVertexArray(_thunders);
  glBindVertexArray(0);
}

void mod1_c::_init_uniforms(float width, float height) {
  /* tunnel uniforms init */
  GLint tunResIndex;

  glUseProgram(_tunP.id());
  _tunTimeIndex = _tunP.map_uniform("time");
  tunResIndex = _tunP.map_uniform("res");
  glUniform4f(tunResIndex, width, height, 1.f/width, 1.f/height);

  /* thunders field init */
  glUseProgram(_thunP.id());
  auto projIndex = _thunP.map_uniform("proj");
  auto p = gen_perspective(FOVY, width/height, ZNEAR, ZFAR);

  glUniformMatrix4fv(projIndex, 1, GL_FALSE, p._);
  _thunTimeIndex = _thunP.map_uniform("time");

  /* thunders blur init */
  glUseProgram(_thunHBlurP.id());
  auto texBlurIndex = _thunHBlurP.map_uniform("offtex");
  auto thunBlurResIndex = _thunHBlurP.map_uniform("res");
  glUniform4f(thunBlurResIndex, width, height, 1.f/width, 1.f/height);
  glUniform1i(texBlurIndex, 0);
  glUseProgram(_thunVBlurP.id());
  auto texVBlurIndex = _thunVBlurP.map_uniform("offtex");
  auto thunVBlurResIndex = _thunVBlurP.map_uniform("res");
  glUniform4f(thunVBlurResIndex, width, height, 1.f/width, 1.f/height);
  glUniform1i(texVBlurIndex, 0);

  /* swap lines */
  glUseProgram(_swapLinesP.id());
  auto swapLinesRes = _swapLinesP.map_uniform("res");
  auto swapLinesOfftexIndex = _swapLinesP.map_uniform("offtex");
  _swapLinesTimeIndex = _swapLinesP.map_uniform("time");
  glUniform4f(swapLinesRes, width, height, 1.f/width, 1.f/height);
  glUniform1i(swapLinesOfftexIndex, 0);
}

void mod1_c::render(float time) {
  glEnable(GL_BLEND); /* maybe we'll need to place that in the bootstrap's initialization */
  glBlendFunc(GL_ONE, GL_ONE);

  /* tunnel render */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[2]);
  glUseProgram(_tunP.id());
  glUniform1f(_tunTimeIndex, time);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glRectf(-1.f, 1.f, 1.f, -1.f);

  /* thunders render */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[0]);
  glUseProgram(_thunP.id());
  glPatchParameteriARB(GL_PATCH_VERTICES, 2);
  glUniform1f(_thunTimeIndex, time);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(_thunders);
  glDrawArrays(GL_PATCHES, 0, THUNDERS_VERTICES_NB);
  glBindVertexArray(0);

  /* thunders blur */
  for (int i = 0, id = 0; i < BLUR_PASSES; ++i) {
    /* horizontal blur */
    glUseProgram(_thunHBlurP.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[1]);
    glBindTexture(GL_TEXTURE_2D, _offtex[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRectf(-1.f, 1.f, 1.f, -1.f);
    /* vertical blur */
    glUseProgram(_thunVBlurP.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[0]);
    glBindTexture(GL_TEXTURE_2D, _offtex[1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRectf(-1.f, 1.f, 1.f, -1.f);
    id = 1 - id;
  }

  /* combine blurred thunders to already rendered tunnel */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo[2]);
  glBindTexture(GL_TEXTURE_2D, _offtex[1]);
  glClear(GL_DEPTH_BUFFER_BIT);
  glRectf(-1.f, 1.f, 1.f, -1.f);

  /* swap lines */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glUseProgram(_swapLinesP.id());
  glBindTexture(GL_TEXTURE_2D, _offtex[2]);
  glUniform1f(_swapLinesTimeIndex, time);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glRectf(-1.f, 1.f, 1.f, -1.f);

  /* intro title */
  float t = time - 27.5f;
  _textWriter.start_draw();
  glClear(GL_DEPTH_BUFFER_BIT);
  _textWriter.draw_string("Lightning Road To Liquid Radiator", 1.f-t, 0.5f, 0.2f);
  _textWriter.end_draw();
}

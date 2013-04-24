#include <iostream>
#include "common.hpp"
#include "mod0.hpp"

using namespace std;

mod0_c::mod0_c() :
  _stdVS(GL_VERTEX_SHADER),
  _stdTCS(GL_TESS_CONTROL_SHADER),
  _stdTES(GL_TESS_EVALUATION_SHADER),
  _stdFS(GL_FRAGMENT_SHADER),
  _ppFS(GL_FRAGMENT_SHADER) {
  _stdVS.source(load_source(STD_VS_PATH).c_str());
  _stdVS.compile();
  if (!_stdVS.compiled()) {
    cerr << "mod0_c STD Vertex shader failed to compile:\n" << _stdVS.compile_log() << endl;
    exit(1);
  }
  _stdTCS.source(load_source(STD_TCS_PATH).c_str());
  _stdTCS.compile();
  if (!_stdTCS.compiled()) {
    cerr << "mod0_c STD Tessellation control shader failed to compile:\n" << _stdTCS.compile_log() << endl;
    exit(1);
  }
  _stdTES.source(load_source(STD_TES_PATH).c_str());
  _stdTES.compile();
  if (!_stdTCS.compiled()) {
    cerr << "mod0_c STD Tessellation evaluation shader failed to compile:\n" << _stdTES.compile_log() << endl;
    exit(1);
  }
  _stdFS.source(load_source(STD_FS_PATH).c_str());
  _stdFS.compile();
  if (!_stdFS.compiled()) {
    cerr << "mod0_c STD Fragment shader failed to compile:\n" << _stdFS.compile_log() << endl;
    exit(1);
  }
  _stdP.attach(_stdVS);
  _stdP.attach(_stdTCS);
  _stdP.attach(_stdTES);
  _stdP.attach(_stdFS);
  _stdP.link();
  if (!_stdP.linked()) {
    cerr << "mod0_c Program failed to link:\n" << _stdP.link_log() << endl;
    exit(2);
  }

  /* post-process program */
  _ppFS.source(load_source(PP_FS_PATH).c_str());
  _ppFS.compile();
  if (!_ppFS.compiled()) {
    cerr << "PP Fragment shader failed to compile:\n" << _ppFS.compile_log() << endl;
    exit(1);
  }
  _ppP.attach(_ppFS);
  _ppP.link();
  if (!_ppP.linked()) {
    cerr << "PP Program failed to link:\n" << _ppP.link_log() << endl;
    exit(2);
  }
  /* offscreen */
  _gen_offscreen_tex();
  _gen_rdbf();
  _gen_framebuffer();
  _setup_offscreen();
  /* cube generation */
  _gen_buffers();
  _gen_cube();
  /* uniforms */
  _init_uniforms();
  /* tessellation */
  _setup_tessellation();
}

mod0_c::~mod0_c() {
}

void mod0_c::_gen_offscreen_tex() {
  glGenTextures(1, &_offtex);
  glBindTexture(GL_TEXTURE_2D, _offtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void mod0_c::_gen_rdbf() {
  glGenRenderbuffers(1, &_rdbf);
  glBindRenderbuffer(GL_RENDERBUFFER, _rdbf);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void mod0_c::_gen_framebuffer() {
  glGenFramebuffers(1, &_fbo);
}

void mod0_c::_setup_offscreen() {
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

void mod0_c::_gen_buffers() {
  glGenBuffers(2, _cubeBuffers); /* 0 for VBO, 1 for IBO */
  glBindBuffer(GL_ARRAY_BUFFER, _cubeBuffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cubeBuffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES), CUBE_INDICES, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void mod0_c::_gen_cube() {
  GLint coid;

  coid = glGetAttribLocation(_stdP.id(), "co");
  if (coid != -1)
    cout << "'co' is active" << endl;

  glGenVertexArrays(1, &_cube);
  glBindVertexArray(_cube);
    glEnableVertexAttribArray(coid);
    glBindBuffer(GL_ARRAY_BUFFER, _cubeBuffers[0]);
    glVertexAttribPointer(coid, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cubeBuffers[1]);
  glBindVertexArray(0);
}

void mod0_c::_init_uniforms() {
  auto p = gen_perspective(FOVY, RATIO, ZNEAR, ZFAR);

  glUseProgram(_stdP.id());
  _projIndex = _stdP.map_uniform("proj");
  _stdTimeIndex = _stdP.map_uniform("time");
  glUniformMatrix4fv(_projIndex, 1, GL_FALSE, p._);

  glUseProgram(_ppP.id());
  _offtexIndex = _ppP.map_uniform("offtex");
  _ppResIndex = _ppP.map_uniform("res");
  _ppTimeIndex = _ppP.map_uniform("time");
  glUniform2f(_ppResIndex, WIDTH, HEIGHT);
  glUniform1i(_offtexIndex, 0);
}

void mod0_c::_setup_tessellation() {
  glUseProgram(_stdP.id());
  glPatchParameteri(GL_PATCH_VERTICES, 3);
}

void mod0_c::render(float time) {
  /* offscreen */
  glUseProgram(_stdP.id());
  glUniform1f(_stdTimeIndex, time);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(_cube);
  glDrawElements(GL_PATCHES, 36, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  /* post-process */
  glUseProgram(_ppP.id());
  glUniform1f(_ppTimeIndex, time);
  glBindTexture(GL_TEXTURE_2D, _offtex);
  glRectf(-1.f, 1.f, 1.f, -1.f);
  glUseProgram(0); /* end of frame */
}


#ifndef __CUBE_TCS_HPP
#define __CUBE_TCS_HPP

char const *SHD_CUBE_TCS =
"#version 330\n"
"#extension GL_ARB_tessellation_shader : enable\n"
"layout(vertices=3)out;"
"in vec3 gpos[];"
"out vec3 tpos[];"
"void main(){"
"tpos[gl_InvocationID]=gpos[gl_InvocationID];"
"if(gl_InvocationID==0){"
    "float t=12.;"
    "gl_TessLevelOuter[0]=t;"
    "gl_TessLevelOuter[1]=t;"
    "gl_TessLevelOuter[2]=t;"
    "gl_TessLevelInner[0]=t;"
  "}"
"}";

#endif /* guard */


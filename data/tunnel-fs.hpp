#ifndef __TUNNEL_FS_HPP
#define __TUNNEL_FS_HPP

char const *SHD_TUNNEL_FS =
"#version 330 core\n"
"out vec4 frag;"
"uniform vec4 res;"
"uniform float time;"
"const float PI=3.14159265359;"
"float fovy = PI/2.;"
"vec2 get_uv(){"
  "vec2 uv=2.*gl_FragCoord.xy*res.zw-1.;"
  "uv.y*=res.y*res.z;"
  "return uv;"
"}"
"float tunnel(vec3 ray,float r){"
  "return r/(1.+ray.z);"
"}"
"float tex(vec2 uv){"
  "return sin(uv.x*PI*8.+time*8.)+sin(uv.y*PI*8.)+0.4;"
"}"
"void main(){"
  "vec2 uv=get_uv();"
  "vec3 cam=vec3(0.,0.,1./tan(fovy/2.));"
  "vec3 ray=normalize(vec3(uv,0.)-cam);"
  "float hit=tunnel(ray,1.);"
  "float d=hit/100.;"
  "float pip=(ray.y<0.?-1.:1.);"
  "vec2 lookup=vec2(mod(hit*0.01,1.),acos(normalize(pip*ray.xy).x)/PI);"
  "float atten=max(1.,0.15*hit);"
  "float f=tex(lookup+vec2(0.,time/5.))/atten;"
  "frag=vec4(0.3*f*cos((time+PI)/2.),0.25*f, 0.2*f*sin(time),1.)*mod(gl_FragCoord.y,2.);"
  "frag+=vec4(atten/max(0.,20.*(time-27.5)));"
"}";

#endif /* guard */


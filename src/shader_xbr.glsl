R"(
#define XBR_EQ_THRESHOLD    15.0
#define XBR_LV2_COEFFICIENT  2.0
#define XBR_SCALE            4.0
#define lv2_cf XBR_LV2_COEFFICIENT

const vec4 Ao = vec4( 1.0, -1.0, -1.0,  1.0);
const vec4 Bo = vec4( 1.0,  1.0, -1.0, -1.0);
const vec4 Co = vec4( 1.5,  0.5, -0.5,  0.5);
const vec4 Ax = vec4( 1.0, -1.0, -1.0,  1.0);
const vec4 Bx = vec4( 0.5,  2.0, -0.5, -2.0);
const vec4 Cx = vec4( 1.0,  1.0, -0.5,  0.0);
const vec4 Ay = vec4( 1.0, -1.0, -1.0,  1.0);
const vec4 By = vec4( 2.0,  0.5, -2.0, -0.5);
const vec4 Cy = vec4( 2.0,  0.0, -1.0,  0.5);
const vec4 Ci = vec4(0.25, 0.25, 0.25, 0.25);

const vec4 delta   = vec4(1.0/XBR_SCALE);
const vec4 delta_l = vec4(0.5/XBR_SCALE, 1.0/XBR_SCALE, 0.5/XBR_SCALE, 1.0/XBR_SCALE);
const vec4 delta_u = vec4(1.0/XBR_SCALE, 0.5/XBR_SCALE, 1.0/XBR_SCALE, 0.5/XBR_SCALE);
const vec4 rcp2d   = vec4(XBR_SCALE*0.5);
const vec4 rcp2dl  = vec4(XBR_SCALE,     XBR_SCALE*0.5, XBR_SCALE,     XBR_SCALE*0.5);
const vec4 rcp2du  = vec4(XBR_SCALE*0.5, XBR_SCALE,     XBR_SCALE*0.5, XBR_SCALE    );

vec4 df4(vec4 A, vec4 B)  { return abs(A - B); }
vec4 diff(vec4 A, vec4 B) { return vec4(notEqual(A, B)); }
vec4 eq(vec4 A, vec4 B)   { return step(df4(A, B), vec4(XBR_EQ_THRESHOLD)); }
vec4 neq(vec4 A, vec4 B)  { return vec4(1.0) - eq(A, B); }
float c_df(vec3 c1, vec3 c2) { vec3 d = abs(c1-c2); return d.r+d.g+d.b; }

vec4 wd(vec4 a,vec4 b,vec4 c,vec4 d,vec4 e,vec4 f,vec4 g,vec4 h) {
    return df4(a,b)+df4(a,c)+df4(d,e)+df4(d,f)+4.0*df4(g,h);
}

vec4 xbrSample(sampler2D tex, vec2 uv) {
    float dx = 1.0/uTexSize.x, dy = 1.0/uTexSize.y;
    vec4 t1 = uv.xxxy + vec4(-dx,  0.0,  dx, -2.0*dy);
    vec4 t2 = uv.xxxy + vec4(-dx,  0.0,  dx,     -dy);
    vec4 t3 = uv.xxxy + vec4(-dx,  0.0,  dx,     0.0);
    vec4 t4 = uv.xxxy + vec4(-dx,  0.0,  dx,      dy);
    vec4 t5 = uv.xxxy + vec4(-dx,  0.0,  dx,  2.0*dy);
    vec4 t6 = uv.xyyy + vec4(-2.0*dx, -dy, 0.0,  dy);
    vec4 t7 = uv.xyyy + vec4( 2.0*dx, -dy, 0.0,  dy);

    vec3 A1=texture2D(tex,t1.xw).xyz, B1=texture2D(tex,t1.yw).xyz, C1=texture2D(tex,t1.zw).xyz;
    vec3 A =texture2D(tex,t2.xw).xyz, B =texture2D(tex,t2.yw).xyz, C =texture2D(tex,t2.zw).xyz;
    vec3 D =texture2D(tex,t3.xw).xyz, E =texture2D(tex,t3.yw).xyz, F =texture2D(tex,t3.zw).xyz;
    vec3 G =texture2D(tex,t4.xw).xyz, H =texture2D(tex,t4.yw).xyz, I =texture2D(tex,t4.zw).xyz;
    vec3 G5=texture2D(tex,t5.xw).xyz, H5=texture2D(tex,t5.yw).xyz, I5=texture2D(tex,t5.zw).xyz;
    vec3 A0=texture2D(tex,t6.xy).xyz, D0=texture2D(tex,t6.xz).xyz, G0=texture2D(tex,t6.xw).xyz;
    vec3 C4=texture2D(tex,t7.xy).xyz, F4=texture2D(tex,t7.xz).xyz, I4=texture2D(tex,t7.xw).xyz;

    const vec3 rgbw = vec3(14.352, 28.176, 5.472);
    vec4 b=vec4(dot(B,rgbw),dot(D,rgbw),dot(H,rgbw),dot(F,rgbw));
    vec4 c=vec4(dot(C,rgbw),dot(A,rgbw),dot(G,rgbw),dot(I,rgbw));
    vec4 d=b.yzwx, e=vec4(dot(E,rgbw)), f=b.wxyz, g=c.zwxy, h=b.zwxy, i=c.wxyz;
    vec4 i4=vec4(dot(I4,rgbw),dot(C1,rgbw),dot(A0,rgbw),dot(G5,rgbw));
    vec4 i5=vec4(dot(I5,rgbw),dot(C4,rgbw),dot(A1,rgbw),dot(G0,rgbw));
    vec4 h5=vec4(dot(H5,rgbw),dot(F4,rgbw),dot(B1,rgbw),dot(D0,rgbw));
    vec4 f4=h5.yzwx;

    vec2 fp=fract(uv*uTexSize);
    vec4 fx  =Ao*fp.y+Bo*fp.x;
    vec4 fx_l=Ax*fp.y+Bx*fp.x;
    vec4 fx_u=Ay*fp.y+By*fp.x;

    vec4 irlv0=diff(e,f)*diff(e,h);
    vec4 irlv1=irlv0*(neq(f,b)*neq(f,c)+neq(h,d)*neq(h,g)
               +eq(e,i)*(neq(f,f4)*neq(f,i4)+neq(h,h5)*neq(h,i5))+eq(e,g)+eq(e,c));
    vec4 irlv2l=diff(e,g)*diff(d,g);
    vec4 irlv2u=diff(e,c)*diff(b,c);

    vec4 fx45i=clamp((fx  +delta  -Co-Ci)*rcp2d,  vec4(0.0),vec4(1.0));
    vec4 fx45 =clamp((fx  +delta  -Co   )*rcp2d,  vec4(0.0),vec4(1.0));
    vec4 fx30 =clamp((fx_l+delta_l-Cx   )*rcp2dl, vec4(0.0),vec4(1.0));
    vec4 fx60 =clamp((fx_u+delta_u-Cy   )*rcp2du, vec4(0.0),vec4(1.0));

    vec4 wd1=wd(e,c, g,i,h5,f4,h,f);
    vec4 wd2=wd(h,d,i5,f,i4, b,e,i);
    vec4 edri =step(wd1,wd2)*irlv0;
    vec4 edr  =step(wd1+vec4(0.1),wd2)*step(vec4(0.5),irlv1);
    vec4 edr_l=step(lv2_cf*df4(f,g),df4(h,c))*irlv2l*edr;
    vec4 edr_u=step(lv2_cf*df4(h,c),df4(f,g))*irlv2u*edr;

    fx45 =edr  *fx45;
    fx30 =edr_l*fx30;
    fx60 =edr_u*fx60;
    fx45i=edri *fx45i;

    vec4 px=step(df4(e,f),df4(e,h));
    vec4 mx=max(max(fx30,fx60),max(fx45,fx45i));

    vec3 res1=E;
    res1=mix(res1,mix(H,F,px.x),mx.x);
    res1=mix(res1,mix(B,D,px.z),mx.z);
    vec3 res2=E;
    res2=mix(res2,mix(F,B,px.y),mx.y);
    res2=mix(res2,mix(D,H,px.w),mx.w);
    vec3 res=mix(res1,res2,step(c_df(E,res1),c_df(E,res2)));
    return vec4(res,1.0);
}
)"

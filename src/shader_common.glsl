R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform sampler2D uTextureMap;
uniform sampler2D uTextureMenu;
uniform sampler2D uTextureIntro;
uniform vec2      uTexSize;
uniform int       uFiltro;
uniform float     uEfecto;
uniform float     uBookState;
uniform float uFlipT; 

// ---- Efecto de paleta ----
vec4 applyEfecto(vec4 color) {
    float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec4 gris  = vec4(grey, grey, grey, color.a);
    vec4 fosfo = vec4(grey * 0.2, grey * 0.9, grey * 0.1, color.a);
    vec4 amber = vec4(grey * 0.9, grey * 0.5, 0.0,        color.a);
    vec4 res   = color;
    res = mix(res,  gris, step(0.5, uEfecto) * step(uEfecto, 1.5));
    res = mix(res, fosfo, step(1.5, uEfecto) * step(uEfecto, 2.5));
    res = mix(res, amber, step(2.5, uEfecto) * step(uEfecto, 3.5));
    return res;
}

// ---- Libro: parámetros ----
#define BOOK_MARGIN    0.03
#define PAGE_CONTENT   1.00
#define SPINE_WIDTH    0.05
#define CURVE_STRENGTH 0.022
#define CURVE_FALLOFF  0.18

vec4 paperColor() {
    return vec4(0.945, 0.941, 0.910, 1.0);
}

vec4 bgPattern(vec2 uv) {
    vec2  g    = fract(uv * 18.0);
    float pat  = clamp(step(0.92, g.x) + step(0.92, g.y)
                     + step(0.95, fract((uv.x + uv.y) * 12.0)), 0.0, 1.0) * 0.18;
    return vec4(0.149, 0.127, 0.361, 1.0) + vec4(pat);
}

#define CONTENT_RATIO (320.0 / 400.0)   // ancho/alto del contenido CPC real


bool debugThumbs(vec2 uv) {
    float thumbH = BOOK_MARGIN * 8.0;
    float thumbW = thumbH * CONTENT_RATIO;
    float gap    = (1.0 - 4.0 * thumbW) / 5.0;
    for (int i = 0; i < 4; i++) {
        float x0 = gap + float(i) * (thumbW + gap);
        float x1 = x0 + thumbW;
        float y0 = BOOK_MARGIN;
        float y1 = BOOK_MARGIN + thumbH;
        if (uv.x > x0 && uv.x < x1 && uv.y > y0 && uv.y < y1) {
            vec2 tUV = vec2((uv.x - x0) / thumbW, (uv.y - y0) / thumbH);
            if      (i == 0) gl_FragColor = texture2D(uTexture,      tUV);
            else if (i == 1) gl_FragColor = texture2D(uTextureMap,   tUV);
            else if (i == 2) gl_FragColor = texture2D(uTextureMenu,  tUV);
            else             gl_FragColor = texture2D(uTextureIntro, tUV);
            return true;
        }
    }
    return false;
}

)"

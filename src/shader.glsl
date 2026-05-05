R"(
#version 120
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uTexSize;
uniform int uFiltro;

void main() {
    vec2 uv = clamp(vTexCoord, vec2(0.0), vec2(1.0));
    vec2 ps = 1.0 / uTexSize;

    // Muestreo seguro (evita wrap en bordes)
    vec4 A = texture2D(uTexture, clamp(uv + vec2(-1.0, -1.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 B = texture2D(uTexture, clamp(uv + vec2( 0.0, -1.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 C = texture2D(uTexture, clamp(uv + vec2( 1.0, -1.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 D = texture2D(uTexture, clamp(uv + vec2(-1.0,  0.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 E = texture2D(uTexture, uv);
    vec4 F = texture2D(uTexture, clamp(uv + vec2( 1.0,  0.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 G = texture2D(uTexture, clamp(uv + vec2(-1.0,  1.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 H = texture2D(uTexture, clamp(uv + vec2( 0.0,  1.0) * ps, vec2(0.0), vec2(1.0)));
    vec4 I = texture2D(uTexture, clamp(uv + vec2( 1.0,  1.0) * ps, vec2(0.0), vec2(1.0)));

    // Si filtro desactivado, píxel crudo
    if (uFiltro < 1) { gl_FragColor = E; return; }

    float th = 0.25;
    vec4 th4 = vec4(th);
    vec4 diff(vec4 c1, vec4 c2) { return step(th4, abs(c1 - c2)); }

    vec4 edr = diff(E, D) * diff(E, H);
    vec4 ebr = diff(E, B) * diff(E, F);
    vec4 elr = diff(E, G) * diff(E, I);
    vec4 etr = diff(E, A) * diff(E, C);

    // MODO DIAGNÓSTICO: uFiltro=2
    if (uFiltro == 2) {
        vec4 edgeMask = vec4(0.0);
        if (all(lessThan(etr, vec4(0.5)))) edgeMask = vec4(1.0, 0.2, 0.2, 1.0); // Rojo
        if (all(lessThan(edr, vec4(0.5)))) edgeMask = vec4(0.2, 1.0, 0.2, 1.0); // Verde
        if (all(lessThan(ebr, vec4(0.5)))) edgeMask = vec4(0.2, 0.2, 1.0, 1.0); // Azul
        if (all(lessThan(elr, vec4(0.5)))) edgeMask = vec4(1.0, 1.0, 0.0, 1.0); // Amarillo
        gl_FragColor = mix(E, edgeMask, 0.7);
        return;
    }

    // MODO XBR NORMAL: uFiltro=1
    vec4 res = E;
    if (all(lessThan(etr, vec4(0.5)))) res = mix(E, mix(A, C, 0.5), 0.5);
    if (all(lessThan(edr, vec4(0.5)))) res = mix(E, mix(D, H, 0.5), 0.5);
    if (all(lessThan(ebr, vec4(0.5)))) res = mix(E, mix(B, F, 0.5), 0.5);
    if (all(lessThan(elr, vec4(0.5)))) res = mix(E, mix(G, I, 0.5), 0.5);

    gl_FragColor = res;
}
)";


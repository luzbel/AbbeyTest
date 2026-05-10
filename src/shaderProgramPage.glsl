R"(
//#ifdef GL_ES
//precision mediump float;
//#endif
//varying vec2 vTexCoord;
//uniform float uFlipT;
void main() {
    float angle  = uFlipT * 3.14159;
    float shadow = sin(angle) * 0.5;

    // Antes de 90°: tapa sobre página derecha, sombra cerca del lomo (x=0)
    // Después de 90°: tapa sobre página izquierda, sombra cerca del lomo (x=1)
    float shadowOnRight = shadow * (1.0 - vTexCoord.x) * step(uFlipT, 0.5);
    float shadowOnLeft  = shadow * vTexCoord.x          * step(0.5, uFlipT);

    float s = 1.0 - shadowOnRight - shadowOnLeft;
    gl_FragColor = vec4(s, 0.0, 0.0, 1.0);
}
)";

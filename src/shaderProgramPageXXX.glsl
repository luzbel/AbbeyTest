R"(
void main() {
    float angle = uFlipT * 3.14159;
    float light = uFlipT < 0.5
        ? mix(1.0, 0.7, uFlipT * 2.0)
        : mix(0.7, 1.0, (uFlipT - 0.5) * 2.0);

    // Letterbox: centrar 320x200 respetando ratio
    // El quad de la tapa es mitad de pantalla en X, pantalla completa en Y
    // Ratio del quad en pantalla: 0.5 * ww / hh — aproximamos con CONTENT_RATIO
    float scaleY = CONTENT_RATIO;  // ajustar si hace falta
    float mY = (1.0 - scaleY) * 0.5;
    vec2 uv = vec2(vTexCoord.x, (vTexCoord.y - mY) / scaleY);
    bool outside = uv.y < 0.0 || uv.y > 1.0;

    vec4 color;
    if (outside) {
        color = paperColor();
    } else {
        color = uFlipT < 0.5
            ? texture2D(PAGE_RIGHT,    uv)
            : texture2D(NEXT_PAGE_LEFT, uv);
    }
    gl_FragColor = vec4(color.rgb * light, 1.0);
}
)";

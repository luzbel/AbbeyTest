R"(
void main() {
    vec2 uv = vTexCoord;

gl_FragColor = bgPattern(uv);  // fondo por defecto
    if (debugThumbs(uv)) return;
    renderCover(uv);  // sobreescribe solo donde toca
}
)";

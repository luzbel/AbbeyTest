R"(
void main() {
    float angle = uFlipT * 3.14159;
    float light = uFlipT < 0.5
        ? mix(1.0, 0.7, uFlipT * 2.0)
        : mix(0.7, 1.0, (uFlipT - 0.5) * 2.0);

    vec4 color = uFlipT < 0.5
        ? texture2D(PAGE_RIGHT, vTexCoord)
        : texture2D(NEXT_PAGE_LEFT, vTexCoord);

    gl_FragColor = vec4(color.rgb * light, 1.0);
}
)";

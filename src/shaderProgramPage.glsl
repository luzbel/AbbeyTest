R"(
void main() {
    float angle = uFlipT * 3.14159;
    float shadow = sin(angle) * 0.5;
    float shadowOnRight = shadow * (1.0 - vTexCoord.x) * step(uFlipT, 0.5);
    float shadowOnLeft  = shadow * vTexCoord.x          * step(0.5, uFlipT);
    float s = 1.0 - shadowOnRight - shadowOnLeft;

    vec4 color = vTexCoord.x < 0.5
        ? texture2D(PAGE_LEFT,  vTexCoord * 2.0)
        : texture2D(PAGE_RIGHT, (vTexCoord - vec2(0.5, 0.0)) * 2.0);

    gl_FragColor = vec4(color.rgb * s, 1.0);
}
)";

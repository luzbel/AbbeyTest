R"(
void main() {
    float angle = uFlipT * 3.14159;
    float shadow = sin(angle) * 0.5;
    float shadowOnRight = shadow * (1.0 - vTexCoord.x) * step(uFlipT, 0.5);
    float shadowOnLeft  = shadow * vTexCoord.x          * step(0.5, uFlipT);
    float s = 1.0 - shadowOnRight - shadowOnLeft;

    float scaleY = CONTENT_RATIO;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 uv = vec2(vTexCoord.x, (vTexCoord.y - mY) / scaleY);
    bool outside = uv.y < 0.0 || uv.y > 1.0;

    vec4 color = outside ? paperColor() : texture2D(NEXT_PAGE_RIGHT, uv);
    gl_FragColor = vec4(color.rgb * s, 1.0);
}
)";

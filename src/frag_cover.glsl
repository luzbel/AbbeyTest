R"(
void main() {
    float angle = uFlipT * 3.14159;
    float cosA  = cos(angle);
    float light = uFlipT < 0.5
        ? mix(1.0, 0.7, uFlipT * 2.0)
        : mix(0.7, 1.0, (uFlipT - 0.5) * 2.0);

    float quadRatio = 0.5;
    float scaleX = 1.0;
    float scaleY = quadRatio / CONTENT_RATIO;
    float mX = 0.0;
    float mY = (1.0 - scaleY) * 0.5;

    vec2 tc = vec2(vTexCoord.x, vTexCoord.y * 0.5 + 0.25);
    vec2 uv = vec2((tc.x - mX) / scaleX, (tc.y - mY) / scaleY);
    bool outside = uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;

    vec4 color;
    if (outside) {
        color = paperColor();
    } else if (uFlipT < 0.5) {
        color = sampleTex(PAGE_RIGHT, uv);
    } else {
        color = sampleTex(NEXT_PAGE_LEFT, vec2(1.0 - uv.x, uv.y));
    }
    gl_FragColor = vec4(color.rgb * light, 1.0);
}
)";

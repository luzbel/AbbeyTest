R"(
void main() {
    float angle = uFlipT * 3.14159;
    float shadow = sin(angle) * 0.5;
    float shadowOnRight = shadow * (1.0 - vTexCoord.x) * step(uFlipT, 0.5);
    float shadowOnLeft  = shadow * vTexCoord.x          * step(0.5, uFlipT);
    float s = 1.0 - shadowOnRight - shadowOnLeft;

/*    float scaleY = CONTENT_RATIO;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 uv = vec2(vTexCoord.x, (vTexCoord.y - mY) / scaleY);
    bool outside = uv.y < 0.0 || uv.y > 1.0;
*/

vec2 texCoord = vec2(vTexCoord.x, vTexCoord.y * 0.5 + 0.25);
// Quad ratio: ancho=0.9 NDC, alto=1.8 NDC -> ratio = 0.9/1.8 = 0.5
    // Textura ratio: 320/200 = 1.6
    // Textura más ancha que quad: ajustar por ancho, márgenes en Y
    float quadRatio = 0.5;
    float scaleX = 1.0;
    float scaleY = (quadRatio / CONTENT_RATIO);  // 0.5/1.6 = 0.3125
    float mX = 0.0;
    float mY = (1.0 - scaleY) * 0.5;
    //vec2 uv = vec2((vTexCoord.x - mX) / scaleX, (vTexCoord.y - mY) / scaleY);
    vec2 uv = vec2((texCoord.x - mX) / scaleX, (texCoord.y - mY) / scaleY);
    bool outside = uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;

    vec4 color = outside ? paperColor() : sampleTex(NEXT_PAGE_RIGHT, uv);
    gl_FragColor = vec4(color.rgb * s, 1.0);
}
)";

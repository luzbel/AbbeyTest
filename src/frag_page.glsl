R"(
void main() {

    float angle = uFlipT * 3.14159;
    float shadow = sin(angle) * 0.5;

    bool isLeft = vTexCoord.x < 0.5;

    // Sombra: antes de 90° sobre derecha, después sobre izquierda
    float shadowOnRight = shadow * (1.0 - vTexCoord.x) * step(uFlipT, 0.5);
    float shadowOnLeft  = shadow * vTexCoord.x          * step(0.5, uFlipT);
//    float s = 1.0 - shadowOnRight - shadowOnLeft;
float s=1.0;

    // Remapear cada mitad a 0..1
    vec2 pageUV = isLeft
        ? vec2(vTexCoord.x * 2.0,        vTexCoord.y)
        : vec2((vTexCoord.x - 0.5) * 2.0, vTexCoord.y);

    vec2 texCoord = vec2(pageUV.x, pageUV.y * 0.5 + 0.25);

    float quadRatio = 0.5;
    float scaleX = 1.0;
    float scaleY = quadRatio / CONTENT_RATIO;
    float mX = 0.0;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 uv = vec2((texCoord.x - mX) / scaleX, (texCoord.y - mY) / scaleY);
    bool outside = uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
/*ok
    vec4 color = outside ? paperColor()
        : (isLeft ? 
		sampleTex(PAGE_LEFT, uv) : 
		sampleTex(NEXT_PAGE_RIGHT, uv)); 
*/
/*
    vec4 color;
    if (outside) {
        color = paperColor();
    } else if (isLeft && uPortada==1) {
        discard;
    } else {
        color = isLeft ? sampleTex(PAGE_LEFT, uv) : sampleTex(NEXT_PAGE_RIGHT, uv);
    }
*/

//    vec4 color = outside ? paperColor()
//        : (isLeft ? 
//		((uPortada>0)?vec4(1.0,0,0,1.0):sampleTex(PAGE_LEFT, uv)) : 
//		sampleTex(NEXT_PAGE_RIGHT, uv)); 
//vec4 color=outside?paperColor():isLeft?vec4(0.0,0.0,1.0,1.0):vec4(1.0,0.0,0.0,1.0);
//vec4 color=isLeft?(uPortada>0)?vec4(1.0,0.0,0.0,1.0):vec4(0.0,1.0,0.0,1.0):outside?paperColor():vec4(0.0,0.0,1.0,1.0);
//vec4 color=(uPortada>0)?vec4(1.0,0.0,0.0,1.0):vec4(0.0,0.0,1.0,1.0);
/*
    vec4 color = (uPortada>0)?vec4(1.0,0.0,0.0,1.0):outside ? paperColor()
        : (isLeft ? 
		sampleTex(PAGE_LEFT, uv) : 
		sampleTex(NEXT_PAGE_RIGHT, uv)); 
*/
/*
vec4 color = outside ? paperColor() : 
             (isLeft && uPortada > 0 ) ? vec4(1.0,0.0,0.0,1.0) : 
             (isLeft ? sampleTex(PAGE_LEFT, uv) : sampleTex(NEXT_PAGE_RIGHT, uv)); */

	

//    gl_FragColor = vec4(color.rgb * s, 1.0);

vec4 bgColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 color;
bool coverClosed = (uPortada == 1);

if (outside) {
    color = (coverClosed && isLeft) ? bgColor : paperColor();
} else if (coverClosed && isLeft) {
    color = bgColor;
} else {
    color = isLeft ? sampleTex(PAGE_LEFT, uv) : sampleTex(NEXT_PAGE_RIGHT, uv);
}
gl_FragColor = vec4(color.rgb * s, 1.0);
}
)";

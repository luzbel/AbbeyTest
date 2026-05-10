R"(
void main() {

/*
    vec2 uv = vTexCoord;
	if (debugThumbs(uv)) return;

    // Fondo fuera del libro
    if (uv.x < BOOK_MARGIN || uv.x > 1.0 - BOOK_MARGIN ||
        uv.y < BOOK_MARGIN || uv.y > 1.0 - BOOK_MARGIN) {
        gl_FragColor = bgPattern(uv);
        return;
    } 
    // Interior del libro
    vec2  inner     = (uv - vec2(BOOK_MARGIN)) / vec2(1.0 - 2.0 * BOOK_MARGIN);
    float spineHalf = SPINE_WIDTH * 0.5;
    float cx        = inner.x;

    // Lomo
    if (cx > 0.5 - spineHalf && cx < 0.5 + spineHalf) {
        float t = (cx - (0.5 - spineHalf)) / SPINE_WIDTH;
        float v = 0.3 + 0.4 * abs(t - 0.5) * 2.0;
        gl_FragColor = vec4(v*0.55, v*0.48, v*0.72, 1.0);
        return;
    }

    bool  isLeft = (cx < 0.5);
    float pageX  = isLeft
        ? cx / (0.5 - spineHalf)
        : (cx - (0.5 + spineHalf)) / (0.5 - spineHalf);
    vec2 pageLoc = vec2(pageX, inner.y);

    // Sombra del lomo
    float edgeDist = isLeft ? pageX : (1.0 - pageX);
    float shadow   = (1.0 - smoothstep(0.0, 0.12, edgeDist)) * 0.35;

    // Centrar 320x200 en la página respetando ratio
    float pageW     = (0.5 - spineHalf - BOOK_MARGIN);
    float pageH     = (1.0 - 2.0 * BOOK_MARGIN);
    float pageRatio = pageW / pageH;
    float scaleX, scaleY;
    if (CONTENT_RATIO > pageRatio) {
        scaleX = 1.0;
        scaleY = pageRatio / CONTENT_RATIO;
    } else {
        scaleY = 1.0;
        scaleX = CONTENT_RATIO / pageRatio;
    }

    float mX = (1.0 - scaleX) * 0.5;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 contentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);

    bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 ||
                   contentUV.y < 0.0 || contentUV.y > 1.0;

    if (outside) {
        gl_FragColor = paperColor();
        return;
    }

// sin xbr
//    vec4 color = isLeft
 //       ? texture2D(uTextureMap, contentUV)
 //       : texture2D(uTexture,    contentUV);


    vec4 color = isLeft
        ? (uFiltro < 1 ? applyEfecto(texture2D(PAGE_LEFT, contentUV)) : 
		applyEfecto(xbrSample(PAGE_LEFT, contentUV)))
        : (uFiltro < 1 ? applyEfecto(texture2D(PAGE_RIGHT,    contentUV)) : 
		applyEfecto(xbrSample(PAGE_RIGHT,    contentUV))); 



    color.rgb *= (1.0 - shadow);
    gl_FragColor = color; 
*/

    vec2 uv = vTexCoord;
//    if (debugThumbs(uv)) return;
//    gl_FragColor = renderBook(uv);

//    gl_FragColor = renderCover(uv);
/*
vec4 color = renderCover(uv);
    if (color.a < 0.5) gl_FragColor = bgPattern(uv);
    else gl_FragColor = color; */

gl_FragColor = bgPattern(uv);  // fondo por defecto
    if (debugThumbs(uv)) return;
    renderCover(uv);  // sobreescribe solo donde toca
}
)";

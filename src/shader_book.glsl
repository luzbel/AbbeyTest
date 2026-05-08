R"(
vec4 sampleCurrent(bool isLeft, vec2 uv) {
    return isLeft
        ? (uFiltro < 1 ? applyEfecto(texture2D(PAGE_LEFT,  uv)) : applyEfecto(xbrSample(PAGE_LEFT,  uv)))
        : (uFiltro < 1 ? applyEfecto(texture2D(PAGE_RIGHT, uv)) : applyEfecto(xbrSample(PAGE_RIGHT, uv)));
}

vec4 sampleNext(bool isLeft, vec2 uv) {
    return isLeft
        ? (uFiltro < 1 ? applyEfecto(texture2D(NEXT_PAGE_LEFT,  uv)) : applyEfecto(xbrSample(NEXT_PAGE_LEFT,  uv)))
        : (uFiltro < 1 ? applyEfecto(texture2D(NEXT_PAGE_RIGHT, uv)) : applyEfecto(xbrSample(NEXT_PAGE_RIGHT, uv)));
}

vec4 pageCurl(vec2 pageLoc, bool isLeft, vec2 contentUV, float t,
              float mX, float mY, float scaleX, float scaleY) {
    float angle  = 0.4;
    float radius = 0.06;
    vec2  dir    = vec2(cos(angle), sin(angle));
    float edge   = 1.0 - t;
    float proj   = dot(pageLoc - vec2(edge, 0.5), dir);

    if (proj < -radius) {
        return sampleCurrent(isLeft, contentUV);
    }

    if (proj < radius) {
        float localT    = (proj + radius) / (2.0 * radius);
        float curl      = sin(localT * 3.14159 * 0.5);
        float shadow    = 1.0 - curl * 0.4;
        // Desplazar pageLoc completo y recalcular contentUV
        vec2 curledPage = pageLoc + dir * curl * 0.05;
        vec2 curledUV   = (curledPage - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outside    = curledUV.x < 0.0 || curledUV.x > 1.0 ||
                          curledUV.y < 0.0 || curledUV.y > 1.0;
        vec4 c = outside ? paperColor() : sampleCurrent(isLeft, curledUV);
        c.rgb *= shadow;
        return c;
    }

    float shadow  = 0.7 + 0.3 * min((proj - radius) / 0.3, 1.0);
    // Zona levantada: desplazar también
    vec2 nextPage = pageLoc + dir * radius * 0.1;
    vec2 nextUV   = (nextPage - vec2(mX, mY)) / vec2(scaleX, scaleY);
    bool outside  = nextUV.x < 0.0 || nextUV.x > 1.0 ||
                    nextUV.y < 0.0 || nextUV.y > 1.0;
    vec4 c = outside ? paperColor() : sampleNext(isLeft, nextUV);
    c.rgb *= shadow;
    return c;
}

vec4 renderBook(vec2 uv) {
    if (uv.x < BOOK_MARGIN || uv.x > 1.0 - BOOK_MARGIN ||
        uv.y < BOOK_MARGIN || uv.y > 1.0 - BOOK_MARGIN) {
        return bgPattern(uv);
    }

    vec2  inner     = (uv - vec2(BOOK_MARGIN)) / vec2(1.0 - 2.0 * BOOK_MARGIN);
    float spineHalf = SPINE_WIDTH * 0.5;
    float cx        = inner.x;

    if (cx > 0.5 - spineHalf && cx < 0.5 + spineHalf) {
        float t = (cx - (0.5 - spineHalf)) / SPINE_WIDTH;
        float v = 0.3 + 0.4 * abs(t - 0.5) * 2.0;
        return vec4(v*0.55, v*0.48, v*0.72, 1.0);
    }

    bool  isLeft = (cx < 0.5);
    float pageX  = isLeft
        ? cx / (0.5 - spineHalf)
        : (cx - (0.5 + spineHalf)) / (0.5 - spineHalf);
    vec2 pageLoc = vec2(pageX, inner.y);

    float edgeDist = isLeft ? pageX : (1.0 - pageX);
    float spineShadow = (1.0 - smoothstep(0.0, 0.12, edgeDist)) * 0.35;

    float pageW     = 0.5 - spineHalf - BOOK_MARGIN;
    float pageH     = 1.0 - 2.0 * BOOK_MARGIN;
    float pageRatio = pageW / pageH;
    float scaleX, scaleY;
    if (CONTENT_RATIO > pageRatio) {
        scaleX = 1.0; scaleY = pageRatio / CONTENT_RATIO;
    } else {
        scaleY = 1.0; scaleX = CONTENT_RATIO / pageRatio;
    }
    float mX = (1.0 - scaleX) * 0.5;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 contentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
    bool outside   = contentUV.x < 0.0 || contentUV.x > 1.0 ||
                     contentUV.y < 0.0 || contentUV.y > 1.0;

if (isLeft) {
        vec4 c = outside ? paperColor() : sampleCurrent(true, contentUV);
        c.rgb *= (1.0 - spineShadow);
        return c;
    }

    // Página derecha girando hacia la izquierda
    float edge   = 1.0 - uFlipT;
    float radius = 0.04;

    if (pageX > edge + radius) {
        // Zona todavía visible de la página actual
        vec4 c = outside ? paperColor() : sampleCurrent(false, contentUV);
        c.rgb *= (1.0 - spineShadow);
        return c;
    }

    if (pageX > edge - radius) {
        // Zona del curl
        float localT  = (pageX - (edge - radius)) / (2.0 * radius);
        float curl    = sin(localT * 3.14159 * 0.5);
        float shadow  = curl * 0.4;
        vec2 curledLoc = vec2(pageLoc.x + curl * 0.05, pageLoc.y);
        vec2 curledUV  = (curledLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outC      = curledUV.x < 0.0 || curledUV.x > 1.0 ||
                         curledUV.y < 0.0 || curledUV.y > 1.0;
        vec4 c = outC ? paperColor() : sampleCurrent(false, curledUV);
        c.rgb *= (1.0 - spineShadow - shadow);
        return c;
    }

    // Zona ya girada: reverso de la hoja = NEXT_PAGE_LEFT
    float s = 0.7 + 0.3 * (edge - radius - pageX) / max(edge, 0.001);
    vec4 c  = outside ? paperColor() : sampleNext(true, contentUV);
    c.rgb  *= s * (1.0 - spineShadow);
    return c;
}

)"

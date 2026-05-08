R"(
// --- Helpers de muestreo (reutiliza applyEfecto y xbrSample de common) ---
vec4 sampleCoverFront(vec2 uv) {
    return (uFiltro < 1) ? applyEfecto(texture2D(PAGE_RIGHT, uv)) : applyEfecto(xbrSample(PAGE_RIGHT, uv));
}
vec4 sampleCoverBack(vec2 uv) {
    return (uFiltro < 1) ? applyEfecto(texture2D(NEXT_PAGE_LEFT, uv)) : applyEfecto(xbrSample(NEXT_PAGE_LEFT, uv));
}
vec4 sampleNewRight(vec2 uv) {
    return (uFiltro < 1) ? applyEfecto(texture2D(NEXT_PAGE_RIGHT, uv)) : applyEfecto(xbrSample(NEXT_PAGE_RIGHT, uv));
}
vec4 sampleUnderLeft(vec2 uv) {
    return (uFiltro < 1) ? applyEfecto(texture2D(PAGE_LEFT, uv)) : applyEfecto(xbrSample(PAGE_LEFT, uv));
}

vec4 renderCover(vec2 uv) {
    if (uv.x < BOOK_MARGIN || uv.x > 1.0 - BOOK_MARGIN ||
        uv.y < BOOK_MARGIN || uv.y > 1.0 - BOOK_MARGIN) {
        return bgPattern(uv);
    }

    vec2 inner = (uv - vec2(BOOK_MARGIN)) / vec2(1.0 - 2.0 * BOOK_MARGIN);
    float spineHalf = SPINE_WIDTH * 0.5;
    float cx = inner.x;

    // Lomo
    if (cx > 0.5 - spineHalf && cx < 0.5 + spineHalf) {
        float t = (cx - (0.5 - spineHalf)) / SPINE_WIDTH;
        float v = 0.3 + 0.4 * abs(t - 0.5) * 2.0;
        return vec4(v*0.55, v*0.48, v*0.72, 1.0);
    }

    bool isLeft = cx < 0.5;
    float pageX = isLeft ? cx / (0.5 - spineHalf) : (cx - (0.5 + spineHalf)) / (0.5 - spineHalf);
    vec2 pageLoc = vec2(pageX, inner.y);

    // Escala y márgenes de contenido (320x200)
    float pageW = 0.5 - spineHalf - BOOK_MARGIN;
    float pageH = 1.0 - 2.0 * BOOK_MARGIN;
    float pageRatio = pageW / pageH;
    float scaleX, scaleY;
    if (CONTENT_RATIO > pageRatio) { scaleX = 1.0; scaleY = pageRatio / CONTENT_RATIO; }
    else { scaleY = 1.0; scaleX = CONTENT_RATIO / pageRatio; }
    float mX = (1.0 - scaleX) * 0.5;
    float mY = (1.0 - scaleY) * 0.5;

    // --- Giro rígido de la tapa ---
    float angle = uFlipT * 3.14159265;
    float cosA = cos(angle);
    float sinA = sin(angle);
    float pivot = 0.5;
    float coverW = 0.5;
    float coverEdge = pivot + coverW * cosA;

    float minX = min(pivot, coverEdge);
    float maxX = max(pivot, coverEdge);
    bool onCover = (cx >= minX && cx <= maxX);

    if (onCover) {
        // Coordenada local en la tapa (0 en lomo, 1 en borde exterior)
        float localX = (abs(cosA) > 0.001) ? (cx - pivot) / (coverW * cosA) : 0.5;
        // Compresión vertical sutil para simular perspectiva de plano rígido
        float perspY = (inner.y - 0.5) * (1.0 - 0.12 * sinA) + 0.5;
        vec2 coverUV = vec2(localX, perspY);

        vec2 contentUV = (coverUV - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 || contentUV.y < 0.0 || contentUV.y > 1.0;
        vec4 paper = paperColor();

        if (uFlipT < 0.5) {
            // Anverso: portada (PAGE_RIGHT)
            vec4 c = outside ? paper : sampleCoverFront(contentUV);
            // Atenuación progresiva mientras gira
            c.rgb *= mix(1.0, 0.88, uFlipT * 2.0);
            return c;
        } else {
            // Reverso: contraportada interior (NEXT_PAGE_LEFT)
            vec4 c = outside ? paper : sampleCoverBack(contentUV);
            // Recuperación de iluminación al completar el giro
            c.rgb *= mix(0.88, 1.0, (uFlipT - 0.5) * 2.0);
            return c;
        }
    }

    // --- Páginas expuestas bajo la tapa ---
    // Sombra proyectada por la tapa sobre la nueva página derecha
    float shadow = 0.0;
    if (!isLeft && uFlipT > 0.0 && uFlipT < 1.0) {
        float distFromEdge = cx - coverEdge;
        shadow = smoothstep(0.0, 0.18, distFromEdge) * sinA * 0.45;
    }

    if (isLeft) {
        vec2 contentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 || contentUV.y < 0.0 || contentUV.y > 1.0;
        return outside ? paperColor() : sampleUnderLeft(contentUV);
    } else {
        vec2 contentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 || contentUV.y < 0.0 || contentUV.y > 1.0;
        vec4 c = outside ? paperColor() : sampleNewRight(contentUV);
        c.rgb *= (1.0 - shadow);
        return c;
    }
}
)"

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
    // Geometría básica del libro
    vec2 inner = (uv - vec2(BOOK_MARGIN)) / vec2(1.0 - 2.0 * BOOK_MARGIN);
    float spineHalf = SPINE_WIDTH * 0.5;
    float cx = inner.x;

    // Lomo
    if (cx > 0.5 - spineHalf && cx < 0.5 + spineHalf) {
        float t = (cx - (0.5 - spineHalf)) / SPINE_WIDTH;
        float v = 0.3 + 0.4 * abs(t - 0.5) * 2.0;
        return vec4(v*0.55, v*0.48, v*0.72, 1.0);
    }

    // --- Parámetros de giro ---
    float flipT = clamp(uFlipT, 0.0, 1.0);
    float angle = flipT * 3.14159265;
    float cosA = cos(angle);
    float sinA = abs(sin(angle)); // Magnitud para expansión trapecial

    // Límite X de la tapa en espacio normalizado
    float coverEdge = 0.5 + 0.5 * cosA;
    float minX = min(0.5, coverEdge);
    float maxX = max(0.5, coverEdge);
    bool inCoverX = cx >= minX && cx <= maxX;

    // Coordenada local X de la tapa (0=lomo, 1=borde exterior)
    float localX = 0.5;
    if (abs(cosA) > 0.001) {
        localX = (cx - 0.5) / (0.5 * cosA);
    }
    localX = clamp(localX, 0.0, 1.0);

    // --- Expansión trapecial en Y ---
    // El borde exterior se alarga verticalmente según sin(angle)
    float stretch = sinA * 0.45; 
    float halfH = 0.5 * (1.0 + stretch * localX);
    bool onCover = inCoverX && (abs(inner.y - 0.5) < halfH);

    // Escalado letterbox (idéntico al de las páginas)
    float pageW = 0.5 - spineHalf - BOOK_MARGIN;
    float pageH = 1.0 - 2.0 * BOOK_MARGIN;
    float pageRatio = pageW / pageH;
    float scaleX, scaleY;
    if (CONTENT_RATIO > pageRatio) { scaleX = 1.0; scaleY = pageRatio / CONTENT_RATIO; }
    else { scaleY = 1.0; scaleX = CONTENT_RATIO / pageRatio; }
    float mX = (1.0 - scaleX) * 0.5;
    float mY = (1.0 - scaleY) * 0.5;

    // Renderizar tapa si el fragmento pertenece a ella
    if (onCover) {
        // Mapeo UV con corrección trapecial
        vec2 coverUV;
        coverUV.x = localX;
        coverUV.y = ((inner.y - 0.5) / halfH) * 0.5 + 0.5;

        // Convertir a UV de contenido
        vec2 contentUV = (coverUV - vec2(mX, mY)) / vec2(scaleX, scaleY);
        bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 || contentUV.y < 0.0 || contentUV.y > 1.0;

        vec4 c = outside ? paperColor() : (flipT < 0.5 ? sampleCoverFront(contentUV) : sampleCoverBack(contentUV));
        
        // Atenuación de luz durante el giro
        float light = flipT < 0.5 ? mix(1.0, 0.82, flipT * 2.0) : mix(0.82, 1.0, (flipT - 0.5) * 2.0);
        c.rgb *= light;
        return c;
    }

    // --- Páginas expuestas (fuera de la tapa) ---
    float pageX = cx < 0.5 ? cx / (0.5 - spineHalf) : (cx - (0.5 + spineHalf)) / (0.5 - spineHalf);
    vec2 pageLoc = vec2(pageX, inner.y);
    vec2 pageContentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
    bool pageOutside = pageContentUV.x < 0.0 || pageContentUV.x > 1.0 || pageContentUV.y < 0.0 || pageContentUV.y > 1.0;

    // Sombra proyectada por la tapa
    float shadow = 0.0;
    if (flipT > 0.0 && flipT < 1.0) {
        float dist = cx - coverEdge;
        float sideMask = (flipT < 0.5) ? step(coverEdge, cx) : step(cx, coverEdge);
        shadow = smoothstep(0.0, 0.18, dist) * sinA * 0.5 * sideMask;
    }

    if (cx < 0.5) {
        return pageOutside ? paperColor() : sampleUnderLeft(pageContentUV);
    } else {
        vec4 c = pageOutside ? paperColor() : sampleNewRight(pageContentUV);
        c.rgb *= (1.0 - shadow);
        return c;
    }
}
)"

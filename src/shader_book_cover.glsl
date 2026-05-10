R"(
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

#define DEBUG_VISUALIZE

vec2 renderCover(vec2 uv) {
    float flipT = clamp(uFlipT, 0.0, 1.0);
    float cosA  = cos(flipT * 3.14159265);
    float edgeX = 0.5 + 0.5 * cosA;
    float minX  = min(0.5, edgeX);
    float maxX  = max(0.5, edgeX);

    // Límites verticales fijos del libro
    float yMin = 0.15;
    float yMax = 0.85;

    // 1. Página derecha (Roja)
    if (uv.x > 0.5 && uv.x < 1.0 && uv.y > yMin && uv.y < yMax) {
        gl_FragColor = vec4(1.0, 0.2, 0.2, 1.0);
    }

    // 2. Tapa (Azul)
    bool isClosed = (flipT <= 0.0);
    float cYMin = isClosed ? yMin : 0.0;
    float cYMax = isClosed ? yMax : 1.0;

    vec2 coverUV = uv;
    if (uv.x >= minX && uv.x <= maxX && uv.y >= cYMin && uv.y <= cYMax) {
        float projW = edgeX - 0.5;
        coverUV.x = abs(projW) > 0.001 ? (uv.x - 0.5) / projW : 0.5;
        coverUV.y = isClosed ? (uv.y - yMin) / (yMax - yMin) : uv.y;
        gl_FragColor = vec4(0.1, 0.3, 0.9, 1.0);
    }
    return coverUV;
}
)"

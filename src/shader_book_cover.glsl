R"(
// --- Helpers de muestreo (reutilizan applyEfecto/xbrSample) ---
vec4 sampleCoverFront(vec2 uv) { return (uFiltro < 1) ? applyEfecto(texture2D(PAGE_RIGHT, uv)) : applyEfecto(xbrSample(PAGE_RIGHT, uv)); }
vec4 sampleCoverBack(vec2 uv)  { return (uFiltro < 1) ? applyEfecto(texture2D(NEXT_PAGE_LEFT, uv)) : applyEfecto(xbrSample(NEXT_PAGE_LEFT, uv)); }
vec4 sampleNewRight(vec2 uv)   { return (uFiltro < 1) ? applyEfecto(texture2D(NEXT_PAGE_RIGHT, uv)) : applyEfecto(xbrSample(NEXT_PAGE_RIGHT, uv)); }
vec4 sampleUnderLeft(vec2 uv)  { return (uFiltro < 1) ? applyEfecto(texture2D(PAGE_LEFT, uv)) : applyEfecto(xbrSample(PAGE_LEFT, uv)); }

vec4 renderCover(vec2 uv) {
    float t = clamp(uFlipT, 0.0, 1.0); // 🔒 ANIMACIÓN SE DETIENE EN 1.0
    float angle = t * 3.14159265;
    float cosA = cos(angle);
    float sinA = sin(angle);

    // Fondo fuera del libro
    if (uv.x < BOOK_MARGIN || uv.x > 1.0 - BOOK_MARGIN || uv.y < BOOK_MARGIN || uv.y > 1.0 - BOOK_MARGIN)
        return bgPattern(uv);

    vec2 inner = (uv - vec2(BOOK_MARGIN)) / vec2(1.0 - 2.0 * BOOK_MARGIN);
    float spineHalf = SPINE_WIDTH * 0.5;
    float cx = inner.x;

    // 📐 LOMO DINÁMICO: se "comprime" visualmente cuando la tapa lo estira
    float dynamicSpineW = SPINE_WIDTH * (0.4 + 0.6 * abs(cosA));
    float dynSpineHalf = dynamicSpineW * 0.5;
    float spineLeft = 0.5 - dynSpineHalf;
    float spineRight = 0.5 + dynSpineHalf;

    if (cx > spineLeft && cx < spineRight) {
        float tSpine = (cx - spineLeft) / dynamicSpineW;
        // Simula tensión de encuadernación: más oscuro en el centro cuando está a 90°
        float tension = 1.0 - sinA * 0.25;
        float v = (0.25 + 0.5 * abs(tSpine - 0.5) * 2.0) * tension;
        return vec4(v*0.55, v*0.48, v*0.72, 1.0);
    }

    bool isLeft = cx < 0.5;
    float pageX = isLeft ? cx / (0.5 - spineHalf) : (cx - (0.5 + spineHalf)) / (0.5 - spineHalf);
    vec2 pageLoc = vec2(pageX, inner.y);

    // 📖 PROYECCIÓN DE LA TAPA: pivota en el borde derecho del lomo
    float pivotX = 0.5 + spineHalf;
    float coverW = 1.0 - pivotX;
    float coverEdge = pivotX + coverW * cosA; // Borde exterior proyectado
    bool onCover = (cx >= min(pivotX, coverEdge)) && (cx <= max(pivotX, coverEdge));

    if (onCover) {
        float localX = (cx - pivotX) / max(abs(coverW * cosA), 0.001);
        // Corrección perspectiva vertical
        float perspY = (inner.y - 0.5) * (1.0 - 0.1 * sinA) + 0.5;
        vec2 coverUV = vec2(clamp(localX, 0.0, 1.0), perspY);

        vec4 c;
        if (t < 0.5) {
            c = sampleCoverFront(coverUV);
            c.rgb *= mix(1.0, 0.85, t * 2.0); // Atenuación al girar
        } else {
            c = sampleCoverBack(coverUV);
            c.rgb *= mix(0.85, 1.0, (t - 0.5) * 2.0); // Recuperación al abrirse
        }

        // ✨ EFECTO DE FILO / GROSOR EN EL BORDE DE DOBLEZ
        float distToEdge = abs(cx - coverEdge);
        float edgeHighlight = smoothstep(0.015, 0.0, distToEdge) * 0.9 * sinA;
        float edgeShadow = smoothstep(0.04, 0.0, distToEdge) * 0.25 * sinA;
        c.rgb += edgeHighlight;
        c.rgb -= edgeShadow;
        return c;
    }

    // 📄 PÁGINAS BAJO LA TAPA
    float shadow = 0.0;
    if (!isLeft && t > 0.05 && t < 0.95) {
        float distFromEdge = cx - coverEdge;
        shadow = smoothstep(0.0, 0.2, distFromEdge) * sinA * 0.5;
    }

    // Mapeo de contenido 320x200 (reutilizado de tu lógica)
    float pageW = 0.5 - spineHalf - BOOK_MARGIN;
    float pageH = 1.0 - 2.0 * BOOK_MARGIN;
    float pageRatio = pageW / pageH;
    float scaleX = (CONTENT_RATIO > pageRatio) ? 1.0 : CONTENT_RATIO / pageRatio;
    float scaleY = (CONTENT_RATIO > pageRatio) ? pageRatio / CONTENT_RATIO : 1.0;
    float mX = (1.0 - scaleX) * 0.5;
    float mY = (1.0 - scaleY) * 0.5;
    vec2 contentUV = (pageLoc - vec2(mX, mY)) / vec2(scaleX, scaleY);
    bool outside = contentUV.x < 0.0 || contentUV.x > 1.0 || contentUV.y < 0.0 || contentUV.y > 1.0;
    vec4 paper = paperColor();

    if (isLeft) {
        return outside ? paper : sampleUnderLeft(contentUV);
    } else {
        vec4 c = outside ? paper : sampleNewRight(contentUV);
        c.rgb *= (1.0 - shadow);
        return c;
    }
}
)"

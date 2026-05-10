R"(
void main() {
    float angle = uFlipT * 3.14159;
    float light = uFlipT < 0.5
        ? mix(1.0, 0.7, uFlipT * 2.0)
        : mix(0.7, 1.0, (uFlipT - 0.5) * 2.0);

/*
    // Letterbox: centrar 320x200 respetando ratio
    // El quad de la tapa es mitad de pantalla en X, pantalla completa en Y
    // Ratio del quad en pantalla: 0.5 * ww / hh — aproximamos con CONTENT_RATIO
    float scaleY = CONTENT_RATIO;  // ajustar si hace falta
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

    vec4 color;
    if (outside) {
        color = paperColor();
    } else {
        /*color = uFlipT < 0.5
            ? sampleTex(PAGE_RIGHT,    uv)
            : sampleTex(NEXT_PAGE_LEFT, uv); */
	if (uFlipT<0.5) 
		color = sampleTex(PAGE_RIGHT,    uv);
	else {
		vec2 uvFlipped = vec2(1.0 - uv.x, uv.y);
		color = sampleTex(NEXT_PAGE_LEFT, uvFlipped);
	}
    }
    gl_FragColor = vec4(color.rgb * light, 1.0);
}
)";

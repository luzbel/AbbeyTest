R"(
void main() {
    gl_FragColor = uFlipT < 0.5
        ? vec4(0.0, 0.0, 1.0, 1.0)
        : vec4(0.0, 1.0, 0.0, 1.0);
}
)";

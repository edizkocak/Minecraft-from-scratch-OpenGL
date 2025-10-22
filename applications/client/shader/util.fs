
float linear_depth(float depth, float near, float far) {
    return (far * near) / (far + near - depth * (far - near));
}

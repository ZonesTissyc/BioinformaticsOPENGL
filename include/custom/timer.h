class Timer {
public:
    Timer() {
        lastTime = static_cast<float>(glfwGetTime());
    }

    float tick() {
        float now = static_cast<float>(glfwGetTime());
        delta = now - lastTime;
        lastTime = now;
        return delta;
    }

    float deltaTime() const { return delta; }

private:
    float lastTime = 0.0f;
    float delta = 0.0f;
};

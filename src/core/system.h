#pragma once

class Scene;

/**
 * @brief Base class for per-frame logic that operates on a Scene.
 */
class System
{
public:
    /**
     * @brief Constructs a system bound to a scene.
     * @param scene Scene this system will operate on.
     *
     * Scene is injected once here instead of being passed to execute()
     * on every call, and instead of using a Scene singleton, which
     * would make Scene harder to test in isolation.
     */
    explicit System(Scene& scene) : m_scene(scene) {}
    virtual ~System() = default;

    /**
     * @brief Runs one frame of this system's logic.
     * @param dt Time step in seconds.
     */
    virtual void execute(float dt) = 0;

protected:
    Scene& m_scene;
};

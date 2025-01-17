#pragma once

struct GameObject {
public:

	bool isClientSS = false;

    // Transform component
    vec2 position = vec2 { 0.0f, 0.0f };

	struct
	{
		vec2 initialPosition = vec2{ 0.0f, 0.0f };
		vec2 prevPosition = vec2{ 0.0f, 0.0f };
		float initialAngle = 0.0f;

		vec2  finalPosition = vec2{ 0.0f, 0.0f };
		float finalAngle = 0.0f;

		float secondsElapsed = 0.0f;
		float lerpMaxTime = 0.0f;
	} interpolation;

    // Render component
    vec2 pivot = vec2 { 0.5f, 0.5f };
    vec2 size = vec2 { 0.0f, 0.0f }; // NOTE(jesus): If equals 0, it takes the size of the texture
    float angle = 0.0f;
    vec4 color = vec4 { 1.0f, 1.0f, 1.0f, 1.0f }; // NOTE(jesus): The texture will tinted with this color
    Texture* texture = nullptr;
    int order = 0; // NOTE(jesus): determines the drawing order

    // Collider component
    Collider* collider = nullptr;

    // "Script" component
    Behaviour* behaviour = nullptr;

    // Network identity component
    uint32 networkId = 0; // NOTE(jesus): Only for network game objects

    // NOTE(jesus): Don't use in gameplay systems (use Instantiate, Destroy instead)
    enum State { NON_EXISTING,
        CREATING,
        UPDATING,
        DESTROYING };
    State state = NON_EXISTING;

    // Tag for custom usage
    uint32 tag = 0;

	// Gameplay
	uint32 life = 100;
	uint32 kills = 0;
	GameObject* lifebar = nullptr;

public:
    void write(OutputMemoryStream& packet) const;
    void read(const InputMemoryStream& packet);

private:
    void* operator new(size_t size) = delete;
    void operator delete(void* obj) = delete;

    void releaseComponents();
    friend class ModuleGameObject;
	friend class ReplicationManagerClient;
};

class ModuleGameObject : public Module {
public:
    // Virtual functions

    bool init() override;

    bool preUpdate() override;

    bool update() override;

    bool postUpdate() override;

    bool cleanUp() override;

    static GameObject* Instantiate();

    static void Destroy(GameObject* gameObject);

    GameObject gameObjects[MAX_GAME_OBJECTS] = {};
};

// NOTE(jesus): These functions are named after Unity functions

GameObject* Instantiate();

void Destroy(GameObject* gameObject);

#pragma once

struct Collider {
public:
    ColliderType type = ColliderType::None;
    GameObject* gameObject = nullptr;
    bool isTrigger = false;

public:
    void write(OutputMemoryStream& packet) const;
    void read(const InputMemoryStream& packet);
};

class ModuleCollision : public Module {
public:
    ///////////////////////////////////////////////////////////////////////
    // ModuleCollision public methods
    ///////////////////////////////////////////////////////////////////////

    Collider* addCollider(ColliderType type, GameObject* parent);

    void removeCollider(Collider* collider);

private:
    ///////////////////////////////////////////////////////////////////////
    // Module virtual methods
    ///////////////////////////////////////////////////////////////////////

    bool update() override;

    bool postUpdate() override;

    Collider colliders[MAX_COLLIDERS];
    Collider* activeColliders[MAX_COLLIDERS];
    uint32 activeCollidersCount = 0;

    friend class ModuleRender;
};

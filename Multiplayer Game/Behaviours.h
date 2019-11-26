#pragma once

struct Behaviour {
    GameObject* gameObject = nullptr;

    virtual void start() { }

    virtual void update() { }

    virtual void onInput(const InputController& input, bool isClient = false) { }

    virtual void onCollisionTriggered(Collider& c1, Collider& c2) { }
};

struct Spaceship : public Behaviour {
    void start() override
    {
        gameObject->tag = (uint32)(Random.next() * UINT_MAX);
    }

    void onInput(const InputController& input, bool isClient = false) override
    {
        if (input.horizontalAxis != 0.0f) {
            const float rotateSpeed = 180.0f;
            gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;
            isClient ? void() : NetworkUpdate(gameObject);
        }

        if (input.actionDown == ButtonState::Pressed) {
            const float advanceSpeed = 200.0f;
            gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;
			isClient ? void() : NetworkUpdate(gameObject);
        }

        if (!isClient && input.actionLeft == ButtonState::Press) {
            GameObject* laser = App->modNetServer->spawnBullet(gameObject);
            laser->tag = gameObject->tag;
        }
    }

    void onCollisionTriggered(Collider& c1, Collider& c2) override
    {
        if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag) {
            NetworkDestroy(c2.gameObject); // Destroy the laser

            // NOTE(jesus): spaceship was collided by a laser
            // Be careful, if you do NetworkDestroy(gameObject) directly,
            // the client proxy will poing to an invalid gameObject...
            // instead, make the gameObject invisible or disconnect the client.
			gameObject->life -= 25;
			if (gameObject->life == 0)
			{
				App->modNetServer->disconnectClient(gameObject);
			}
        }
    }
};

struct Laser : public Behaviour {
    float secondsSinceCreation = 0.0f;

    void update() override
    {
        const float pixelsPerSecond = 1000.0f;
        gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

        secondsSinceCreation += Time.deltaTime;

        NetworkUpdate(gameObject);

        const float lifetimeSeconds = 2.0f;
        if (secondsSinceCreation > lifetimeSeconds)
            NetworkDestroy(gameObject);
    }
};

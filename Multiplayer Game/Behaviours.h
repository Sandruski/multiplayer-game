#pragma once

struct Behaviour {
    GameObject* gameObject = nullptr;

    virtual void start() { }

    virtual void update() { }

    virtual void onInput(const InputController& input, bool isClient) { }

    virtual void onCollisionTriggered(Collider& c1, Collider& c2) { }
};

struct Spaceship : public Behaviour {

	float timer = 0.0f;
	bool die = false;

    void start() override
    {
        gameObject->tag = (uint32)(Random.next() * UINT_MAX);
    }

	void update() override 
	{ 
		if (die)
		{
			GameObject* orb = App->modNetServer->spawnOrb(gameObject);
			orb->tag = gameObject->tag;
			App->modNetServer->disconnectClient(gameObject);
			die = false;
		}
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
			GameObject* laser = App->modNetServer->spawnBullet(gameObject, gameObject->angle);
            laser->tag = gameObject->tag;
        }

		if (!isClient && input.actionRight == ButtonState::Press) {
			GameObject* laser = App->modNetServer->spawnBullet(gameObject, gameObject->angle + 180.0f);
			laser->tag = gameObject->tag;
		}

		const float cooldown = 5.0f;
		if (!isClient && input.actionUp == ButtonState::Press && timer >= cooldown) {
			GameObject* laser1 = App->modNetServer->spawnBullet(gameObject, gameObject->angle);
			laser1->tag = gameObject->tag;
			GameObject* laser2 = App->modNetServer->spawnBullet(gameObject, gameObject->angle + 180.0f);
			laser2->tag = gameObject->tag;
			GameObject* laser3 = App->modNetServer->spawnBullet(gameObject, gameObject->angle + 90.0f);
			laser3->tag = gameObject->tag;
			GameObject* laser4 = App->modNetServer->spawnBullet(gameObject, gameObject->angle - 90.0f);
			laser4->tag = gameObject->tag;

			GameObject* laser5 = App->modNetServer->spawnBullet(gameObject, gameObject->angle + 45.0f);
			laser5->tag = gameObject->tag;
			GameObject* laser6 = App->modNetServer->spawnBullet(gameObject, gameObject->angle - 45.0f);
			laser6->tag = gameObject->tag;
			GameObject* laser7 = App->modNetServer->spawnBullet(gameObject, gameObject->angle + 135.0f);
			laser7->tag = gameObject->tag;
			GameObject* laser8 = App->modNetServer->spawnBullet(gameObject, gameObject->angle - 135.0f);
			laser8->tag = gameObject->tag;

			timer = 0.0f;
		}

		timer += Time.deltaTime;
    }

    void onCollisionTriggered(Collider& c1, Collider& c2) override
    {
        if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag) {
			gameObject->life -= 25;
			NetworkUpdate(gameObject);

			if (gameObject->life == 0)
			{
				die = true;

				GameObject* killerGameObject = App->modNetServer->getClientGameObject(c2.gameObject->tag);
				if (killerGameObject != nullptr)
				{
					++killerGameObject->kills;
					NetworkUpdate(killerGameObject);
				}
			}

			NetworkDestroy(c2.gameObject); // Destroy the laser

			// NOTE(jesus): spaceship was collided by a laser
			// Be careful, if you do NetworkDestroy(gameObject) directly,
			// the client proxy will poing to an invalid gameObject...
			// instead, make the gameObject invisible or disconnect the client.
        }
		else if (c2.type == ColliderType::Orb && c2.gameObject->tag != gameObject->tag)
		{
			if (gameObject->life < 100)
			{
				gameObject->life += 25;
				NetworkUpdate(gameObject);
			}

			NetworkDestroy(c2.gameObject);
		}
    }
};

struct Laser : public Behaviour {
    float secondsSinceCreation = 0.0f;

    void update() override
    {
		if (App->modNetClient->isEnabled())
			return;

        const float pixelsPerSecond = 1000.0f;
        gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

        secondsSinceCreation += Time.deltaTime;

        NetworkUpdate(gameObject);

        const float lifetimeSeconds = 2.0f;
        if (secondsSinceCreation > lifetimeSeconds)
            NetworkDestroy(gameObject);
    }
};

struct Orb : public Behaviour {
	float secondsSinceCreation = 0.0f;

	void update() override
	{
		if (App->modNetClient->isEnabled())
			return;

		secondsSinceCreation += Time.deltaTime;

		NetworkUpdate(gameObject);

		const float lifetimeSeconds = 100.0f;
		if (secondsSinceCreation > lifetimeSeconds)
			NetworkDestroy(gameObject);
	}
};

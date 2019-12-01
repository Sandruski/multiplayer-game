#include "ModuleGameObject.h"
#include "Networks.h"

void GameObject::write(OutputMemoryStream& packet) const
{
    // Transform component
    packet.Write(position.x);
    packet.Write(position.y);

    // Render component
    packet.Write(pivot.x);
    packet.Write(pivot.y);
    packet.Write(size.x);
    packet.Write(size.y);
    packet.Write(angle);
    packet.Write(color.x);
    packet.Write(color.y);
    packet.Write(color.z);
    packet.Write(color.w);

    // Texture component
	std::string textureFilename = texture != nullptr ? texture->filename : "";
    packet.Write(textureFilename);

    packet.Write(order);

    // Collider component
	ColliderType colliderType = collider != nullptr ? collider->type : ColliderType::None;
    packet.Write(colliderType);
	if (collider != nullptr)
	{
		collider->write(packet);
	}

    // Behaviour component

    packet.Write(tag);

	packet.Write(life);
	packet.Write(kills);
}

void GameObject::read(const InputMemoryStream& packet)
{
	// set interpolation values
	//interpolation.secondsElapsed = 0.0f;
	//interpolation.initialPosition = position;
    //packet.Read(interpolation.finalPosition.x);
    //packet.Read(interpolation.finalPosition.y);
	packet.Read(position.x);
	packet.Read(position.y);

    // Render component
    packet.Read(pivot.x);
    packet.Read(pivot.y);
    packet.Read(size.x);
    packet.Read(size.y);

	packet.Read(angle);

    packet.Read(color.x);
    packet.Read(color.y);
    packet.Read(color.z);
    packet.Read(color.w);

    // Texture component
    std::string textureFilename;
    packet.Read(textureFilename);
    if (texture == nullptr) {
        if (textureFilename == "space_background.jpg") {
            texture = App->modResources->space;
        } else if (textureFilename == "asteroid1.png") {
            texture = App->modResources->asteroid1;
        } else if (textureFilename == "asteroid2.png") {
            texture = App->modResources->asteroid2;
        } else if (textureFilename == "spacecraft1.png") {
            texture = App->modResources->spacecraft1;
        } else if (textureFilename == "spacecraft2.png") {
            texture = App->modResources->spacecraft2;
        } else if (textureFilename == "spacecraft3.png") {
            texture = App->modResources->spacecraft3;
        } else if (textureFilename == "laser.png") {
            texture = App->modResources->laser;
        } else if (textureFilename == "orb.png") {
			texture = App->modResources->orb;
		}
    }

    packet.Read(order);

    // Collider component
    ColliderType type = ColliderType::None;
    packet.Read(type);
    if (collider == nullptr) {
        collider = App->modCollision->addCollider(type, this);
    }
	if (collider != nullptr)
	{
		collider->read(packet);
	}

    // Behaviour component
    if (behaviour == nullptr) {
        switch (type) {
        case ColliderType::Player: {
            behaviour = new Spaceship;
			lifebar = ModuleGameObject::Instantiate();
            break;
        }

        case ColliderType::Laser: {
            behaviour = new Laser;
            break;
        }

		case ColliderType::Orb: {
			behaviour = new Orb;
			break;
		}

        default: {
            ASSERT("Invalid collider type");
            break;
        }
        }

		if (behaviour != nullptr)
		{
			behaviour->gameObject = this;
		}
    }

    packet.Read(tag);

	packet.Read(life);
	if (lifebar != nullptr)
	{
		lifebar->position = vec2{ position.x, position.y - size.y / 2.0f };

		const float alpha = 0.8f;
		const int height = 8;
		if (life == 100)
		{
			lifebar->color = vec4{ 0.0f, 1.0f, 0.0f, alpha };
			lifebar->size = { 80, height };
		}
		else if (life == 75)
		{
			lifebar->color = vec4{ 1.0f, 1.0f, 0.0f, alpha };
			lifebar->size = { 60, height };
		}
		else if (life == 50)
		{
			lifebar->color = vec4{ 1.0f, 0.5f, 0.0f, alpha };
			lifebar->size = { 40, height };
		}
		else if (life == 25)
		{
			lifebar->color = vec4{ 1.0f, 0.0f, 0.0f, alpha };
			lifebar->size = { 20, height };
		}
	}

	packet.Read(kills);
}

void GameObject::releaseComponents()
{
    if (behaviour != nullptr) {
		delete behaviour;
        behaviour = nullptr;
    }
    if (collider != nullptr) {
        App->modCollision->removeCollider(collider);
        collider = nullptr;
    }
	if (lifebar != nullptr) {
		Destroy(lifebar);
		lifebar = nullptr;
	}
}

bool ModuleGameObject::init()
{
    return true;
}

bool ModuleGameObject::preUpdate()
{
    for (GameObject& gameObject : gameObjects) {
        if (gameObject.state == GameObject::NON_EXISTING)
            continue;

        if (gameObject.state == GameObject::DESTROYING) {
            gameObject.releaseComponents();
            gameObject = GameObject();
            gameObject.state = GameObject::NON_EXISTING;
        } else if (gameObject.state == GameObject::CREATING) {
            if (gameObject.behaviour != nullptr)
                gameObject.behaviour->start();
            gameObject.state = GameObject::UPDATING;
        }
    }

    return true;
}

bool ModuleGameObject::update()
{
    for (GameObject& gameObject : gameObjects) {
        if (gameObject.state == GameObject::UPDATING) {
            if (gameObject.behaviour != nullptr)
                gameObject.behaviour->update();
        }
    }

    return true;
}

bool ModuleGameObject::postUpdate()
{
    return true;
}

bool ModuleGameObject::cleanUp()
{
    for (GameObject& gameObject : gameObjects) {
        gameObject.releaseComponents();
    }

    return true;
}

GameObject* ModuleGameObject::Instantiate()
{
    for (GameObject& gameObject : App->modGameObject->gameObjects) {
        if (gameObject.state == GameObject::NON_EXISTING) {
            gameObject.state = GameObject::CREATING;
            return &gameObject;
        }
    }

    ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
    return nullptr;
}

void ModuleGameObject::Destroy(GameObject* gameObject)
{
    ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

    gameObject->state = GameObject::DESTROYING;
}

GameObject* Instantiate()
{
    GameObject* result = ModuleGameObject::Instantiate();
    return result;
}

void Destroy(GameObject* gameObject)
{
    ModuleGameObject::Destroy(gameObject);
}

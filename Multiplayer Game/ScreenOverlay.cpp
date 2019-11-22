#include "Networks.h"


void ScreenOverlay::enable()
{
	ASSERT(oldScene != nullptr && newScene != nullptr);
	oldScene->shouldUpdate = false;
	newScene->shouldUpdate = false;

	overlay = Instantiate();
	//overlay->color[0] = 0.0f;
	//overlay->color[1] = 0.0f;
	//overlay->color[2] = 0.0f;
	//overlay->color[3] = 1.0f;
	overlay->texture = App->modResources->background;
	overlay->order = 9999;
	//overlay->scene = this;

	transitionTimeElapsed = 0.0f;

	App->modUI->setInputsEnabled(false);
}

void ScreenOverlay::update()
{
	ASSERT(oldScene != nullptr && newScene != nullptr);

	overlay->size = { (float)Window.width, (float)Window.height };

	const float halfTransitionTime = transitionTimeMax * 0.5f;

	if (transitionTimeElapsed < halfTransitionTime)
	{
		overlay->color.a = transitionTimeElapsed / halfTransitionTime;
	}
	else
	{
		oldScene->enabled = false;
		newScene->enabled = true;

		overlay->color.a = 1.0f - (transitionTimeElapsed - halfTransitionTime) / halfTransitionTime;
		if (overlay->color.a < 0.0f) { overlay->color.a = 0.0f; }
	}

	if (transitionTimeElapsed > transitionTimeMax)
	{
		enabled = false;
		oldScene->shouldUpdate = true;
		newScene->shouldUpdate = true;
		oldScene = nullptr;
		newScene = nullptr;
	}

	transitionTimeElapsed += Time.deltaTime;
}

void ScreenOverlay::disable()
{
	App->modUI->setInputsEnabled(true);

	Destroy(overlay);
}

#include "Component.h"
#include "GameObject.h"
#include "Application.h"

Component::Component(Comp_Type t, GameObject* parent) :type(t), parent(parent)
{
	//uid = App->random->Int();
}

Component::~Component()
{
}

bool Component::Enable()
{
	if (!active)
	{
		active = true;
	}
	return active;
}

bool Component::Disable()
{
	if (active)
	{
		active = false;
	}
	return active;
}

void Component::preUpdate(float dt)
{
}

void Component::Update(float dt)
{
}

void Component::Draw()
{
}

void Component::ShowInspectorInfo()
{
}

Comp_Type Component::GetType() const
{
	return type;
}

bool Component::isActive() const
{
	return active;
}

void Component::Save(JSON_Object * object, std::string name) const
{
}

void Component::Load(const JSON_Object * object, std::string name)
{
}
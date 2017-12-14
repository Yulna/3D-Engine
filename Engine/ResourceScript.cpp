#include "ResourceScript.h"
#include "CSharpScript.h"


ResourceScript::ResourceScript(uint uid) : Resource(uid, Resource::Type::MATERIAL, Resource::State::UNLOADED)
{
	NumGameObjectsUseMe = 0;
	LOG("Recource Material Created!");
}

ResourceScript::~ResourceScript()
{
}

void ResourceScript::InitInfo(std::string path_dll_, std::string path_assets_)
{
	path_dll = path_dll_;
	path_assets = path_assets_;
}

void ResourceScript::SetCSharp(CSharpScript* csharp_)
{
	csharp = csharp_;
}

bool ResourceScript::Update(float dt)
{
	csharp->DoMainFunction(FunctionBase::CS_Update);
	return false;
}

void ResourceScript::DeleteToMemory()
{
	state = Resource::State::UNLOADED;
	//glDeleteTextures(1, &texture.id);
	LOG("UnLoaded Recource Script");
}

bool ResourceScript::LoadToMemory()
{
	state = Resource::State::LOADED;
	LOG("Loaded Recource Script");
	return true;
}

Resource::State ResourceScript::IsCompiled()
{
	return state;
}

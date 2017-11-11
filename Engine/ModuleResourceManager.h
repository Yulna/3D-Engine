#pragma once
#include "Module.h"
#include "Resource_.h"
#include <map>

#define PRIMITIVERESERVE 2

struct ResourceImport
{
	const char* name = nullptr;
	uint uuid = 0;
	Resource::Type type = Resource::Type::UNKNOWN;
};

class ModuleResourceManager : public Module
{
public:
	ModuleResourceManager(bool start_enabled = true);
	virtual ~ModuleResourceManager();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	bool CleanUp();

	void ImportFile(const char* file);
	Resource* CreateNewResource(Resource::Type type);
	Resource* GetResource(uint id);
	Resource::Type CheckFileType(const char* filedir);

	void Save();
	void Load();

private:
	//uint last_uid = PRIMITIVERESERVE;
	uint last_uid = 0;
	std::map<uint, Resource*> resources;
};
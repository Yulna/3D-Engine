#include "ImportScript.h"
#include "Application.h"
#include "ModuleFS.h"
#include "ModuleImporter.h"
#include "ModuleGUI.h"
#include "ResourceScript.h"
#include "ModuleResourceManager.h"
#include "JSONSerialization.h"
#include "CSharpScript.h"
#include "Globals.h"
#include "Timer.h"

#include <direct.h>
#pragma comment(lib, "mono-2.0-sgen.lib")

ImportScript::ImportScript()
{

}


ImportScript::~ImportScript()
{
}

bool ImportScript::InitScriptingSystem()
{
	char my_path[FILENAME_MAX];
	// Fill my_path char array with the path of the .dll
	_getcwd(my_path, FILENAME_MAX);

	// Set where Mono Directory is placed in mono_path
	mono_path = my_path;
	mono_path += "/Mono";

	// Use the standard configuration ----
	mono_config_parse(NULL);

	// Get the correct dirs --------------
	std::string lib = mono_path;
	lib += "/lib";
	std::string etc = mono_path;
	etc += "/etc";


	// Setup the mono directories to start working with
	mono_set_dirs(lib.c_str(), etc.c_str());
	domain = mono_jit_init("CulverinEngine");

	MonoAssembly* culverin_assembly = mono_domain_assembly_open(domain, "./ScriptManager/AssemblyReference/CulverinEditor.dll");
	if (culverin_assembly)
	{
		culverin_mono_image = mono_assembly_get_image(culverin_assembly);

		//Register internal calls to set Scripting functions to work with .cs files
		LinkFunctions();
		return true;
	}

	return false;
}

//This method is called once Mono domain is init to Link C# functions 
//that users will use in their scripts with C++ functions of the application
void ImportScript::LinkFunctions()
{
	//GAMEOBJECT FUNCTIONS ---------------
	/*mono_add_internal_call("CulverinEditor.GameObject::CreateGameObject",(const void*)CreateGameObject);
	mono_add_internal_call("CulverinEditor.GameObject::Destroy",(const void*)DestroyGameObject);
	mono_add_internal_call("CulverinEditor.GameObject::SetActive",(const void*)SetActive);
	mono_add_internal_call("CulverinEditor.GameObject::IsActive",(const void*)IsActive);
	mono_add_internal_call("CulverinEditor.GameObject::SetParent",(const void*)SetParent);
	mono_add_internal_call("CulverinEditor.GameObject::SetName",(const void*)SetName);
	mono_add_internal_call("CulverinEditor.GameObject::GetName",(const void*)GetName);
	mono_add_internal_call("CulverinEditor.GameObject::AddComponent",(const void*)AddComponent);
	mono_add_internal_call("CulverinEditor.GameObject::GetComponent",(const void*)GetComponent);*/

	//CONSOLE FUNCTIONS ------------------
	mono_add_internal_call("CulverinEditor.Debug.Debug::Log",(const void*)ConsoleLog);

	//INPUT FUNCTIONS -------------------
	/*mono_add_internal_call("CulverinEditor.Input::KeyDown",(const void*)KeyDown);
	mono_add_internal_call("CulverinEditor.Input::KeyUp",(const void*)KeyUp);
	mono_add_internal_call("CulverinEditor.Input::KeyRepeat",(const void*)KeyRepeat);
	mono_add_internal_call("CulverinEditor.Input::MouseButtonDown",(const void*)MouseButtonDown);
	mono_add_internal_call("CulverinEditor.Input::MouseButtonUp",(const void*)MouseButtonUp);
	mono_add_internal_call("CulverinEditor.Input::MouseButtonRepeat",(const void*)MouseButtonRepeat);*/
}

//Log messages into Engine Console
void ImportScript::ConsoleLog(MonoString* string)
{
	//Pass from MonoString to const char*
	const char* message = mono_string_to_utf8(string);
	LOG(message);
}

bool ImportScript::Import(const char* file, uint uuid)
{
	uint uuid_script = 0;
	if (uuid == 0) // if direfent create a new resource with the resource deleted
	{
		uuid_script = App->random->Int();
	}
	else
	{
		uuid_script = uuid;
	}
	if (IsNameUnique(App->fs->GetOnlyName(file)) == false)
	{
		LOG("[error] Script: %s, The Name of Script must be unique, there is already a script with that name.", App->fs->GetOnlyName(file).c_str());
		return false;
	}
	else
	{
		// First add name into nameScripts in ImportScript
		nameScripts.push_back(App->fs->GetOnlyName(file));

		// Now create Resource and Import C#
		ResourceScript* res_script = (ResourceScript*)App->resource_manager->CreateNewResource(Resource::Type::SCRIPT, uuid_script);
		if (res_script != nullptr)
		{
			std::string fileassets = App->fs->CopyFileToAssetsS(file);
			// Create TextEditor from the script.
			res_script->SetScriptEditor(App->fs->GetOnlyName(fileassets));

			// First Compile The CSharp
			std::string path_dll;
			if (CompileScript(fileassets.c_str(), path_dll, std::to_string(uuid_script).c_str()) != 0)
			{
				LOG("[error] Script: %s, Not Compiled", App->fs->GetOnlyName(fileassets).c_str());
				res_script->InitInfo(path_dll, fileassets);
				res_script->SetState(Resource::State::FAILED);
				return false;
			}
			else
			{
				LOG("Script: %s, Compiled without errors", App->fs->GetOnlyName(fileassets).c_str());
				res_script->InitInfo(path_dll, fileassets);
				res_script->SetState(Resource::State::LOADED);
				//now 
				CSharpScript* newCSharp = LoadScript_CSharp(path_dll);
				res_script->SetCSharp(newCSharp);
			}


			// Then Create Meta
			std::string Newdirectory = ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory();
			Newdirectory += "\\" + App->fs->FixName_directory(file);
			App->Json_seria->SaveScript(res_script, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory(), Newdirectory.c_str());

		}
	}
	return true;
}

bool ImportScript::LoadResource(const char* file, ResourceScript* resourceScript)
{
	if (resourceScript != nullptr)
	{
		std::string path = file;
		path = "Assets/" + path;
		path = App->fs->GetFullPath(path);
		std::string path_dll;
		// First Compile The CSharp
		if (CompileScript(path.c_str(), path_dll, file) != 0)
		{
			LOG("[error] Script: %s, Not Compiled", App->fs->GetOnlyName(path).c_str());
			resourceScript->InitInfo(path_dll, path);
			resourceScript->SetState(Resource::State::FAILED);
			return false;
		}
		else
		{
			LOG("Script: %s, Compiled without errors", App->fs->GetOnlyName(path).c_str());
			resourceScript->InitInfo(path_dll, path);
			resourceScript->SetState(Resource::State::LOADED);
			//now 
			CSharpScript* newCSharp = LoadScript_CSharp(path_dll);
			resourceScript->SetCSharp(newCSharp);
		}


		// Then Create Meta
		std::string Newdirectory = ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory();
		Newdirectory += "\\" + App->fs->FixName_directory(file);
		App->Json_seria->SaveScript(resourceScript, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory(), Newdirectory.c_str());
	}
	return true;
}

bool ImportScript::ReImportScript(std::string fileAssets, std::string uid_script, ResourceScript* resourceScript)
{
	// First ReCompile The CSharp
	std::string path_dll;
	//if (CompileScript(fileAssets.c_str(), path_dll, uid_script.c_str()) != 0)
	//{
	//	LOG("[error] Script: %s, Not Compiled", App->fs->GetOnlyName(fileAssets).c_str());
	//	return false;
	//}
	//else
	//{
	//	LOG("Script: %s, Compiled without errors", App->fs->GetOnlyName(fileAssets).c_str());
	//	//now 
	//	//CSharpScript* newCSharp = LoadScript_CSharp(path_dll);
	//	//resourceScript->SetCSharp(newCSharp);
	//}


	// Then Create Meta
	//std::string Newdirectory = ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory();
	//Newdirectory += "\\" + App->fs->FixName_directory(file);
	//App->Json_seria->SaveScript(resourceScript, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory(), Newdirectory.c_str());
	return true;
}

bool ImportScript::CreateNewScript(bool& active)
{
	//ImGui::PushStyleVar() // Center
	ImGui::Begin("Create New Script", &active, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders);
	static Timer timeshowResult;
	static int result = 0;
	ImGui::Text("Put Name Name Class: ");
	char namedit[50];
	strcpy_s(namedit, 50, nameNewScript.c_str());
	ImGui::Bullet();
	if (ImGui::InputText("##nameModel", namedit, 50, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (result == 0)
		{
			nameNewScript = App->fs->ConverttoChar(std::string(namedit).c_str());
			if (nameNewScript.compare("") != 0)
			{
				if (IsNameUnique(nameNewScript) == false)
				{
					result = 1;
					timeshowResult.Start();
					LOG("[error] This name is already used! Change name!");
				}
				else
				{
					nameScripts.push_back(nameNewScript);
					result = 2;
					timeshowResult.Start();
					uint uuid_script = App->random->Int();
					ResourceScript* res_script = (ResourceScript*)App->resource_manager->CreateNewResource(Resource::Type::SCRIPT, uuid_script);
					res_script->CreateNewScript(nameNewScript);
					// First Compile The CSharp
					std::string fileassets = nameNewScript;
					fileassets = "Assets/" + fileassets + ".cs";
					std::string path_dll;
					if (CompileScript(App->fs->GetFullPath(fileassets).c_str(), path_dll, std::to_string(uuid_script).c_str()) != 0)
					{
						LOG("[error] Script: %s, Not Compiled", App->fs->GetOnlyName(fileassets).c_str());
						res_script->InitInfo(path_dll, fileassets);
						res_script->SetState(Resource::State::FAILED);
						return false;
					}
					else
					{
						LOG("Script: %s, Compiled without errors", App->fs->GetOnlyName(fileassets).c_str());
						res_script->InitInfo(path_dll, fileassets);
						res_script->SetState(Resource::State::LOADED);
						//now 
						CSharpScript* newCSharp = LoadScript_CSharp(path_dll);
						res_script->SetCSharp(newCSharp);
					}


					// Then Create Meta
					std::string Newdirectory = ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory();
					Newdirectory += "/" + App->fs->GetOnlyName(fileassets) + ".cs";
					App->Json_seria->SaveScript(res_script, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory(), Newdirectory.c_str());
				}
			}
		}
	}
	ImGui::Text("Press 'Enter' to create a new Script");
	if (timeshowResult.ReadSec() < 2)
	{
		if (result == 1)
		{
			ImGui::TextColored(ImVec4(0.933, 0, 0, 1), "[error] This name is already used! Change name!");
		}
		else if (result == 2)
		{
			ImGui::TextColored(ImVec4(0.109, 0.933, 0, 1), "Script: %s, Compiled without errors", nameNewScript.c_str());
		}
	}
	else if (result != 0)
	{
		nameNewScript = "";
		result = 0;
	}

	ImGui::End();
	return true;
}


MonoDomain* ImportScript::GetDomain() const
{
	return domain;
}

MonoImage* ImportScript::GetCulverinImage() const
{
	return culverin_mono_image;
}

std::string ImportScript::GetMonoPath() const
{
	return mono_path;
}

bool ImportScript::IsNameUnique(std::string name) const
{
	if (name != "")
	{
		
		for (std::list<std::string>::const_iterator it = nameScripts.begin(); it != nameScripts.end(); it++)
		{
			if (it._Ptr->_Myval.compare(name) == 0)
			{
				return false;
			}
		}
	}
	return true;
}

int ImportScript::CompileScript(const char* file, std::string& libraryScript, const char* uid)
{
	// Get the path of the project ----
	std::string script_path = file;

	// Get mono directory -------------
	std::string command = GetMonoPath();

	// Save dll to Library Directory --------------------
	libraryScript = App->fs->GetFullPath("Library/Scripts/");
	std::string nameFile = uid;
	nameFile += ".dll";
	libraryScript += nameFile;

	// Compile the script -----------------------------
	command += "/monobin/mcs -debug -target:library -out:" + libraryScript + " ";
	std::string CulverinEditorpath = App->fs->GetFullPath("ScriptManager/AssemblyReference/CulverinEditor.dll");
	command += "-r:" + CulverinEditorpath + " ";
	command += "-lib:" + CulverinEditorpath + " ";
	command += script_path;
	//ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false); -> Hide console (good or bad?)
	return system(command.c_str());
	//system("pause");
}

CSharpScript* ImportScript::LoadScript_CSharp(std::string file)
{
	if (file != "")
	{
		MonoAssembly* assembly = mono_domain_assembly_open(GetDomain(), file.c_str());
		if (assembly)
		{
			// First Get the Image
			MonoImage* image = mono_assembly_get_image(assembly);
			if (image)
			{
				CSharpScript* csharp = CreateCSharp(image);
				if (csharp != nullptr)
				{
					csharp->LoadScript();
					csharp->SetAssembly(assembly);
					return csharp;
				}
				else
				{
					LOG("[error] Can not create MonoClass, file: %s ", file.c_str());
				}
			}
			else
			{
				LOG("[error] Can not Get Image, file: %s ", file.c_str());
			}
		}
		else
		{
			LOG("[error] Can not Open Assembly, file: %s ", file.c_str());
		}
	}
}

CSharpScript* ImportScript::CreateCSharp(MonoImage* image)
{
	if (image)
	{
		std::string classname, name_space;
		MonoClass* entity = GetMonoClassFromImage(image, name_space, classname);
		if (entity)
		{
			CSharpScript* csharp = new CSharpScript();
			csharp->SetDomain(GetDomain());
			csharp->SetImage(image);
			csharp->SetClass(entity);
			csharp->SetClassName(classname);
			csharp->SetNameSpace(name_space);
			return csharp;
		}
	}
	return nullptr;
}


MonoClass* ImportScript::GetMonoClassFromImage(MonoImage* image, std::string& name_space, std::string& classname)
{
	MonoClass* entity = nullptr;
	const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(table_info);
	/* For each row, get some of its values */
	for (int i = 0; i < rows; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
		const char* classnameCS = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		const char* name_spaceCS = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		entity = mono_class_from_name(image, name_spaceCS, classnameCS);
		if (entity)
		{
			name_space = name_spaceCS;
			classname = classnameCS;
		}
	}
	return entity;
}
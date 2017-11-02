#include "Globals.h"
#include "Application.h"
#include "Scene.h"
#include "Primitive.h"
#include "GameObject.h"
#include "Component.h"
#include "CompTransform.h"
#include "CompMesh.h"
#include "CompMaterial.h"
#include "CompCamera.h"
#include "MathGeoLib.h"
#include "Gl3W\include\glew.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_sdl_gl3.h"
#include <direct.h>
#include "WindowInspector.h"

#define SPHERE_DEFINITION 1536

Scene::Scene(bool start_enabled) : Module(start_enabled)
{
	Start_enabled = true;
	preUpdate_enabled = true;
	Update_enabled = true;

	name = "Scene";
	haveConfig = true;
}

Scene::~Scene()
{
}

bool Scene::Start()
{
	perf_timer.Start();

	size_plane = 7;
	CreateMainCamera();

	Start_t = perf_timer.ReadMs();

	return true;
}

update_status Scene::PreUpdate(float dt)
{
	perf_timer.Start();

	frBuff->Bind();

	// PreUpdate GameObjects ------------------------
	for (uint i = 0; i < gameobjects.size(); i++)
	{
		gameobjects[i]->preUpdate();
	}

	preUpdate_t = perf_timer.ReadMs();

	return UPDATE_CONTINUE;
}

update_status Scene::Update(float dt)
{
	perf_timer.Start();

	//Draw Plane
	DrawPlane(size_plane);

	if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
	{
		gameobjects[0]->AddChildGameObject(gameobjects[1]);
		//gameobjects[0]->AddChildGameObject(gameobjects[2]);
		SaveScene();
	}
	if (App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN)
	{
		for (int i = gameobjects.size() - 1; i >= 0; i--)
		{
			delete gameobjects[i];
			gameobjects.pop_back();
		}
		((Inspector*)App->gui->winManager[INSPECTOR])->SetLinkObjectNull();
		LoadScene();
	}



	// Update GameObjects
	for (uint i = 0; i < gameobjects.size(); i++)
	{
		gameobjects[i]->Update();
	}
	
	Update_t = perf_timer.ReadMs();

	return UPDATE_CONTINUE;
}

update_status Scene::UpdateConfig(float dt)
{
	ImGui::PushItemWidth(ImGui::GetWindowWidth() / 4);
	ImGui::SliderInt("Plane Size", &size_plane, 5, 1000);
	ImGui::PopItemWidth();
	return UPDATE_CONTINUE;
}

bool Scene::CleanUp()
{
	return true;
}


void Scene::DrawPlane(int size)
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBegin(GL_LINES);
	for (int i = -size; i <= size; i++)
	{
		glVertex3f(-size, 0, i);
		glVertex3f(size, 0, i);
	}
	for (int i = -size; i <= size; i++)
	{
		glVertex3f(i, 0, size);
		glVertex3f(i, 0, -size);
	}

	glEnd();
}


void Scene::Init_IndexVertex(float3 * vertex_triangulate, uint num_index, CompMesh * mesh)
{
	std::vector<float3> all_index;
	uint size = 0;
	bool temp = false;

	for (int i = 0; i < num_index; i++)
	{
		temp = false;
		size = all_index.size();

		for (int j = 0; j < size; j++)
		{
			if (all_index[j] == vertex_triangulate[i])
			{
				mesh->indices.push_back(j);
				temp = true;
			}
		}

		if (temp == false)
		{
			_Vertex vertex;
			all_index.push_back(vertex_triangulate[i]);
			mesh->indices.push_back(all_index.size() - 1);
			vertex.pos = vertex_triangulate[i];
			mesh->vertices.push_back(vertex);
		}
	}
}

GameObject* Scene::CreateGameObject()
{
	GameObject* obj = new GameObject();
	gameobjects.push_back(obj);
	return obj;
}

GameObject* Scene::CreateCube()
{
	GameObject* obj = new GameObject();

	// SET NAME -----------------------------------
	static uint cube_count = 0;
	std::string name = "Cube ";
	name += std::to_string(cube_count++);
	char* name_str = new char[name.size()+1];
	strcpy(name_str, name.c_str());
	obj->SetName(name_str);

	/* Predefined Cube has 3 Base components: Transform, Mesh & Material */

	//TRANSFORM COMPONENT --------------
	CompTransform* transform = (CompTransform*)obj->AddComponent(C_TRANSFORM);
	transform->Init(float3(0, 0, 0), float3(0, 0, 0), float3(1, 1, 1));
	transform->Enable();

	//MESH COMPONENT -------------------
	CompMesh* mesh = (CompMesh*)obj->AddComponent(C_MESH);
	mesh->isPrimitive = true;
	mesh->TypePrimitive = 2;
	mesh->InitRanges(8, 36, 0); // 0 normals by now.

	OBB* box = new OBB();
	box->pos = transform->GetPos();
	box->r = transform->GetScale();
	box->axis[0] = float3(1, 0, 0);
	box->axis[1] = float3(0, 1, 0);
	box->axis[2] = float3(0, 0, 1);

	obj->bounding_box = new AABB(*box);
	float3* vertices_array = new float3[36];

	obj->bounding_box->Triangulate(1, 1, 1, vertices_array, NULL, NULL, false);

	Init_IndexVertex(vertices_array, mesh->num_indices, mesh);
	mesh->SetupMesh();
	mesh->Enable();

	//MATERIAL COMPONENT -------------------
	CompMaterial* mat = (CompMaterial*)obj->AddComponent(C_MATERIAL);
	mat->Enable();

	App->scene->gameobjects.push_back(obj);

	LOG("CUBE Created.");

	return obj;
}

GameObject* Scene::CreateSphere()
{
	GameObject* obj = new GameObject();

	// SET NAME ----------------------------------
	static uint sphere_count = 0;
	std::string name = "Sphere ";
	name += std::to_string(sphere_count++);
	char* name_str = new char[name.size() + 1];
	strcpy(name_str, name.c_str());
	obj->SetName(name_str);

	/*Predefined sPHERE has 2 Base components: Transform, Mesh & Material */

	// TRANSFORM COMPONENT --------------
	CompTransform* transform = (CompTransform*)obj->AddComponent(C_TRANSFORM);
	transform->Init(float3(0, 0, 0), float3(0, 0, 0), float3(1, 1, 1));
	transform->Enable();

	// MESH COMPONENT -------------------
	CompMesh* mesh = (CompMesh*)obj->AddComponent(C_MESH);
	mesh->isPrimitive = true;
	mesh->TypePrimitive = 1;

	float3* vertices_array = new float3[SPHERE_DEFINITION];

	Sphere* sphere = new Sphere(transform->GetPos(), 1);
	sphere->Triangulate(vertices_array, NULL, NULL, SPHERE_DEFINITION, false);
	
	Init_IndexVertex(vertices_array, SPHERE_DEFINITION, mesh);
	mesh->InitRanges(mesh->vertices.size(), mesh->indices.size(), 0);
	mesh->SetupMesh();
	mesh->Enable();

	/* Set Bounding Box */
	obj->bounding_box = new AABB();
	obj->bounding_box->SetNegativeInfinity();
	obj->bounding_box->Enclose(mesh->vertices, mesh->num_vertices);

	// MATERIAL COMPONENT -------------------
	CompMaterial* mat = (CompMaterial*)obj->AddComponent(C_MATERIAL);
	mat->Enable();

	App->scene->gameobjects.push_back(obj);

	LOG("SPHERE Created.");

	return obj;
}

GameObject* Scene::CreateMainCamera()
{
	GameObject* obj = new GameObject();

	// SET NAME -----------------------------------
	std::string name = "MainCamera";
	char* name_str = new char[name.size() + 1];
	strcpy(name_str, name.c_str());
	obj->SetName(name_str);

	/* Predefined Main Camera has 2 Base components: Transform & Camera */

	// TRANSFORM COMPONENT --------------
	CompTransform* transform = (CompTransform*)obj->AddComponent(C_TRANSFORM);
	transform->Init(float3(0, 0, 0), float3(0, 0, 0), float3(1, 1, 1));
	transform->Enable();

	// CAMERA COMPONENT -----------------
	CompCamera* camera = (CompCamera*)obj->AddComponent(C_CAMERA);
	camera->Init(transform->GetPos());
	camera->Enable();

	App->scene->gameobjects.push_back(obj);

	LOG("MAIN CAMERA Created.");

	return obj;
}

void Scene::SaveScene()
{
	JSON_Value* config_file;
	JSON_Object* config;
	JSON_Object* config_node;

	config_file = json_parse_file("Scene_1.json");

	if (config_file != nullptr)
	{
		config = json_value_get_object(config_file);
		config_node = json_object_get_object(config, "Scene");
		json_object_clear(config_node);
		json_object_dotset_number_with_std(config_node, "Info.Number of GameObjects", gameobjects.size());

		// Update GameObjects
		for (uint i = 0; i < gameobjects.size(); i++)
		{
			std::string name = "GameObject" + std::to_string(i);
			name += ".";
			// UUID--------
			json_object_dotset_number_with_std(config_node, name + "UUID", gameobjects[i]->GetUUID());
			// Parent UUID------------
			json_object_dotset_number_with_std(config_node, name + "Parent", -1);
			// Name- --------
			json_object_dotset_string_with_std(config_node, name + "Name", gameobjects[i]->GetName());

			// Components  ------------
			std::string components = name;
			json_object_dotset_number_with_std(config_node, components + "Number of Components", gameobjects[i]->GetNumComponents());
			if (gameobjects[i]->GetNumComponents() > 0)
			{
				components += "Components.";
				gameobjects[i]->SaveComponents(config_node, components);
			}

			//Save Childs
			json_object_dotset_number_with_std(config_node, name + "Number of Childs", gameobjects[i]->GetNumChilds());
			if (gameobjects[i]->GetNumChilds() > 0)
			{
				std::string name_Parent = name;
				name_Parent += "Childs.";
				for (int j = 0; j < gameobjects[i]->GetNumChilds(); j++)
				{
					std::string child_name = name_Parent;
					child_name += "Child_num " + std::to_string(j);
					child_name += ".";
					SaveChildGameObject(config_node, *gameobjects[i]->GetChildbyIndex(j), gameobjects[i]->GetUUID(), child_name);
				}
			}
		}
	}
	json_serialize_to_file(config_file, "Scene_1.json");
}

void Scene::SaveChildGameObject(JSON_Object* config_node, const GameObject& child, uint uid, std::string name)
{
	// UUID--------
	json_object_dotset_number_with_std(config_node, name + "UUID", child.GetUUID());
	// Parent UUID------------
	json_object_dotset_number_with_std(config_node, name + "Parent", uid);
	// Name- --------
	json_object_dotset_string_with_std(config_node, name + "Name", child.GetName());

	// Components  ------------
	std::string components = name;
	json_object_dotset_number_with_std(config_node, components + "Number of Components", child.GetNumComponents());
	if (child.GetNumComponents() > 0)
	{
		components += "Components.";
		child.SaveComponents(config_node, components);
	}

	//Save Childs
	json_object_dotset_number_with_std(config_node, name + "Number of Childs", child.GetNumChilds());
	if (child.GetNumChilds() > 0)
	{
		name += "Childs.";
		for (int i = 0; i < child.GetNumChilds(); i++)
		{
			std::string name_Child = name;
			name_Child += "Child_num " + std::to_string(i);
			name_Child += ".";
			SaveChildGameObject(config_node, *child.GetChildbyIndex(i), child.GetUUID(), name_Child);
		}
	}
}

void Scene::LoadScene()
{
	JSON_Value* config_file;
	JSON_Object* config;
	JSON_Object* config_node;

	config_file = json_parse_file("Scene_1.json");
	if (config_file != nullptr)
	{
		config = json_value_get_object(config_file);
		config_node = json_object_get_object(config, "Scene");
		int NUmberGameObjects = json_object_dotget_number(config_node, "Info.Number of GameObjects");
		if (NUmberGameObjects > 0)
		{
			for (int i = 0; i < NUmberGameObjects; i++)
			{
				std::string name = "GameObject" + std::to_string(i);
				name += ".";
				//dock.label = _strdup(json_object_dotget_string_with_std(config_node, name + "Label"));
				//dock.pos.x = json_object_dotget_number_with_std(config_node, name + "Position x");
				char* nameGameObject = App->GetCharfromConstChar(json_object_dotget_string_with_std(config_node, name + "Name"));
				uint uid = json_object_dotget_number_with_std(config_node, name + "UUID");
				GameObject* obj = new GameObject(nameGameObject, uid);

				//Load Components
				int NumberofComponents = json_object_dotget_number_with_std(config_node, name + "Number of Components");
				if (NumberofComponents > 0)
				{
					obj->LoadComponents(config_node, name + "Components.", NumberofComponents);
				}

				//Load Childs
				int NumberofChilds = json_object_dotget_number_with_std(config_node, name + "Number of Childs");
				if (NumberofChilds > 0)
				{
					std::string childs = name + "Childs.";
					LoadChildGameObject(config_node, *obj, obj->GetUUID(), NumberofChilds, childs);
				}

				//Add GameObject
				gameobjects.push_back(obj);
			}
		}
	}
}

void Scene::LoadChildGameObject(const JSON_Object* config_node, GameObject& parent, uint uid, uint numChild, std::string name)
{
	uint numCErrors = 0;	
	uint numCloaded = 0;
	for (int i = 0; i < numChild; i++)
	{
		std::string child_n = name + "Child_num " + std::to_string(i) + ".";
		char* nameGameObject = App->GetCharfromConstChar(json_object_dotget_string_with_std(config_node, child_n + "Name"));
		uint uuid = json_object_dotget_number_with_std(config_node, child_n + "UUID");
		uint uuid_parent = json_object_dotget_number_with_std(config_node, child_n + "Parent");
		if (uuid_parent == uid) //Check same parent
		{
			GameObject* obj = new GameObject(nameGameObject, uuid);
			//Load Components
			int NumberofComponents = json_object_dotget_number_with_std(config_node, child_n + "Number of Components");
			if (NumberofComponents > 0)
			{
				obj->LoadComponents(config_node, child_n + "Components.", NumberofComponents);
			}
			//Load Childs
			int NumberofChilds = json_object_dotget_number_with_std(config_node, name + "Number of Childs");
			if (NumberofChilds > 0)
			{
				std::string childs = child_n + "Childs.";
				LoadChildGameObject(config_node, *obj, obj->GetUUID(), NumberofChilds, childs);
			}
			parent.AddChildGameObject(obj);
			numCloaded++;
		}
		else
		{
			numCErrors++;
		}
	}
	LOG("Childs Loaded -----------  Info: num Childs Loaded = %i, num Errors = %i", numCloaded, numCErrors);
}




/*for (int i = 0; i < CHECKERS_HEIGHT; i++)
{
for (int j = 0; j < CHECKERS_WIDTH; j++)
{
int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
checkImage[i][j][0] = (GLubyte)c;
checkImage[i][j][1] = (GLubyte)c;
checkImage[i][j][2] = (GLubyte)c;
checkImage[i][j][3] = (GLubyte)255;
}
}*/
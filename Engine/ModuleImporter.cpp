#include "ModuleImporter.h"
#include "Application.h"
#include "ImportMesh.h"
#include "ImportMaterial.h"
#include "CompMaterial.h"
#include "CompTransform.h"
#include "JSONSerialization.h"

ModuleImporter::ModuleImporter(bool start_enabled) : Module(start_enabled)
{
	//char ownPth[MAX_PATH];

	//// Will contain exe path
	//HMODULE hModule = GetModuleHandle(NULL);
	//if (hModule != NULL)
	//{
	//	// When passing NULL to GetModuleHandle, it returns handle of exe itself
	//	GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
	//}
	name = "Importer";
}


ModuleImporter::~ModuleImporter()
{
}

bool ModuleImporter::Init(JSON_Object* node)
{
	// Will contain exe path
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// When passing NULL to GetModuleHandle, it returns handle of exe itself
		GetModuleFileName(hModule, directoryExe, (sizeof(directoryExe)));
	}
	iMesh = new ImportMesh();
	iMaterial = new ImportMaterial();

	return true;
}

bool ModuleImporter::Start()
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return true;
}

update_status ModuleImporter::PreUpdate(float dt)
{
	perf_timer.Start();

	if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
	{
		//ImportMesh* imp = new ImportMesh();
		//imp->Load("Baker_house.rin");
	}

	//if (App->input->dropped)
	//{
	//	dropped_File_type = CheckFileType(App->input->dropped_filedir);


	//	App->fs->CopyFileToAssets(App->input->dropped_filedir, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory());
	//	((Project*)App->gui->winManager[WindowName::PROJECT])->UpdateNow();

	//	switch (dropped_File_type)
	//	{
	//	case F_MODEL_i:
	//	{
	//		LOG("IMPORTING MODEL, File Path: %s", App->input->dropped_filedir);
	//		std::string file;

	//		//Clear vector of textures, but dont import same textures!
	//		iMesh->PrepareToImport();

	//		const aiScene* scene = aiImportFile(App->input->dropped_filedir, aiProcessPreset_TargetRealtime_MaxQuality);
	//		GameObject* obj = ProcessNode(scene->mRootNode, scene, nullptr);
	//		
	//		aiReleaseImport(scene);

	//		App->scene->gameobjects.push_back(obj);
	//		
	//		//Now Save Serialitzate OBJ -> Prefab
	//		App->Json_seria->SavePrefab(*obj, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory());
	//		break;
	//	}
	//	case F_TEXTURE_i:
	//	{
	//		LOG("IMPORTING TEXTURE, File Path: %s", App->input->dropped_filedir);
	//		//iMaterial->Import(App->input->dropped_filedir);
	//	
	//		break;
	//	}
	//	case F_UNKNOWN_i:
	//	{
	//		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "UNKNOWN file type dropped on window",
	//			App->input->dropped_filedir, App->window->window);
	//		LOG("UNKNOWN FILE TYPE, File Path: %s", App->input->dropped_filedir);

	//		break;
	//	}

	//	default:
	//		break;

	//	}

	//	App->input->dropped = false;
	//}

	Update_t = perf_timer.ReadMs();


	return UPDATE_CONTINUE;
}

GameObject* ModuleImporter::ProcessNode(aiNode* node, const aiScene* scene, GameObject* obj)
{	
	static int count = 0;
	GameObject* objChild = new GameObject(obj);
	objChild->SetName(App->GetCharfromConstChar(node->mName.C_Str()));

	CompTransform* trans = (CompTransform*)objChild->AddComponent(C_TRANSFORM);
	ProcessTransform(node, trans);

	// Process all the Node's MESHES
	for (uint i = 0; i < node->mNumMeshes; i++)
	{
		GameObject* newObj = nullptr;

		if (node->mNumMeshes > 1)
		{
			newObj = new GameObject(obj);
			std::string newName = "Submesh" + std::to_string(i);
			newObj->SetName(App->GetCharfromConstChar(newName.c_str()));
			CompTransform* newTrans = (CompTransform*)newObj->AddComponent(C_TRANSFORM);
			ProcessTransform(node, newTrans);
		}

		else
		{
			newObj = objChild;
		}

		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		iMesh->Import(scene, mesh, newObj, node->mName.C_Str());
	}

	// Process children
	for (uint i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, objChild);
	}

	return objChild;
}

void ModuleImporter::ProcessTransform(aiNode* node, CompTransform* trans)
{
	aiVector3D aiPos;
	aiQuaternion aiRot;
	aiVector3D aiScale;
	aiMatrix4x4 aiMatrix;
	float4x4 matrix;

	aiMatrix = node->mTransformation;

	node->mTransformation.Decompose(aiScale, aiRot, aiPos);

	trans->SetPos(float3(aiPos.x, aiPos.y, aiPos.z));
	trans->SetRot(Quat(aiRot.x, aiRot.y, aiRot.z, aiRot.w));
	trans->SetScale(float3(aiScale.x, aiScale.y, aiScale.z));

	trans->Enable();
}

void ModuleImporter::ProcessTransform(CompTransform* trans)
{
	//Set all variables to zero/identity
	trans->ResetMatrix();
	trans->Enable();
}

update_status ModuleImporter::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleImporter::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleImporter::UpdateConfig(float dt)
{
	return UPDATE_CONTINUE;
}

bool ModuleImporter::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

bool ModuleImporter::Import(const char* file, Resource::Type type)
{
		switch (type)
		{
		case Resource::Type::MESH:
		{
			LOG("IMPORTING MODEL, File Path: %s", file);
			//Clear vector of textures, but dont import same textures!
			iMesh->PrepareToImport();

			const aiScene* scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);
			GameObject* obj = ProcessNode(scene->mRootNode, scene, nullptr);
			obj->SetName(App->GetCharfromConstChar(App->fs->FixName_directory(App->input->dropped_filedir).c_str()));
			
			aiReleaseImport(scene);

			//App->scene->gameobjects.push_back(obj);
			
			//Now Save Serialitzate OBJ -> Prefab
			App->Json_seria->SavePrefab(*obj, ((Project*)App->gui->winManager[WindowName::PROJECT])->GetDirectory());
			break;
		}
		case Resource::Type::MATERIAL:
		{
			LOG("IMPORTING TEXTURE, File Path: %s", file);
			//iMaterial->Import(App->input->dropped_filedir);
		
			break;
		}
		case Resource::Type::UNKNOWN:
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "UNKNOWN file type dropped on window",
				file, App->window->window);
			LOG("UNKNOWN FILE TYPE, File Path: %s", file);

			break;
		}

		default:
			break;
		}

	return true;
}
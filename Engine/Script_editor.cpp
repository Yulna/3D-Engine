#include "Script_editor.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleFS.h"

#include <filesystem>
#include <iostream>
#include <experimental/filesystem>
#include <fstream>


Script_editor::Script_editor()
{
}


Script_editor::~Script_editor()
{
}

void Script_editor::Start(std::string nameScript, bool isnew)
{
	name = nameScript;
	if (isnew)
	{
		SetInitInfo(nameScript);
		//directory = "C:\\Users\\Administrador\\Documents\\GitHub\\3D-Engine\\Engine\\Game";
	}
	else
	{
		//need function
		nameScript = "Assets\\" + nameScript + ".cpp";
		std::ifstream t(nameScript.c_str());
		std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		editor.SetText(str);
	}

	editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
	//directory = "C:\\Users\\Administrador\\Documents\\GitHub\\3D-Engine\\Engine\\zTestScript.cpp";
	//std::ifstream t(directory.c_str());
	//std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	//editor.SetText(str);
}

void Script_editor::SetInitInfo(std::string nameScript)
{
	editor.InsertText("// ------- New Script Culverin Engine 3D ------- \n\n");
	editor.InsertText("using System.Collections;\n");
	editor.InsertText("using System.Collections.Generic;\n");
	editor.InsertText("using UnityEngine;\n\n");

	editor.InsertText("public class ");
	editor.InsertText(nameScript);
	editor.InsertText(" : MonoBehaviour \n");
	editor.InsertText("{\n\n");

	editor.InsertText("	// Use this for initialization\n");
	editor.InsertText("	void Start() {\n\n");

	editor.InsertText("	}\n\n");

	editor.InsertText("	// Update is called once per frame\n");
	editor.InsertText("	void Update() {\n\n");

	editor.InsertText("	}\n");
	editor.InsertText("}\n");
}

void Script_editor::Show(bool& active)
{
	//ImGui::PushStyleVar() // Center
	ImGui::Begin(name.c_str(), &active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", NULL, false, false))
			{
			}
			if (ImGui::MenuItem("Save"))
			{
				SaveScript();
			}
			if (ImGui::MenuItem("Close", NULL, &active))
			{
				active = false;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl + Z"))
			{
				SetAction(ActionEditor::UNDO);
			}
			if (ImGui::MenuItem("ReUndo", "Ctrl + Shift + Z"))
			{
				SetAction(ActionEditor::REUNDO);
			}
			bool activecopy = false;
			if (editor.GetSelectedText() != "")
			{
				activecopy = true;
			}
			if (ImGui::MenuItem("Copy", "Ctrl + C", false, activecopy))
			{
				SetAction(ActionEditor::COPY);
			}
			bool activepaste = false;
			if (textSelected != "")
			{
				activepaste = true;
			}
			if (ImGui::MenuItem("Paste", "Ctrl + V", false, activepaste))
			{
				SetAction(ActionEditor::PASTE);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_STATE::KEY_DOWN ||
		App->input->GetKey(SDL_SCANCODE_RETURN2) == KEY_STATE::KEY_DOWN)
	{
		SetAction(ActionEditor::RETURN);
	}

	// Copy ----------------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_STATE::KEY_REPEAT &&
		App->input->GetKey(SDL_SCANCODE_C) == KEY_STATE::KEY_DOWN)
	{
		SetAction(ActionEditor::COPY);
	}


	// Paste ----------------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_STATE::KEY_REPEAT &&
		App->input->GetKey(SDL_SCANCODE_V) == KEY_STATE::KEY_DOWN)
	{
		SetAction(ActionEditor::PASTE);
	}

	// Undo ----------------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_STATE::KEY_REPEAT &&
		App->input->GetKey(SDL_SCANCODE_Z) == KEY_STATE::KEY_DOWN)
	{
		SetAction(ActionEditor::UNDO);
	}

	// ReUndo ----------------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_STATE::KEY_REPEAT &&
		App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_STATE::KEY_REPEAT &&
		App->input->GetKey(SDL_SCANCODE_Z) == KEY_STATE::KEY_DOWN)
	{
		SetAction(ActionEditor::REUNDO);
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushFont(io.Fonts->Fonts[1]);
	editor.Render("Render...");
	ImGui::PopFont();

	ImGui::End();
}

void Script_editor::SetAction(ActionEditor action)
{
	switch (action)
	{
	case ActionEditor::COPY:
	{
		textSelected = editor.GetSelectedText();
		break;
	}
	case ActionEditor::PASTE:
	{
		editor.InsertText(textSelected);
		break;
	}
	case ActionEditor::UNDO:
	{
		editor.Undo();
		break;
	}
	case ActionEditor::REUNDO:
	{
		editor.Redo();
		break;
	}
	case ActionEditor::RETURN:
	{
		editor.InsertText("\n");
		break;
	}
	}
}

void Script_editor::SaveScript()
{
	App->fs->SaveScript(name, editor, DIRECTORY_IMPORT::IMPORT_DIRECTORY_ASSETS);
}

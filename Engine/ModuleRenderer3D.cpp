#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "SDL/include/SDL_opengl.h"
#include "GL3W\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "parson.h"


#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer";
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init(JSON_Object* node)
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);

	if(context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if(ret == true)
	{
		//Use Vsync
		if(App->GetVSYNC() && SDL_GL_SetSwapInterval(1) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();


		glViewport(0, 0, 800, 600);

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);


		//Load render config info -------
		depth_test = json_object_get_boolean(node, "Depth Test");
		cull_face = json_object_get_boolean(node, "Cull Face");
		lightning = json_object_get_boolean(node, "Lightning");
		color_material = json_object_get_boolean(node, "Color Material");
		texture_2d = json_object_get_boolean(node, "Texture 2D");
		wireframe = json_object_get_boolean(node, "Wireframe");
		axis = json_object_get_boolean(node, "Axis");
		smooth = json_object_get_boolean(node, "Smooth");
		
		node = json_object_get_object(node, "Fog");
		fog_active = json_object_get_boolean(node, "Active");
		fog_density = json_object_get_number(node, "Density");


		(depth_test) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		(cull_face) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		lights[0].Active(true);
		(lightning) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		(color_material) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
		(texture_2d) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
		(smooth) ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);

		if (fog_active)
		{
			glEnable(GL_FOG);
			glFogfv(GL_FOG_DENSITY, &fog_density);
		}

	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	SDL_GL_SwapWindow(App->window->window);
	if (App->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		lights[0].Active(false);
	}
	if (App->input->GetKey(SDL_SCANCODE_H) == KEY_UP)
	{
		lights[0].Active(true);
	}

	return UPDATE_CONTINUE;
}

update_status ModuleRenderer3D::UpdateConfig(float dt)
{
	if (ImGui::CollapsingHeader("Render"))
	{
		if (ImGui::Checkbox("Depth Test", &depth_test))
		{
			(depth_test) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}
		if (ImGui::Checkbox("Cull Face", &cull_face))
		{
			(cull_face) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		}
		if (ImGui::Checkbox("Lightning", &lightning))
		{
			(lightning) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		}
		if (ImGui::Checkbox("Color Material", &color_material))
		{
			(color_material) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
		}
		if (ImGui::Checkbox("Texture 2D", &texture_2d))
		{
			(texture_2d) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
		}
		if (ImGui::Checkbox("Wireframe", &wireframe))
		{
			App->objects->ShowWire(wireframe);
		}
		if (ImGui::Checkbox("Objects Axis", &axis))
		{
			App->objects->ShowAxis(axis);
		}
		if (ImGui::Checkbox("Smooth", &smooth))
		{
			(smooth) ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
		}
		if (ImGui::Checkbox("Fog", &fog_active))
		{
			if (fog_active)
			{
				glEnable(GL_FOG);
				glFogfv(GL_FOG_DENSITY, &fog_density);
			}
			else
			{
				glDisable(GL_FOG);
			}
		}
	}
	return UPDATE_CONTINUE;
}




// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

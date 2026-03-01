#include "Window.h"
#include "RHI.h"
#include "Camera.h"
#include "Renderer.h"

#include "ThreadPool.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Physic.h"
#include "BoxCollider.h"
#include "Rigidbody.h"

#include <filesystem>

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Resources;
using namespace Apex::Physic;
using namespace Apex::Core;

int main(int argc, char* argv[])
{
	std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());

	// Initialize window and RHI
	std::unique_ptr<IWindow> window(CreateWindow("Engine", 1280, 720));

	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());

	Camera camera({ 0.f, 0.f, 3.f });

	// Capture mouse for FPS look
	window->SetCursorCaptured(true);

	ThreadPool tp;
	ResourceManager rm(tp);
	PhysicSystem physics;

	rhi->SetupVertexLayout();

	// Input callbacks
	window->SetKeyCallback([&](Key key, bool pressed)
		{
			if (key == Key::Escape && pressed)
				window->Close();
		});

	window->SetMouseMoveCallback([&](double dx, double dy)
		{
			camera.ProcessMouse(dx, dy);
		});

	window->SetScrollCallback([&](double, double yoff)
		{
			camera.ProcessScroll(yoff);
		});

	// dragon (slow)
	//ResourceHandle mesh = rm.CreateAsync<Mesh>("Assets/Meshes/M_B_44_Qishilong_skin_Skeleton.FBX");
	// faster
	ResourceHandle<Mesh> mesh = rm.CreateAsync<Mesh>("Assets/Meshes/sphere.obj");
	ResourceHandle<Texture> texture = rm.CreateAsync<Texture>("Assets/Textures/container.jpg");
	ResourceHandle<Shader> shader = rm.CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");

	ResourceHandle<Mesh> cube = rm.CreateAsync<Mesh>("Assets/Meshes/cube.obj");

	//LibMath::Matrix4 trans = LibMath::Matrix4::createScale(LibMath::Vector3(0.0005f));
	//trans = trans * LibMath::Matrix4::createRotationX(LibMath::Degree(-90));
	LibMath::Matrix4 trans = LibMath::Matrix4::createScale(LibMath::Vector3(1));
	LibMath::Matrix4 trans2 = LibMath::Matrix4::createScale(LibMath::Vector3(2));

	BoxCollider collider = BoxCollider({ 1.f, 1.f, 1.f });
	RigidBodyComponent body = RigidBodyComponent(1, BodyType::DYNAMIC);


	BoxCollider collider2 = BoxCollider({ 10.f, .5f, 10.f });
	RigidBodyComponent body2 = RigidBodyComponent(0, BodyType::STATIC);

	body.SetPosition({ 0, 10, 0 });
	body2.SetPosition({ 0, 0, 0 });

	physics.CreateActor(collider, body);
	physics.CreateActor(collider2, body2);
	//physics.AddGround();

	// Scene objects
	Renderer cubeRenderer(rhi.get());
	cubeRenderer.LoadTexture("Assets/Textures/container.jpg");

	float totalTime = 0.f;
	while (!window->ShouldClose())
	{
		window->PollEvents();
		float deltaTime = window->GetDeltaTime();
		totalTime += deltaTime;

		// Camera keyboard movement
		camera.ProcessKeyboard(
			window->IsKeyPressed(Key::W),
			window->IsKeyPressed(Key::S),
			window->IsKeyPressed(Key::A),
			window->IsKeyPressed(Key::D),
			window->IsKeyPressed(Key::E),
			window->IsKeyPressed(Key::Q),
			deltaTime
		);

		// Clear
		rhi->Clear(0.12f, 0.12f, 0.15f);

		// Matrices from camera
		LibMath::Matrix4 view = camera.GetViewMatrix();
		LibMath::Matrix4 proj = camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov);

		if (mesh.IsReady() && texture.IsReady() && shader.IsReady() && cube.IsReady())
		{
			printf("here\r");

			physics.Update(deltaTime);
			physics.SyncFromPhysx();

			shader->Use();
			texture->Bind();
			GLuint prog = shader->GetProgram();
			
			rhi->SetUniformMat4(prog, "uModel", trans * LibMath::Matrix4::createTranslation(body.GetPosition()));
			rhi->SetUniformMat4(prog, "uView", view);
			rhi->SetUniformMat4(prog, "uProjection", proj);
			rhi->SetUniformVec3(prog, "uCameraPos", camera.GetPosition());
								
			rhi->SetUniformVec3(prog, "uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
			rhi->SetUniformVec3(prog, "uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
			rhi->SetUniformVec3(prog, "uViewPos", camera.GetPosition());

			rhi->SetUniformInt(prog, "uTexture", 0);
			rhi->SetUniformInt(prog, "uUseTexture", 1);

			rhi->BindVAO(cube->GetVAO());
			rhi->DrawIndexed(static_cast<GLsizei>(cube->m_indices.size()));


			rhi->SetUniformMat4(prog, "uModel", trans2 * LibMath::Matrix4::createTranslation(body2.GetPosition()));
			rhi->BindVAO(cube->GetVAO());
			rhi->DrawIndexed(static_cast<GLsizei>(cube->m_indices.size()));
		}

		// Draw cube
		cubeRenderer.Render(view, proj, camera.GetPosition(), totalTime);

		rm.ProcessGpuUploads();

		window->SwapBuffers();
	}

	return 0;
}
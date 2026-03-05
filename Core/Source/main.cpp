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
#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "Rigidbody.h"

#include "DebugRenderer.h"

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

	Camera camera({ 0.f, 0.f, 10.f });

	// Capture mouse for FPS look
	window->SetCursorCaptured(true);

	ThreadPool tp;
	ResourceManager rm(tp);
	PhysicSystem physics;
	DebugRenderer::Initialize(rm.CreateAsync<Shader>("Assets/Shaders/LineVert.glsl|Assets/Shaders/LineFrag.glsl"));

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
	ResourceHandle mesh2 = rm.CreateAsync<Mesh>("Assets/Meshes/M_B_44_Qishilong_skin_Skeleton.FBX");
	// faster
	ResourceHandle<Mesh> mesh = rm.CreateAsync<Mesh>("Assets/Meshes/sphere.obj");
	ResourceHandle<Texture> texture = rm.CreateAsync<Texture>("Assets/Textures/container.jpg");
	ResourceHandle<Texture> texture2 = rm.CreateAsync<Texture>("Assets/Textures/T_M_B_44_Qishilong_body01_B.png");
	ResourceHandle<Shader> shader = rm.CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");

	ResourceHandle<Mesh> cube = rm.CreateAsync<Mesh>("Assets/Meshes/cube.obj");

	while (!mesh.IsReady() || 
		!mesh2.IsReady() || 
		!cube.IsReady() || 
		!texture.IsReady() || 
		!shader.IsReady())
		rm.ProcessGpuUploads();

	BoxCollider collider = BoxCollider({ 1.f, 1.f, 1.f });
	RigidBodyComponent body = RigidBodyComponent(1, BodyType::DYNAMIC);


	BoxCollider collider2 = BoxCollider({ 10.f, .5f, 10.f });
	RigidBodyComponent body2 = RigidBodyComponent(0, BodyType::STATIC);

	MeshCollider meshCollider = MeshCollider(mesh, MeshColliderType::Convex);
	MeshCollider meshCollider2 = MeshCollider(mesh2, MeshColliderType::Triangle);

	CapsuleCollider capsule(2, 2);

	body.SetPosition({ 0, 10, 0 });
	body2.SetPosition({ 0, 0, 0 });
	body2.SetScale(LibMath::Vector3(0.001f));
	body2.SetRotation(LibMath::Quaternion(LibMath::Degree(-90), LibMath::Radian(0), LibMath::Radian(0)));


	physics.CreateActor(meshCollider, body);
	physics.CreateActor(meshCollider2, body2);


	float accumulator = 0.f;
	while (!window->ShouldClose())
	{
		window->PollEvents();
		float deltaTime = window->GetDeltaTime();
		if (deltaTime > 0.1f) deltaTime = 1.f / 60.f;

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


		accumulator += deltaTime;
		while (accumulator >= 1.f / 60.f)
		{
			physics.Update(1.f / 60.f);
			accumulator -= 1.f / 60.f;
		}
		physics.SyncFromPhysx();

		shader->Use();
		texture->Bind();
		GLuint prog = shader->GetProgram();

		rhi->SetUniformMat4(prog, "uModel", static_cast<LibMath::Matrix4>(body.GetTransform()));
		rhi->SetUniformMat4(prog, "uView", view);
		rhi->SetUniformMat4(prog, "uProjection", proj);
		rhi->SetUniformVec3(prog, "uCameraPos", camera.GetPosition());
							
		rhi->SetUniformVec3(prog, "uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
		rhi->SetUniformVec3(prog, "uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
		rhi->SetUniformVec3(prog, "uViewPos", camera.GetPosition());

		rhi->SetUniformInt(prog, "uTexture", 0);
		rhi->SetUniformInt(prog, "uUseTexture", 1);

		rhi->BindVAO(mesh->GetVAO());
		rhi->DrawIndexed(static_cast<GLsizei>(mesh->m_indices.size()));

		texture2->Bind();
		rhi->SetUniformMat4(prog, "uModel", static_cast<LibMath::Matrix4>(body2.GetTransform()));
		rhi->BindVAO(mesh2->GetVAO());
		rhi->DrawIndexed(static_cast<GLsizei>(mesh2->m_indices.size()));


		physics.DrawColliders();
		DebugRenderer::Get().DrawLines(rhi.get(), proj * view);

		rm.ProcessGpuUploads();

		window->SwapBuffers();
	}

	return 0;
}
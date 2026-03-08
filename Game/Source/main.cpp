// ── Engine abstractions ───────────────────────────────────────
#include "Window.h"
#include "Audio.h"
#include "RHI.h"
#include "Camera.h"

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

#include "Object.h"

#include <filesystem>

#include "UI.h"
#include "Scene.h"
#include "Hud.h"

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::UserInterface;
using namespace Apex::Core;
using namespace Apex::Resources;
using namespace Apex::Audio;
using namespace Apex::Physic;

int main(int argc, char* argv[])
{
	std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());
	// Initialize
	std::unique_ptr<IWindow> window(CreateWindow("Apex Renderer", 1280, 720));
	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());
	std::unique_ptr<IAudioEngine> audio(CreateAudioEngine());

	// Initialize the scene
	ThreadPool threadPool;
	Scene scene(rhi.get(), threadPool);

	// Audio setup 
	if (audio)
	{
		audio->Preload("fragments of hope.ogg");
	}

	//SoundHandle backgroundMusic = audio ? audio->Play2D("fragments of hope.ogg", { .m_loop = true, .m_volume = 1.f }) : INVALID_SOUND;

	// Initialize Camera
	Camera camera;
	window->SetCursorCaptured(true);

	ResourceManager rm(threadPool);
	rm.SetRHI(rhi.get());
	PhysicSystem physics;
	DebugRenderer::Initialize(rm.CreateAsync<Shader>("Assets/Shaders/LineVert.glsl|Assets/Shaders/LineFrag.glsl"));

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
	//ResourceHandle mesh2 = rm.CreateAsync<Mesh>("Assets/Meshes/M_B_44_Qishilong_skin_Skeleton.FBX");
	// faster
	ResourceHandle<Mesh> mesh = rm.CreateAsync<Mesh>("Assets/Meshes/sphere.obj");
	ResourceHandle<Texture> texture = rm.CreateAsync<Texture>("Assets/Textures/container.jpg");
	ResourceHandle<Texture> texture2 = rm.CreateAsync<Texture>("Assets/Textures/T_M_B_44_Qishilong_body01_B.png");
	ResourceHandle<Shader> shader = rm.CreateAsync<Shader>("Assets/Shaders/Cube.vert|Assets/Shaders/Cube.frag");

	ResourceHandle<Mesh> cube = rm.CreateAsync<Mesh>("Assets/Meshes/cube.obj");

	//HUD
	UIManager uiManager(window->NativeHandle());

	uiManager.GetCanvas().AddElement(std::make_unique<UIText>("This is a test", Vector2(50.f, 50.f), 100.f, FontID::Default ));


	while (!mesh.IsReady() || 
		//!mesh2.IsReady() || 
		!cube.IsReady() || 
		!texture.IsReady() || 
		!shader.IsReady())
		rm.ProcessGpuUploads();

	PhysMatHandle mat = physics.CreateMaterial("base material", 0.1f, 1.f, 0.f);

	BoxCollider collider = BoxCollider({ 1.f, 1.f, 1.f }, false);
	RigidBodyComponent body = RigidBodyComponent(1, BodyType::Dynamic);


	BoxCollider collider2 = BoxCollider({ 10.f, .5f, 10.f }, false, mat);
	RigidBodyComponent body2 = RigidBodyComponent(0, BodyType::Static);

	MeshCollider meshCollider = MeshCollider(mesh, MeshColliderType::Convex, true, mat);
	//MeshCollider meshCollider2 = MeshCollider(mesh2, MeshColliderType::Triangle, false);

	CapsuleCollider capsule(2.f, 2.f, false);

	body.SetPosition({ 0.f, 10.f, 0.f });
	body2.SetPosition({ 0.f, 0.f, 0.f });
	//body2.SetScale(LibMath::Vector3(0.001f));
	//body2.SetRotation(LibMath::Quaternion(LibMath::Degree(-90), LibMath::Radian(0), LibMath::Radian(0)));


	physics.CreateActor(meshCollider, body);
	physics.CreateActor(collider2, body2);

	float totalTime = 0.f;
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

		// Audio Listener
		if (audio)
		{
			LibMath::Vector3 pos = camera.GetPosition();
			LibMath::Vector3 front = camera.GetFront();
			audio->SetListenerPosition(
				{ pos[0],   pos[1],   pos[2] },
				{ front[0], front[1], front[2] }
			);
		}

		// Rendering
		rhi->Clear(0.12f, 0.12f, 0.15f);

		//scene.Update(camera.GetViewMatrix(),
		//	camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov),
		//	camera.GetPosition(), totalTime);

		// Draw UI
		//uiManager.RenderCanvas();

		accumulator += deltaTime;
		totalTime += deltaTime;
		while (accumulator >= 1.f / 60.f)
		{
			physics.Update(1.f / 60.f);
			accumulator -= 1.f / 60.f;
		}
		physics.SyncFromPhysx();

		shader->Use();
		texture->Bind();

		LibMath::Matrix4 view = camera.GetViewMatrix();
		LibMath::Matrix4 proj = camera.GetProjectionMatrix(window->GetAspectRatio());

		shader->SetUniform("uModel", static_cast<LibMath::Matrix4>(body.GetTransform()));
		shader->SetUniform("uView", view);
		shader->SetUniform("uProjection", proj);
		shader->SetUniform("uCameraPos", camera.GetPosition());
					
		shader->SetUniform("uLightPos", LibMath::Vector3(2.f, 3.f, 2.f));
		shader->SetUniform("uLightColor", LibMath::Vector3(1.f, 1.f, 1.f));
		shader->SetUniform("uViewPos", camera.GetPosition());
		
		shader->SetUniform("uTexture", 0);
		shader->SetUniform("uUseTexture", 1);

		mesh->Draw();

		texture2->Bind();
		shader->SetUniform("uModel", static_cast<LibMath::Matrix4>(body2.GetTransform()));
		cube->Draw();


		physics.DrawColliders();
		DebugRenderer::Get().DrawLines(rhi.get(), proj * view);

		rm.ProcessGpuUploads();

		//Swap buffer
		window->SwapBuffers();
	}

	return 0;
}
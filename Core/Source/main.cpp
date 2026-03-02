// ── Engine abstractions ───────────────────────────────────────
#include "Window.h"
#include "RHI.h"
#include "Camera.h"
#include "Scene.h"
#include "SceneGraph.h"

// ── Resource system ───────────────────────────────────────────
#include "ThreadPool.h"

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Core;
using namespace Apex::Resources;

int main()
{
	// Initialize window and RHI
	std::unique_ptr<IWindow> window(CreateWindow("Apex Renderer", 1280, 720));

	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());

	// Initialize the scene
	ThreadPool threadPool;
	Scene scene(rhi.get(), threadPool);

	Apex::Data::Object* bunny = scene.AddMesh("bunny.obj", "container.jpg", "Cube.vert|Cube.frag");
	Apex::Data::Object* tyrannosaurus = scene.AddMesh("tyra.obj", "container2.png", "Cube.vert|Cube.frag");

	bunny->SetTransform(LibMath::Matrix4::createTranslation({ 5.f, 0.f, 0.f }));
	tyrannosaurus->SetTransform(LibMath::Matrix4::createTranslation({ -5.f, 0.f, 0.f }));

	bunny->GetNode()->AddChild(tyrannosaurus->GetNode());

	// Initialize Camera
	Camera camera;
	window->SetCursorCaptured(true);

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

		scene.Update(camera.GetViewMatrix(),
			camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov),
			camera.GetPosition(), totalTime);

		window->SwapBuffers();
	}

	return 0;
}
// ── Engine abstractions ───────────────────────────────────────
#include "Window.h"
#include "RHI.h"
#include "Camera.h"
#include "Renderer.h"

// ── Resource system ───────────────────────────────────────────
#include "ThreadPool.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Shader.h"

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::Core;
using namespace Apex::Resources;

int main()
{
	// Initialize window and RHI
	std::unique_ptr<IWindow> window(CreateWindow("Apex Renderer", 1280, 720));

	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());

	// ThreadPool spins up hardware_concurrency worker threads for async loads.
	ThreadPool threadPool;
	ResourceManager resourceManager(threadPool);

	auto textureHandle = resourceManager.CreateAsync<Texture>("container.jpg");
	auto shaderHandle = resourceManager.CreateAsync<Shader>("Cube.vert|Cube.frag");

	Camera camera({ 0.f, 0.f, 3.f });
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

	// Scene objects
	Renderer cubeRenderer(rhi.get());

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

		resourceManager.ProcessGpuUploads();

		if (textureHandle.IsReady()) cubeRenderer.SetTexture(textureHandle);
		if (shaderHandle.IsReady())  cubeRenderer.SetShader(shaderHandle);

		// Clear
		rhi->Clear(0.12f, 0.12f, 0.15f);

		// Matrices from camera
		LibMath::Matrix4 view = camera.GetViewMatrix();
		LibMath::Matrix4 proj = camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov);

		// Draw cube
		cubeRenderer.Render(view, proj, camera.GetPosition(), totalTime);

		window->SwapBuffers();
	}

	return 0;
}
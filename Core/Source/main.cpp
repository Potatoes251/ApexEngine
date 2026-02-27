#include "Application.h"
//
//#include "Window.h"
// 
//#include "RHI.h"
//
//#include "Camera.h"
//
//#include "Renderer.h"
//
//using namespace Apex::Windowing;
//using namespace Apex::Rendering;

//int main()
//{
//	// Initialize window and RHI
//	std::unique_ptr<IWindow> window(CreateWindow("Apex Renderer", 1280, 720));
//
//	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());
//
//	Camera camera({ 0.f, 0.f, 3.f });
//
//	// Capture mouse for FPS look
//	window->SetCursorCaptured(true);
//
//	// Input callbacks
//	window->SetKeyCallback([&](Key key, bool pressed)
//	{
//		if (key == Key::Escape && pressed)
//			window->Close();
//	});
//
//	window->SetMouseMoveCallback([&](double dx, double dy) 
//	{
//		camera.ProcessMouse(dx, dy);
//	});
//
//	window->SetScrollCallback([&](double, double yoff) 
//	{
//		camera.ProcessScroll(yoff);
//	});
//
//	// Scene objects
//	Renderer cubeRenderer(rhi.get());
//	cubeRenderer.LoadTexture("../../Assets/Textures/container.jpg");
//
//	float totalTime = 0.f;
//	while (!window->ShouldClose()) 
//	{
//		window->PollEvents();
//		float deltaTime = window->GetDeltaTime();
//		totalTime += deltaTime;
//
//		// Camera keyboard movement
//		camera.ProcessKeyboard(
//			window->IsKeyPressed(Key::W),
//			window->IsKeyPressed(Key::S),
//			window->IsKeyPressed(Key::A),
//			window->IsKeyPressed(Key::D),
//			window->IsKeyPressed(Key::E),
//			window->IsKeyPressed(Key::Q),
//			deltaTime
//		);
//
//		// Clear
//		rhi->Clear(0.12f, 0.12f, 0.15f);
//
//		// Matrices from camera
//		LibMath::Matrix4 view = camera.GetViewMatrix();
//		LibMath::Matrix4 proj = camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov);
//
//		// Draw cube
//		cubeRenderer.Render(view, proj, camera.GetPosition(), totalTime);
//
//		window->SwapBuffers();
//	}
//
//	return 0;
//}

int main()
{
	Apex::Application app;
	app.Run();
	return 0;
}
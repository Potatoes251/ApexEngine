// ── Engine abstractions ───────────────────────────────────────
#include "Window.h"
#include "Audio.h"
#include "RHI.h"
#include "Camera.h"
#include "Scene.h"
#include "SceneGraph.h"

// ── Resource system ───────────────────────────────────────────
#include "ThreadPool.h"

#include "Hud.h"

using namespace Apex::Windowing;
using namespace Apex::Rendering;
using namespace Apex::UserInterface;
using namespace Apex::Core;
using namespace Apex::Resources;
using namespace Apex::Audio;

int main()
{
	// Initialize
	std::unique_ptr<IWindow> window(CreateWindow("Apex Renderer", 1280, 720));
	std::unique_ptr<IRHI> rhi(CreateOpenGLRHI());
	std::unique_ptr<IAudioEngine> audio(CreateAudioEngine());

	// Initialize the scene
	ThreadPool threadPool;
	Scene scene(rhi.get(), threadPool);

	Apex::Data::Object* bunny = scene.AddMesh("bunny.obj", "container.jpg", "Cube.vert|Cube.frag");
	Apex::Data::Object* tyrannosaurus = scene.AddMesh("tyra.obj", "container2.png", "Cube.vert|Cube.frag");

	bunny->SetTransform(LibMath::Matrix4::createTranslation({ 5.f, 0.f, 0.f }));
	tyrannosaurus->SetTransform(LibMath::Matrix4::createTranslation({ -5.f, 0.f, 0.f }));

	bunny->GetNode()->AddChild(tyrannosaurus->GetNode());

	// Audio setup 
	if (audio)
	{
		audio->Preload("fragments of hope.ogg");
	}

	SoundHandle backgroundMusic = audio ? audio->Play2D("fragments of hope.ogg", 
		{ .m_loop = true, .m_volume = 1.f }) : INVALID_SOUND;

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

	// GUI
	//std::unique_ptr<IGUI> gui(CreateGUI(window->NativeHandle()));


	//HUD
	UIManager uiManager(window->NativeHandle());

	uiManager.GetCanvas().AddElement(std::make_unique<UIText>("This is a test", Vector2(50.f, 50.f), 100, FontID::Default ));


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

		scene.Update(camera.GetViewMatrix(),
			camera.GetProjectionMatrix(window->GetAspectRatio(), camera.m_fov),
			camera.GetPosition(), totalTime);

		window->SwapBuffers();

		// Draw UI
		uiManager.RenderCanvas();

		//Swap buffer
		window->SwapBuffers();
	}

	return 0;
}
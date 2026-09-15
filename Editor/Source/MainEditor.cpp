//#include "Editor.h"
//
//#include "Window.h"
//
//#include "ResourceManager.h"
//
//#include <filesystem>
//#include <iostream>
//
//#ifdef _WIN32
//
//#include <windows.h>
//
//#endif
//
//static std::filesystem::path GetExeDir(const char* argv0)
//{
//    return std::filesystem::path(argv0).parent_path();
//}
//
//// Walk up from the exe until we find the true project root.
//// We identify it by the presence of .git  folder,
//// which only exist at the project root, unlike Assets/ which
//// may be copied into build output folders.
//static std::filesystem::path FindProjectRoot(std::filesystem::path dir)
//{
//    while (dir.has_parent_path())
//    {
//        if (std::filesystem::exists(dir / ".git"))
//            return dir;
//        std::filesystem::path parent = dir.parent_path();
//        if (parent == dir) break;
//        dir = parent;
//    }
//    return dir;
//}
//
//int main(int /*argc*/, char* argv[])
//{
//    std::filesystem::path exeDir = GetExeDir(argv[0]);
//
//    std::filesystem::path projectRoot = FindProjectRoot(exeDir);
//    std::filesystem::current_path(projectRoot);
//
//    Apex::Editor::Editor editor;
//    if (!editor.InitWindow())
//        return 1;
//
//    Apex::Application app(editor.GetWindow(), editor.GetGUI());
//
//	app.LoadScene(false, false);
//
//    if (!editor.InitPanels(app))
//        return 1;
//
//    while (!editor.GetWindow()->ShouldClose())
//    {
//        // Processes events for all windows must be unconditional
//        // so the editor window stays responsive when the game is stopped.
//        Apex::Windowing::PollAllEvents();
//        editor.Update();
//        // Editor render - always runs while editor window is open.
//        editor.Render();
//        app.GetResourceManager()->ProcessGpuUploads();
//    }
//
//    editor.Shutdown();
//
//	return 0;
//}

#include <fstream>
#include <iostream>

int main() {
    const int N = 150;           // vertices per side
    const float size = 10.0f;    // total plane size
    const float step = size / (N - 1);
    const float half = size / 2.0f;

    std::ofstream out("plane.obj");
    if (!out) {
        std::cerr << "Failed to open output file\n";
        return 1;
    }

    out << "# Plane " << N << "x" << N << " vertices\n";
    out << "o Plane\n";

    // Vertices (lying flat on XZ plane, Y = 0)
    for (int z = 0; z < N; ++z) {
        for (int x = 0; x < N; ++x) {
            float px = -half + x * step;
            float pz = -half + z * step;
            out << "v " << px << " 0.0 " << pz << "\n";
        }
    }

    // Texture coordinates
    for (int z = 0; z < N; ++z) {
        for (int x = 0; x < N; ++x) {
            float u = static_cast<float>(x) / (N - 1);
            float v = static_cast<float>(z) / (N - 1);
            out << "vt " << u << " " << v << "\n";
        }
    }

    // Normal (flat plane, all pointing up)
    out << "vn 0.0 1.0 0.0\n";

    // Faces (two triangles per quad), OBJ indices are 1-based
    for (int z = 0; z < N - 1; ++z) {
        for (int x = 0; x < N - 1; ++x) {
            int i0 = z * N + x + 1;
            int i1 = z * N + (x + 1) + 1;
            int i2 = (z + 1) * N + (x + 1) + 1;
            int i3 = (z + 1) * N + x + 1;

            out << "f " << i0 << "/" << i0 << "/1 "
                << i1 << "/" << i1 << "/1 "
                << i2 << "/" << i2 << "/1\n";

            out << "f " << i0 << "/" << i0 << "/1 "
                << i2 << "/" << i2 << "/1 "
                << i3 << "/" << i3 << "/1\n";
        }
    }

    out.close();
    std::cout << "Generated plane.obj with " << N * N << " vertices and "
        << 2 * (N - 1) * (N - 1) << " triangles\n";

    return 0;
}
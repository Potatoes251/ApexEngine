# Engine Project





### Project characteristics



**Name**: Apex Engine

**Game genre**: 3D/puzzle platformer, pseudo mario, third-person

**Animations**: on the player character

**Controls**: WASD/ZQSD to move, space to jump, left-click to shoot

**Physical interactions**: player can shoot a ball to interact with objects, player collision with environment

**3D environment**: map containing obstacles/puzzles

**Win condition**: go through whole level and reach the end

**Lose condition**: lose all hit points

**HUD**: hit points + score

**Save**: 1 level with multiple checkpoints, high score

**Menu**: restart/quit buttons, press esc to access it anytime. menu also shows up on the end screen (win/lose)

**Sound**: when player shoots, jumps, takes damage, wins/loses, pickups items, background music

**Score**: score can be increased by picking up items on the map





### Gameplay bonus



* Projectile system





### Engine bonus

* Shadow Mapping
* Skybox
* Resources visualizer





### External libraries



#### Rendering API



Chosen API: **OpenGL**



*Reasons*



* Already used it several times since GP1
* Cross-platform
* Large ecosystem and documentation
* Fits well with current engine architecture



*Integration*



* Linked via glad to load functions
* Context created through GLFW
* OpenGL headers included in rendering module



*Documentation*



* Official: https://registry.khronos.org/OpenGL/
* LearnOpenGL: https://learnopengl.com



*How to use*



1. Create an OpenGL context (via GLFW)
2. Load OpenGL functions with glad
3. Create buffers (VBO, VAO, EBO)
4. Compile shaders
5. Render using glDrawElements or glDrawArrays





#### Windowing \& Input



Chosen API: **GLFW**



*Reasons*



* Already used and stable
* Lightweight
* Works with multiple rendering APIs
* Very simple input management





*Integration*



* Add it as a linked static library
* Initialize with glfwInit()





*Documentation*



* https://www.glfw.org/documentation.html



*How to use*



1. Call glfwInit()
2. Create window with glfwCreateWindow()
3. Register callbacks (keyboard, mouse)
4. Use glfwPollEvents() and glfwSwapBuffers() in game loop
5. Destroy window with glfwWindowShouldClose() and then glfwDestroyWindow()







#### Graphical User Interface



Chosen API: **Dear ImGui**



*Reasons*



* Immediate mode (perfect for engine tools)
* Easy debug UI
* Lightweight integration
* Used several times in previous projects





*Integration*



* Add ImGui source files to project
* Use ImGui OpenGL + GLFW backend
* Initialize in engine initialization





*Documentation*



* https://github.com/ocornut/imgui



*How to use*



1. Call ImGui::NewFrame() every frame
2. Create UI elements (ImGui::Begin(), ImGui::Button(), etc.)
3. Call ImGui::Render()





#### Physics



Chosen API: **PhysX**



*Reasons*



* Better performance
* Industry-level physics engine
* More realistic simulations
* Good for advanced physics features





*Integration*



* Download PhysX SDK
* Link PhysX static libraries
* Initialize PxFoundation and PxPhysics





*Documentation*



* https://nvidiagameworks.github.io/simulation.html#physx



*How to use*



1. Initialize Scene:
	1. Create PxFoundation (The core memory/error manager)
	2. Create PxPhysics (The factory for all objects)
	3. Create PxScene (Define gravity and the CPU dispatcher)
2. Setup level:
	1. Create PxMaterial (Define friction and bounciness)
	2. Create PxRigidActor (Static, Dynamic, or Kinematic)
	3. Add actors to scene via PxScene::addActor()
3. Update gameplay each frame:
	1. Update Kinematics via setKinematicTarget() (For moving platforms)
	2. Update Global Transform via setGlobalPose() (For teleporting or resetting actors)
	3. Call PxScene::simulate() to start the physics step
	4. Call PxScene::fetchResults() to wait for the math to finish
	5. Sync Graphics by calling getGlobalPose() to move your 3D meshes





#### Audio



Chosen API: **irrKlang**



*Reasons*



* Already known
* Simple API
* Fast integration
* Suitable for game engine scale





*Integration*



* Include irrKlang headers
* Link irrKlang library
* Create sound engine with createIrrKlangDevice()





*Documentation*



* https://www.ambiera.com/irrklang/docu/index.html



*How to use*



1. Create ISoundEngine
2. Play sound with play2D() or play3D()
3. Manage sound instances if needed
4. Destroy ISoundEngine with drop()





#### 3D Models



Chosen API: **FBX SDK**



*Reasons*



* Better animation control
* Direct support for FBX format
* Industry standard format
* More precise access to animation data





*Integration*



* Download Autodesk FBX SDK
* Link FBX libraries
* Include FBX headers





*Documentation*



* https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX\_API\_Reference\_cpp\_ref\_index\_html



*How to use*



1. Create FbxManager with FbxManager::Create()
2. Create FbxScene with FbxScene::Create()
3. Load file using FbxImporter
4. Parse nodes and extract meshes, bones, animations





#### Textures



Chosen API: **stb\_image**



*Reasons*



* Header-only
* Very lightweight
* Already used
* Supports common formats



*Integration*



* Include stb\_image.h
* Define STB\_IMAGE\_IMPLEMENTATION in one source file



*Documentation*



* https://github.com/nothings/stb



*How to use*



* Call stbi\_load() to load image
* Create OpenGL texture
* Upload data with glTexImage2D()
* Free memory with stbi\_image\_free()





#### Scripting



Chosen API: **Lua**



*Reasons*



* Simple integration
* Lightweight
* Widely used in game industry
* Good performance





*Integration*



* Use Lua library or sol2 binding
* Link Lua library
* Create Lua state with luaL\_newstate()





*Documentation*



* https://www.lua.org/docs.html



*How to use*



1. Create Lua state with lua\_open()
2. Load script with luaL\_dofile()
3. Bind C++ functions with lua\_register()
4. Call Lua functions from C++:
	* lua_getglobal() to push function on lua stack
	* lua_push...() to push function argument on lua stack: lua_pushinteger(), lua_pushstring() etc.
	* lua_pcall() to call function with arguments that are on top of lua stack
	* lua_pop() to remove return value from stack once it is stored in c++





#### Mathematics



Chosen Library: **LibMath (Mateo)**



*Reason*



* Best working and clean one



*Integration*



* Included directly in engine core
* Used in rendering, physics, transforms



*Documentation*



* Internal project documentation



*How to use*



1. Use Vector3, Vector4, Matrix4
2. Use transformation functions (translation, rotation, scale)
3. Use matrix multiplication for MVP computation

#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics1/Color.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/System.h>
#include <Kore/Log.h>

#include "MeshObject.h"

using namespace Kore;
using namespace Kore::Graphics4;

namespace {
	
	const int width = 1024;
	const int height = 768;
	
	double startTime;
	double lastTime;
	
	VertexStructure structure;
	Shader* vertexShader;
	Shader* fragmentShader;
	PipelineState* pipeline;
	
	// Uniform locations
	TextureUnit tex;
	ConstantLocation pLocation;
	ConstantLocation vLocation;
	ConstantLocation mLocation;
	
	bool left, right = false;
	bool down, up = false;
	bool forward, backward = false;
	bool rotate = false;
	int mousePressX, mousePressY = 0;
	
	MeshObject* cube;
	MeshObject* avatar;
	
	vec3 playerPosition = vec3(0, 10, 30);
	vec3 globe = vec3(Kore::pi, Kore::pi, Kore::pi/2);
	int frame = 0;
	
	float angle = 0;
	vec4 desPos1 = vec4(0, 0, 0, 0);
	vec4 desPos2 = vec4(0, 0, 0, 0);
	
	void update() {
		float t = (float)(System::time() - startTime);
		double deltaT = t - lastTime;
		lastTime = t;
		
		const float speed = 0.5;
		if (left) {
			playerPosition.x() -= speed;
		}
		if (right) {
			playerPosition.x() += speed;
		}
		if (forward) {
			playerPosition.z() += speed;
		}
		if (backward) {
			playerPosition.z() -= speed;
		}
		if (up) {
			playerPosition.y() += speed;
		}
		if (down) {
			playerPosition.y() -= speed;
		}
		
		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Graphics1::Color::Black, 1.0f, 0);
		
		Graphics4::setPipeline(pipeline);
		
		// projection matrix
		mat4 P = mat4::Perspective(45, (float)width / (float)height, 0.01f, 1000);
		
		// view matrix
		vec3 lookAt = playerPosition + vec3(0, 0, -1);
		mat4 V = mat4::lookAt(playerPosition, lookAt, vec3(0, 1, 0));
		V *= mat4::Rotation(globe.x(), globe.y(), globe.z());
		
		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);
		
		Graphics4::setMatrix(mLocation, cube->M);
		cube->render(tex);
		
		Graphics4::setMatrix(mLocation, avatar->M);
		/*avatar->setAnimation(frame);
		frame++;
		if (frame > 200) frame = 0;*/
		avatar->animate(tex, deltaT);
		
		angle += 0.05f;
		float radius = 2;
		desPos1 = vec4(2 + radius * Kore::cos(angle), -4, 3 + radius * Kore::sin(angle), 1);
		radius = 1;
		desPos2 = vec4(-4 + radius * Kore::cos(angle), -3, 11 + radius * Kore::sin(angle), 1);
		avatar->setDesiredPosition(49, desPos1);		// Left foot 49, right foot 53, vec4(2, -4, 3, 1)
		avatar->setDesiredPosition(29, desPos2);		// Right hand, vec4(-4, -3, 11, 1)
		
		//cube->drawVertices(cube->M, V, P, width, height);
		avatar->drawJoints(avatar->M, V, P, width, height, true);
		
		Graphics4::end();
		Graphics4::swapBuffers();
	}
	
	void keyDown(KeyCode code, wchar_t character) {
		switch (code) {
			case Kore::Key_Left:
			case Kore::Key_A:
				left = true;
				break;
			case Kore::Key_Right:
			case Kore::Key_D:
				right = true;
				break;
			case Kore::Key_Down:
				down = true;
				break;
			case Kore::Key_Up:
				up = true;
				break;
			case Kore::Key_W:
				forward = true;
				break;
			case Kore::Key_S:
				backward = true;
				break;
				
			case Key_L:
				Kore::log(Kore::LogLevel::Info, "Position: (%.2f, %.2f, %.2f)", playerPosition.x(), playerPosition.y(), playerPosition.z());
				Kore::log(Kore::LogLevel::Info, "Rotation: (%.2f, %.2f, %.2f)", globe.x(), globe.y(), globe.z());
			default:
				break;
		}
	}
	
	void keyUp(KeyCode code, wchar_t character) {
		switch (code) {
			case Kore::Key_Left:
			case Kore::Key_A:
				left = false;
				break;
			case Kore::Key_Right:
			case Kore::Key_D:
				right = false;
				break;
			case Kore::Key_Down:
				down = false;
				break;
			case Kore::Key_Up:
				up = false;
				break;
			case Kore::Key_W:
				forward = false;
				break;
			case Kore::Key_S:
				backward = false;
				break;
			default:
				break;
		}
	}
	
	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		float rotationSpeed = 0.01;
		if (rotate) {
			globe.x() += (float)((mousePressX - x) * rotationSpeed);
			globe.z() += (float)((mousePressY - y) * rotationSpeed);
			mousePressX = x;
			mousePressY = y;
		}
	}
	
	void mousePress(int windowId, int button, int x, int y) {
		rotate = true;
		mousePressX = x;
		mousePressY = y;
	}
	
	void mouseRelease(int windowId, int button, int x, int y) {
		rotate = false;
	}
	
	void init() {
		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		//FileReader vs("shader_lighting.vert");
		//FileReader fs("shader_lighting.frag");
		vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
		
		// This defines the structure of your Vertex Buffer
		structure.add("pos", Float3VertexData);
		structure.add("tex", Float2VertexData);
		structure.add("nor", Float3VertexData);
		
		pipeline = new PipelineState;
		pipeline->inputLayout[0] = &structure;
		pipeline->inputLayout[1] = nullptr;
		pipeline->vertexShader = vertexShader;
		pipeline->fragmentShader = fragmentShader;
		pipeline->depthMode = ZCompareLess;
		pipeline->depthWrite = true;
		pipeline->compile();
		
		tex = pipeline->getTextureUnit("tex");
		
		pLocation = pipeline->getConstantLocation("P");
		vLocation = pipeline->getConstantLocation("V");
		mLocation = pipeline->getConstantLocation("M");
		
		cube = new MeshObject("cube.ogex", "", structure);
		cube->M = mat4::Translation(5, 0, 0);
		avatar = new MeshObject("avatar/avatar_skeleton.ogex", "avatar/", structure);
		avatar->M = mat4::Translation(-5, 0, 0);
		
		Graphics4::setTextureAddressing(tex, Graphics4::U, Repeat);
		Graphics4::setTextureAddressing(tex, Graphics4::V, Repeat);
	}
	
}

int kore(int argc, char** argv) {
	System::init("BodyTracking", width, height);
	
	init();
	
	System::setCallback(update);
	
	startTime = System::time();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;
	
	System::start();
	
	return 0;
}

#pragma once
#include <glm/glm.hpp>
#include "Camera.hpp"
#include "Model.hpp"
#include "IShader.hpp"

class Model;
class IShader;
//kind of like a context for application
class RenderState{
	public:
	bool rotate = false;
	bool scale = false;
	bool translate = false;
	bool useDepth = true;
	IShader* boundShader = nullptr;
	std::vector<glm::vec4> boundVerts;
	std::vector<unsigned int> boundIndices;
	
	glm::mat4 Viewport = glm::mat4(1.0f);
	glm::mat4 ModelView = glm::mat4(1.0f);
	glm::mat4 Projection = glm::mat4(1.0f);

	unsigned char cullmode = 0;
	// backface 0, frontface 1
	unsigned char drawmode = 0;
	//phong, gouraud, halftone, wireframe, custom
	unsigned char clipmode = 0;
	//100% 0, 75% 1, 50% 2, 25% 3, 0% 4
};

class Context{
	public:
	//Context Variables
	int lastTime = 0;
	int deltaTime;
	RenderState renderstate;

	//renderables vars
	unsigned int selectedpoly = 0;
	std::vector<Model> models;

	//camera vars
	bool freecam = false;
	bool viewpersp = false;
	bool viewdepth = false;
	Camera freeorthocam;
	Camera freeperspcam;
	Camera xyCam;
	Camera xzCam;
	Camera yzCam;
	//mouse position vars
	bool first = true;
	float lastX = 0;
	float lastY = 0;

	//image buffer
	float * image;
	float * depth;
	int xres = 0;
	int yres = 0;
	int rowsize = 0;

	//user changable clippts
	glm::vec2 clippt1 = glm::vec2(0.0f, 0.0f);
	glm::vec2 clippt2 = glm::vec2(0.0f, 0.0f);
};
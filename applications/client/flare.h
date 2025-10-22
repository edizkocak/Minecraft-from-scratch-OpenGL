#pragma once
#include <cppgl.h>

void init_flare();

class Flare
{
public:
	//Flare(Texture2D texture, float scale);
	Flare(Texture2D texture, float scale);
	Flare();



	Texture2D texture;
	float scale;

	static Drawelement prototype;
	glm::vec2 screenPos = glm::vec2(0.0, 0.0);
	glm::mat4 model;

	GLuint vao;
	GLuint vbo;

	void draw();

};

class FlareHelper {

public:
	std::vector<std::shared_ptr<Flare>> flares;
	std::vector<Texture2D> flareTextures;


	glm::vec2 CENTER_SCREEN = glm::vec2(0.5f, 0.5f);

	//Flare flareTextures[1];
	float spacing;

	//FlareRenderer renderer;

	FlareHelper(float spacing);

	void render(glm::vec3 sunWorldPos);

	void calcFlarePositions(glm::vec2 sunToCenter, glm::vec2 sunCoords);

	glm::vec2 convertToScreenSpace(glm::vec3  worldPos, glm::mat4 viewMat, glm::mat4 projectionMat);



};








/*
class Quad2D {
public:
    Quad2D ();
    ~Quad2D();

    void draw();
    GLuint vao;
    GLuint vbo;
};
*/
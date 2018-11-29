#pragma once
#include "OBJ_Loader.h"
#include <glm/glm.hpp>
#include "IShader.hpp"
class IShader;

class Model{
    public:
    objl::Loader Loader;
    IShader* shader = nullptr;
    glm::mat4 translate = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);
    glm::mat4 rotate = glm::mat4(1.0f);

    Model(char* path, IShader* shade);
    Model(char* path);
    ~Model();
    
    void setupModel(std::vector<glm::vec4> & tris, std::vector<unsigned int> & indices);
};
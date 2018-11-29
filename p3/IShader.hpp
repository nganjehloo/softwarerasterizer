#pragma once
#include <glm/glm.hpp>
#include "Context.hpp"
class Context;

class IShader {
    public:
    virtual ~IShader();
    virtual glm::vec4 vertex(glm::vec4 & vertex, unsigned int mdindex, unsigned int msindex, unsigned int vindex, Context & ctx) = 0;
    virtual bool fragment(glm::vec3 bar, glm::vec3 &color) = 0;
};
#pragma once

#include "IShader.hpp"
#include "Context.hpp"
#include <glm/glm.hpp>

#include <vector>
#include "Context.hpp"
class IShader;

class Gouraud : public IShader {
    public:
    glm::mat3x4 varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
    glm::mat3 varying_nrm; // normal per vertex to be interpolated by FS
    glm::mat3 ndc_tri;     // triangle in normalized device coordinates
    std::vector<glm::vec3> dir_light;
    std::vector<glm::vec3> point_light;

    glm::vec4 vertex(glm::vec4 & vertex, unsigned int mdindex, unsigned int msindex, unsigned int vindex, Context & ctx);

    bool fragment(glm::vec3 bar, glm::vec3 &color);
};
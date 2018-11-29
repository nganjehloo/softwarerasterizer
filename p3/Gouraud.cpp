#include "Gouraud.hpp"

    glm::vec4 Gouraud::vertex(glm::vec4 & vertex, unsigned int mdindex, unsigned int msindex, unsigned int vindex, Context & ctx) {
        glm::vec4 pos = vertex;
        
        glm::vec4 nrm = glm::vec4(ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.X, 
            ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.Y, 
            ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.Z, 0.0f);
        
        //printf("Pre Transform\n");
        //printf("vertex pos: %f, %f, %f\n", pos.x, pos.y, pos.z);
        //printf("vertex nrm: %f, %f, %f\n", nrm.x, nrm.y, nrm.z);
        
        glm::vec3 tnrm = nrm;
        varying_nrm[vindex%3] = tnrm;

        glm::vec4 gl_Vertex = ctx.renderstate.Viewport*ctx.renderstate.Projection*ctx.renderstate.ModelView*pos;
        varying_tri[vindex%3] = gl_Vertex;

        for(int i = 0; i < point_light.size(); ++i){
             point_light[i] = ctx.renderstate.Viewport*ctx.renderstate.Projection*ctx.renderstate.ModelView*glm::vec4(point_light[i], 1.0f);
        }

        ndc_tri[vindex%3] = glm::vec3(gl_Vertex/gl_Vertex.w);
        //printf("Post Transform\n");
        //printf("vertex pos: %f, %f, %f\n", gl_Vertex.x, gl_Vertex.y, gl_Vertex.z);
        //printf("vertex nrm: %f, %f, %f\n", tnrm.x, tnrm.y, tnrm.z);
        return gl_Vertex; // transform it to screen coordinates
    }

    bool Gouraud::fragment(glm::vec3 bar, glm::vec3 &color) {
        glm::vec3 normal = glm::normalize(varying_nrm*bar);   // interpolate intensity for the current pixel
        glm::vec3 position = varying_tri*bar;
        //printf("Post Interpolated\n");
        //printf("vertex pos: %f, %f, %f\n", position.x, position.y, position.z);
        //printf("vertex nrm: %f, %f, %f\n", normal.x, normal.y, normal.z);
        float lighting = 0;
        
        for(int i = 0; i < dir_light.size(); ++i){
            lighting += glm::clamp(glm::dot(normal, glm::normalize(dir_light[i])), 0.0f, 1.0f);
        }
       // printf("light intensity: %f\n", lighting);
        for(int i = 0; i < point_light.size(); ++i){
            lighting +=  glm::clamp(glm::dot(normal, glm::normalize(point_light[i]-position)), 0.0f, 1.0f);
        }
        //color *= lighting;
        color = normal*0.1f + glm::vec3(0.3f, 0.4f, 0.2f)*lighting + glm::vec3(0.1f, 0.0f, 0.1f); //glm::vec3(lighting, lighting, lighting);
        //color = normal;
        return false;                              // no, we do not discard this pixel
    }
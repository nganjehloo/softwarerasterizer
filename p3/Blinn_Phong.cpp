#include "Blinn_Phong.hpp"

    glm::vec4 Blinn_Phong::vertex(glm::vec4 & vertex, unsigned int mdindex, unsigned int msindex, unsigned int vindex, Context & ctx) {
        
        glm::vec4 nrm = glm::vec4(ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.X, 
            ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.Y, 
            ctx.models[mdindex].Loader.LoadedMeshes[msindex].Vertices[vindex].Normal.Z, 0.0f);
        
        //printf("Pre Transform\n");
        //printf("vertex pos: %f, %f, %f\n", pos.x, pos.y, pos.z);
        //printf("vertex nrm: %f, %f, %f\n", nrm.x, nrm.y, nrm.z);
        
        glm::vec3 tnrm = nrm;
        varying_nrm[vindex%3] = tnrm;

        glm::vec4 gl_Vertex = ctx.renderstate.Viewport*ctx.renderstate.Projection*ctx.renderstate.ModelView*vertex;
        varying_tri[vindex%3] = ctx.renderstate.ModelView*vertex;
        varying_tri[vindex%3] = varying_tri[vindex%3]/varying_tri[vindex%3].w;

        for(int i = 0; i < point_light.size(); ++i){
             point_light[i] = ctx.renderstate.Viewport*ctx.renderstate.Projection*ctx.renderstate.ModelView*glm::vec4(point_light[i], 1.0f);
        }
        
        ndc_tri[vindex%3] = glm::vec3(gl_Vertex/gl_Vertex.w);
        //printf("Post Transform\n");
        //printf("vertex pos: %f, %f, %f\n", gl_Vertex.x, gl_Vertex.y, gl_Vertex.z);
        //printf("vertex nrm: %f, %f, %f\n", tnrm.x, tnrm.y, tnrm.z);
        if(ctx.viewpersp){
            return glm::vec4(gl_Vertex/gl_Vertex.w); // transform it to screen coordinates
        }else{
            return gl_Vertex;
        }
    }

    bool Blinn_Phong::fragment(glm::vec3 bar, glm::vec3 &color) {
        glm::vec3 normal = glm::normalize(varying_nrm*bar);   // interpolate intensity for the current pixel
        glm::vec3 position = varying_tri*bar;
        //printf("Post Interpolated\n");
        //printf("vertex pos: %f, %f, %f\n", position.x, position.y, position.z);
        //printf("vertex nrm: %f, %f, %f\n", normal.x, normal.y, normal.z);
        float lighting = 0;
        glm::vec3 viewDir = glm::normalize(viewPos-position);
        //printf("View: %f, %f, %f,\n", viewDir.x, viewDir.y, viewDir.z);
        for(int i = 0; i < dir_light.size(); ++i){
            glm::vec3 halfangle = glm::normalize(glm::normalize(dir_light[i]) - viewDir);
            float specularity = glm::pow(glm::max(glm::dot(normal, halfangle), 0.0f),128.0f);
            lighting += glm::clamp(glm::dot(normal, glm::normalize(dir_light[i])), 0.0f, 1.0f)*0.7f  + specularity;
        }
       // printf("light intensity: %f\n", lighting);
        for(int i = 0; i < point_light.size(); ++i){
            lighting +=  glm::clamp(glm::dot(normal, glm::normalize(point_light[i]-position)), 0.0f, 1.0f);
        }
        //color *= lighting;
        color = normal*0.1f + glm::vec3(0.5f, 0.8f, 0.6f)*lighting + glm::vec3(0.1f, 0.0f, 0.1f);
        //color = normal;
        return false;                              // no, we do not discard this pixel
    }
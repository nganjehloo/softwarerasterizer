#include "Model.hpp"

Model::~Model(){}

Model::Model(char* path){
    Loader.LoadFile(path);
}

Model::Model(char* path, IShader* shade){
    Loader.LoadFile(path);
    shader = shade;
}

void Model::setupModel(std::vector<glm::vec4> & tris, std::vector<unsigned int> & indices){

    for(int i = 0; i < Loader.LoadedMeshes.size(); ++i){
        objl::Mesh curMesh = Loader.LoadedMeshes[i];
        for(int j = 0; j < curMesh.Indices.size(); j+=3){
            int vi0 = curMesh.Indices[j];
            int vi1 = curMesh.Indices[j+1];
            int vi2 = curMesh.Indices[j+2];

            glm::vec4 v0 = glm::vec4(curMesh.Vertices[vi0].Position.X, curMesh.Vertices[vi0].Position.Y, curMesh.Vertices[vi0].Position.Z, 1.0f);
            glm::vec4 v1 = glm::vec4(curMesh.Vertices[vi1].Position.X, curMesh.Vertices[vi1].Position.Y, curMesh.Vertices[vi1].Position.Z, 1.0f);
            glm::vec4 v2 = glm::vec4(curMesh.Vertices[vi2].Position.X, curMesh.Vertices[vi2].Position.Y, curMesh.Vertices[vi2].Position.Z, 1.0f);
          
            indices.push_back(vi0);
            indices.push_back(vi1);
            indices.push_back(vi2);
            tris.push_back(v0);
            tris.push_back(v1);
            tris.push_back(v2);
        }
    }
}
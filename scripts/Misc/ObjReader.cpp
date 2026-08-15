#include "../headers/Hittables/Triangle.cuh"
#include "../headers/Util/ObjReader.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>



std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}



glm::vec3 readInLine(const std::string& text) {
    std::stringstream ss(text);
    std::string prefix;
    glm::vec3 vec;

    ss >> prefix >> vec.x >> vec.y >> vec.z;

    return vec;
}

FaceNormalIndex readInFace(const std::string& text) {
    FaceNormalIndex result{};
    std::stringstream ss(text);

    std::string prefix;
    std::string token0;
    std::string token1;
    std::string token2;

    ss >> prefix >> token0 >> token1 >> token2;

    std::vector<std::string> parts0 = split(token0, '/');
    std::vector<std::string> parts1 = split(token1, '/');
    std::vector<std::string> parts2 = split(token2, '/');

    result.faceIndex = glm::vec3(
        std::stof(parts0[0]),
        std::stof(parts1[0]),
        std::stof(parts2[0])
    );

    result.normalIndex = glm::vec3(
        std::stof(parts0[2]),
        std::stof(parts1[2]),
        std::stof(parts2[2])
    );

    return result;
}




std::vector<Raytracer::Triangle> FileOps::ReadInObj(std::string filePath){
    std::ifstream objFile(filePath);

    std::vector<Raytracer::Triangle> triangles;
    
    if(!objFile.is_open()){
        std::cerr << "could not open OBJ file at " << filePath;
        return triangles;
    }
    std::string text;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vNormals;
// list of vertices
// list of faces (indices)
// list of normals
    while (getline (objFile, text)){
        std::stringstream ss(text);
        std::string prefix;
        ss >> prefix;

        auto tok = strToTok.find(prefix);
        if(tok == strToTok.end()) continue;
        switch(tok->second){
            case(ObjToken::VERTEX):{
                glm::vec3 vertex = readInLine(text);
                vertices.push_back(vertex);
                break;
            }

            case(ObjToken::FACE):{
                FaceNormalIndex fnIndex = readInFace(text);
                Raytracer::Triangle triangle;

                // account for 1 indexed file format
                triangle.p1 = vertices.at(fnIndex.faceIndex.x - 1);
                triangle.p2 = vertices.at(fnIndex.faceIndex.y - 1);
                triangle.p3 = vertices.at(fnIndex.faceIndex.z - 1);
                
                if(!vNormals.empty()){
                    triangle.p1Normal = vNormals.at(fnIndex.normalIndex.x - 1);
                    triangle.p2Normal = vNormals.at(fnIndex.normalIndex.y - 1);
                    triangle.p3Normal = vNormals.at(fnIndex.normalIndex.z - 1);
                }

                triangles.push_back(triangle);
                break;
            }

            case(ObjToken::VNORMAL):{
                glm::vec3 vertexNormal = readInLine(text);
                vNormals.push_back(vertexNormal);
                break;
            }
        }
    }

    objFile.close();
    return triangles;
}
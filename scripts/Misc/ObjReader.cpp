#include "../headers/Hittables/Triangle.cuh"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

glm::vec3 readInLine(const std::string& text) {
    std::stringstream ss(text);
    std::string prefix;
    glm::vec3 vec;

    ss >> prefix >> vec.x >> vec.y >> vec.z;

    return vec;
}

std::vector<Raytracer::Triangle> ReadInObj(std::string filePath){
    std::ifstream objFile(filePath);

    std::vector<Raytracer::Triangle> triangles;
    
    if(!objFile.is_open()){
        std::cerr << "could not open OBJ file at " << filePath;
        return triangles;
    }
    const char vertex = 'v';
    const char face = 'f';

    std::string text;

    std::vector<glm::vec3> vertices;

    while (getline (objFile, text)){
        
        const char firstChar = text.at(0); 

        // vertex
        switch(firstChar){
            case(vertex):{
                glm::vec3 vertex = readInLine(text);
                vertices.push_back(vertex);
                break;
            }
            case(face):{
                glm::vec3 faceIndices = readInLine(text);
                Raytracer::Triangle triangle;

                // account for 1 indexed file format
                triangle.p1 = vertices.at(faceIndices.x - 1);
                triangle.p2 = vertices.at(faceIndices.y - 1);
                triangle.p3 = vertices.at(faceIndices.z - 1);
                
                triangles.push_back(triangle);
            }
        }
    }

    objFile.close();
    return triangles;
}
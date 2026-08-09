#include "../headers/Hittables/Triangle.cuh"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

glm::vec3 readInVertex(const std::string& text) {
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
        // vertex
        const char firstChar = text.at(0); 
        // vertex

        switch(firstChar){
            case(vertex):{
                glm::vec3 vertex = readInVertex(text);
                vertices.push_back(vertex);
                break;
            }
            case(face):{
                
            }
        }
        std::cout << text << "\n";
    }

    objFile.close();
    return triangles;
}
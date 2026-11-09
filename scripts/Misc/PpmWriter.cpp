

#include <iostream>
#include <vector>
#include <glm/ext/vector_float3.hpp>
#include <fstream>

#include "../headers/Util/PpmWriter.hpp"

void FileOps::writeColorsToPPM(std::vector<glm::vec3> colors, int height, int width, std::string filePath){

    std::ofstream ppmFile(filePath);

    ppmFile << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i);

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        ppmFile << ir << ' ' << ig << ' ' << ib << '\n';
    }
    ppmFile << std::flush;
}

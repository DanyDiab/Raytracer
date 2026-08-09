#pragma once


#include <string>
#include <vector>
#include "../Hittables/Triangle.cuh"

std::vector<Raytracer::Triangle> ReadInObj(std::string filePath);
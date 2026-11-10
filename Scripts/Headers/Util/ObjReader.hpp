#pragma once


#include <map>
#include <string>
#include <vector>
#include "../Hittables/Triangle.cuh"

enum ObjToken{
    FACE,
    VERTEX,
    VNORMAL
};

struct FaceNormalIndex{
    glm::vec3 faceIndex;
    glm::vec3 normalIndex;
};

namespace FileOps{
    std::vector<Raytracer::Triangle> ReadInObj(std::string filePath);

    inline std::map<std::string, ObjToken> strToTok = {{"f", ObjToken::FACE}, {"v", ObjToken::VERTEX}, {"vn", ObjToken::VNORMAL}};
}

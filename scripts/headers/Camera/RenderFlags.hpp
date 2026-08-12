#pragma once
#include <iostream>
#include <map>
#include <string>

namespace Flags{

    enum RenderFlags{
        DebugNormals = 1, // show normals rather than colors
        TestFlag = 1 << 1 // not used
    };
    
    // map string inputs to flag
    inline std::map<std::string, RenderFlags> strToFlag = {{"-DN", RenderFlags::DebugNormals}};
    inline std::map<RenderFlags, std::string> flagToName = {{RenderFlags::DebugNormals, "Debug Normals"}};

    inline int ProcessFlags(int argc, char** argv){
        int numFlags = argc - 1;

        int renderingFlags = 0;
        for(int i = 0; i < numFlags; i++){

            char* currFlag = argv[i + 1];
            auto key = Flags::strToFlag.find(currFlag);

            if(key == Flags::strToFlag.end()) continue;
            
            Flags::RenderFlags flagSet = key->second;

            std::string name = Flags::flagToName.at(flagSet);
            std::cout << "Render Flag Set: " << name << std::endl;
            renderingFlags |= flagSet;
        }

        return renderingFlags;
    }
}


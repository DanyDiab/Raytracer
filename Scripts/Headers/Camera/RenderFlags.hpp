#pragma once
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <string>

namespace Flags{

    enum RenderFlags{
        DebugNormals = 1, // show normals rather than colors
        Progress = 1 << 1, // show the progress as it renders
        Performance = 1 << 2 // log the performance metrics
    };
    
    // map string inputs to flag
    inline std::map<std::string, RenderFlags> strToFlag = {
    {"-dn", RenderFlags::DebugNormals},
    {"-pro", RenderFlags::Progress},
    {"-perf", RenderFlags::Performance}
    };
    inline std::map<RenderFlags, std::string> flagToName = {
    {RenderFlags::DebugNormals, "Debug Normals"},
    {RenderFlags::Progress, "Show Progress"},
    {RenderFlags::Performance, "Show Performance Metrics"}
    };

    inline int ProcessFlags(int argc, char** argv){
        int numFlags = argc - 1;

        int renderingFlags = 0;
        for(int i = 0; i < numFlags; i++){

            std::string currFlag = argv[i + 1];
            
            // convert to lower
            std::transform(currFlag.begin(),currFlag.end(),currFlag.begin(),[](unsigned char c){
                return std::tolower(c);
            });

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


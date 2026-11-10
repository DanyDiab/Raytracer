#pragma once

#include <driver_types.h>
#include <string>
#include <vector>
#include <map>
namespace Time{

    struct Group{
        std::vector<cudaEvent_t> events;
        bool complete;
    };

    class GPUTimer{
        public:
            GPUTimer(bool enabled, cudaStream_t stream);
    
            void addMarker(std::string groupName);
    
            float totalGroupTime(std::string groupName);
            float avgGroupMarkerTime(std::string groupName);
    
            void printAllGroupTimes();

        private:
            void syncGroup(std::string groupName);

            std::vector<float> getGroupTimes(Group group);
            std::map<std::string, Group> markers;
            bool enabled;
            cudaStream_t stream;
    };
}

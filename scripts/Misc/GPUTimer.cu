#include "../headers/Util/GPUTimer.cuh"
#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <iostream>
#include <string>
#include <vector>




Time::GPUTimer::GPUTimer(bool enabled, cudaStream_t stream){
    this->enabled = enabled;
    this->stream = stream;
}

void Time::GPUTimer::addMarker(std::string groupName){
    if(!enabled) return;
    
    cudaEvent_t event;
    
    cudaEventCreate(&event);
    cudaEventRecord(event, stream);

    if(markers.find(groupName) != markers.end()){
        markers[groupName].events.push_back(event);
        return;
    }
    Group group{
        .events = std::vector<cudaEvent_t>(),
        .complete = false
    };

    group.events.push_back(event);
    markers[groupName] = group;
}

void Time::GPUTimer::syncGroup(std::string groupName){
    auto groupIt = markers.find(groupName);
    Time::Group group = groupIt->second;
    
    group.complete = true;
    cudaEventSynchronize(group.events.at(group.events.size() - 1));
}

float Time::GPUTimer::totalGroupTime(std::string groupName){
    auto groupIt = markers.find(groupName);

    if(groupIt == markers.end()) return -1.0f;

    Time::Group group = groupIt->second;
    if(group.events.empty()) return -1.0f;
    
    if(!group.complete){
        syncGroup(groupName);
    }

    float ms = 0.0f;

    cudaEventElapsedTime(&ms, group.events.at(0), group.events.at(group.events.size() - 1));

    return ms;
}

std::vector<float> Time::GPUTimer::getGroupTimes(Group group){

    float totalMS = 0;

    std::vector<float> times;

    for(int i = 1; i < group.events.size(); ++i){
        float delta = 0.0f;
        cudaEventElapsedTime(&delta, group.events[i - 1], group.events[i]);
        times.push_back(delta);
    }

    return times;
}


float Time::GPUTimer::avgGroupMarkerTime(std::string groupName){
    auto groupIt = markers.find(groupName);

    if(groupIt == markers.end()) return -1.0f;

    Time::Group group = groupIt->second;
    if(group.events.empty()) return -1.0f;
    
    if(!group.complete){
        syncGroup(groupName);
    }

    std::vector<float> times = getGroupTimes(group);
    float totalMS = 0;
    
    for(const auto& time : times){
        totalMS += time;
    }

    return totalMS / times.size();
}


void Time::GPUTimer::printAllGroupTimes(){
    for(const auto& group : markers){
        if(!group.second.complete){
            syncGroup(group.first);
        }

        std::string name = group.first;

        std::cout << " GROUP " << name << "\n";
        std::cout << "--------------------" "\n";

        std::vector<float> times = getGroupTimes(group.second);
        
        int index = 0;
        float totalTime = 0;
        for(const auto& time : times){
            std::cout << "time " << index << ": " << time << "\n";
            totalTime+= time;
            index++; 
        }
        std::cout << "Total Group Time: " << totalTime << "\n";


        std::cout << "\n\n";
    }
}


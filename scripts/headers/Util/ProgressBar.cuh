#pragma once

#include <glm/common.hpp>
#include <iostream>

inline void CUDART_CB ProgressCallBack(void* userData){
    float* percent = static_cast<float*>(userData);
    
    char completed = 'X';
    char unCompleted = 'O';

    const int barWidth = 50;
    int completedChars = static_cast<int>(*percent * barWidth);

    std::cout << "\rRendering Progress | [";
    for(int i = 0; i < barWidth; i++){
        if(i < completedChars){
            std::cout << completed;
        }
        else{
            std::cout << unCompleted;
        } 
    }
    std::cout << "] " << static_cast<int>(*percent * 100.0f) << "%" << std::flush;
}
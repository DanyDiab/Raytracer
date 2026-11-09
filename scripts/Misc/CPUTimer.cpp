#include "../headers/Util/CPUTimer.hpp"
#include <chrono>
#include <iostream>


Time::CPUTimer::CPUTimer(bool enabled){
    this->enabled = enabled;
    currState = TimerState::IDLE;
}

double currTime() {
    auto now = std::chrono::steady_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration<double, std::milli>(epoch).count();
}

void Time::CPUTimer::Start(){
    if(currState != TimerState::IDLE || !enabled) return;

    this->startTime = currTime();

    this->lastLapTime = startTime;
    this->lapTimes.clear();

    this->endTime = -1;

    currState = TimerState::RUNNING;
}

void Time::CPUTimer::Stop(){
    if(currState != TimerState::RUNNING || !enabled) return;

    this->endTime = currTime();
    currState = TimerState::IDLE;
}

double Time::CPUTimer::TotalTimeElapsed(){
    if(currState != TimerState::IDLE || !enabled) return -1.0f;
    return endTime - startTime;
}

double Time::CPUTimer::CurrTimeElapsed(){
    double curr = currTime();

    double delta = curr - startTime;
    return delta;
}

double Time::CPUTimer::lapTimeAndAdvance(){
    double curr = currTime();
    double elapsedTime = curr - lastLapTime;

    lastLapTime = curr;

    return elapsedTime;
}

void Time::CPUTimer::addEventToLaps(double time, std::string tag){
    
    Time::LapEvent event{
        .time = time,
        .tag = tag
    };

    lapTimes.push_back(event);
}

void Time::CPUTimer::AddLap(std::string tag){
    if(currState != TimerState::RUNNING || !enabled) return;

    double currTime = lapTimeAndAdvance();

    addEventToLaps(currTime, tag);
}

void Time::CPUTimer::AddLap(){
    if(currState != TimerState::RUNNING || !enabled) return;

    double currTime = lapTimeAndAdvance();

    std::string tag = "No Tag";

    addEventToLaps(currTime, tag);
}

double Time::CPUTimer::AverageLapTime(){
    if (lapTimes.empty()) {
        return 0.0;
    }
    double sum = 0;

    for(const auto& lapTime : lapTimes){
        sum += lapTime.time;
    }

    double avg = sum / lapTimes.size();

    return avg;
}


void Time::CPUTimer::PrintLapTimes(){
    int idx = 0;
    printf("IDX | TAG: TIME\n");
    for(const auto& event : lapTimes){
        std::cout << idx  << " | " << event.tag << ": " << event.time << "\n";
        idx++;
    }
}

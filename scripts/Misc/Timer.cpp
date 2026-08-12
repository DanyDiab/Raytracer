#include "../headers/Util/Timer.hpp"
#include <chrono>
#include <iostream>


Time::Timer::Timer(bool enabled){
    this->enabled = enabled;
    currState = TimerState::IDLE;
}

double currTime() {
    auto now = std::chrono::steady_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration<double, std::milli>(epoch).count();
}

void Time::Timer::Start(){
    if(currState != TimerState::IDLE || !enabled) return;

    this->startTime = currTime();

    this->lastLapTime = startTime;
    this->lapTimes.clear();

    this->endTime = -1;

    currState = TimerState::RUNNING;
}

void Time::Timer::Stop(){
    if(currState != TimerState::RUNNING || !enabled) return;

    this->endTime = currTime();
    currState = TimerState::IDLE;
}

double Time::Timer::TotalTimeElapsed(){
    if(currState != TimerState::IDLE || !enabled) return -1.0f;
    return endTime - startTime;
}

double Time::Timer::CurrTimeElapsed(){
    double curr = currTime();

    double delta = curr - startTime;
    return delta;
}

double Time::Timer::lapTimeAndAdvance(){
    double curr = currTime();
    double elapsedTime = curr - lastLapTime;

    lastLapTime = curr;

    return elapsedTime;
}

void Time::Timer::addEventToLaps(double time, std::string tag){
    
    Time::LapEvent event{
        .time = time,
        .tag = tag
    };

    lapTimes.push_back(event);
}

void Time::Timer::AddLap(std::string tag){
    if(currState != TimerState::RUNNING || !enabled) return;

    double currTime = lapTimeAndAdvance();

    addEventToLaps(currTime, tag);
}

void Time::Timer::AddLap(){
    if(currState != TimerState::RUNNING || !enabled) return;

    double currTime = lapTimeAndAdvance();

    std::string tag = "No Tag";

    addEventToLaps(currTime, tag);
}

double Time::Timer::AverageLapTime(){
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


void Time::Timer::PrintLapTimes(){
    int idx = 0;
    printf("IDX | TAG: TIME\n");
    for(const auto& event : lapTimes){
        std::cout << idx  << " | " << event.tag << ": " << event.time << "\n";
        idx++;
    }
}












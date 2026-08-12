#include <string>
#include <vector>
namespace Time{

    enum TimerState{
        IDLE,
        RUNNING
    };

    struct LapEvent {
        double time;
        std::string tag;
    };
    
    class Timer{
        public:
            Timer(bool enabled);
        // basic tools
            void Start();
            void AddLap(std::string name);
            void AddLap();
            void Stop();
            
        // metrics 
            double TotalTimeElapsed();
            double CurrTimeElapsed();
            double AverageLapTime();

            void PrintLapTimes();
            
        private:
        // list of times
            std::vector<LapEvent> lapTimes;

            double startTime;
            double endTime;
            double lastLapTime;

            bool enabled;

            TimerState currState;
            double lapTimeAndAdvance();
            void addEventToLaps(double time, std::string tag);
    };
}
#include <Arduino.h>
#include <SPIFFS.h>
#include <functional>
#include <optional>
#include <algorithm>

class MethodCoulombCounting
{
    float capacity_;
    float capacity_max_;
    uint64_t update_prev_;

public:
    MethodCoulombCounting(float capacity_max) : capacity_(capacity_max), capacity_max_(capacity_max)
    {
    }

    void init()
    {
        update_prev_ = micros();
    }

    void update(float amp)
    {
        uint64_t tick = micros();
        capacity_ += amp * 1000 * (tick - update_prev_);
        std::clamp(capacity_, 0.0f, capacity_max_);
        update_prev_ = tick;
    }

    float GetSoc()
    {
        return std::clamp(capacity_ / capacity_max_ * 100.0f, 0.0f, 100.0f);
    }

    float GetCapacity()
    {
        return capacity_;
    }
};
struct BatteryState
{
    const std::string path = "/batterystate.txt";
    float soc;
    float soh;

    BatteryState()
    {
    }

    bool init()
    {
        begin_ = SPIFFS.begin(true);
        return begin_;
    }

    bool write()
    {
        if (!begin_)
            return false;

        File fp = SPIFFS.open(path.c_str(), "w");

        if (!fp)
            return false;

        fp.println(std::to_string(soc).c_str());
        fp.println(std::to_string(soh).c_str());
        fp.close();

        return true;
    }

    bool read()
    {
        if(!begin_)
            return false;

        File fp = SPIFFS.open(path.c_str(), "r");

        if(!fp)
            return false;

        soc = fp.readStringUntil('\n').toFloat();
        soh = fp.readStringUntil('\n').toFloat();
        fp.close();
        
        return true;
    }

private:
    bool begin_ = false;
};

class Estimator
{
    MethodCoulombCounting method_;
    BatteryState state_;

    float estimate_soc(float amp)
    {
        method_.update(amp);
        return method_.GetSoc();
    }

    float estimate_soh()
    {
    }

public:
    Estimator() : method_(6000){};

    void init()
    {
        method_.init();
        state_.init();
        state_.read();
    }

    void update(float current, float whatt)
    {
        bool write_strorage = false;
        float data = estimate_soc(current);
        if(std::abs(state_.soc - data) > 1)  write_strorage = true;
        state_.soc = data;
        data = estimate_soh();
        if(std::abs(state_.soh - data) > 1)  write_strorage = true;
        if(write_strorage)  state_.write();
    }

    float GetSoc()
    {
        return state_.soc;
    }

    float GetSoh()
    {
        return state_.soh;
    }
};
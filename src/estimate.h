#include <Arduino.h>
#include <functional>

class Estimator
{

    float soc_;
    float soh_;

    float estimate_soc() {

    }

    float estimate_soh() {

    }

public:
    Estimator(){};

    void update(float current, float whatt)
    {
        soc_ = estimate_soc();
        soh_ = estimate_soh();
    }

    float GetSoc()
    {
        return soc_;
    }

    float GetSoh()
    {
        return soh_;
    }
};
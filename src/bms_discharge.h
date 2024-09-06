#pragma once

#include "ltc6811_driver.h"

namespace ltc6811::discharge
{
    struct Method_Min
    {
        void discharge()
        {
        }
    };

    struct Method_Mean
    {
        void discharge()
        {
        }
    };
    template <class C>
    void loop()
    {
        C{}.discharge();
    }
};
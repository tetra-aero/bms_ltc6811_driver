#include <map>
#include <array>
#include <variant>
#include <optional>
#include <CAN.h>
#include "constant.h"

class BMSMessage
{
    static constexpr uint32_t CAN_REQUEST_VOLTAGE = 0x4600;
    static constexpr uint32_t CAN_REQUEST_CELLVOLTAGE = 0x4700;
    static constexpr uint32_t CAN_REQUEST_CELLBALANCE = 0x4800;
    static constexpr uint32_t CAN_REQUEST_TEMPERATURE = 0x4900;
    std::map<uint32_t, std::variant<CELL_DATA &, TEMP_DATA &, VOL_DATA &>> data;

public:
    BMSMessage(CELL_DATA &cell, TEMP_DATA &temp, VOL_DATA &vol)
    {
        data[CAN_REQUEST_VOLTAGE] = vol;
        data[CAN_REQUEST_TEMPERATURE] = temp;
        data[CAN_REQUEST_CELLVOLTAGE] = cell;
        data[CAN_REQUEST_CELLBALANCE] = cell;
    }
    std::optional<std::variant<CELL_DATA &, TEMP_DATA &, VOL_DATA &>> get_data(uint32_t id)
    {
        auto it = data.find(id);
        if (it == data.end())
        {
            return std::nullopt;
        }
        return it->second;
    }
};

class CANDriver
{
    BMSMessage message_;
    CANDriver(BMSMessage &&message) : message_(message)
    {
    }

    bool setup(int bitrate)
    {
        return CAN.begin(bitrate);
    }

    void update()
    {
        int packetSize = CAN.parsePacket();

        if (packetSize == 0)
            return;

        auto data = message_.get_data(CAN.packetId());
        if (!data.has_value())
            return;
    }
};
#pragma once
#include "Arduino.h"
#include "constant.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace ltc6811::data
{
    using ic_id = uint8_t;
    using cell_id = uint8_t;
    struct CellVoltage
    {
        uint32_t sum{0};
        CELL_DATA vol;
        std::pair<uint16_t, uint16_t> vol_range{std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min()};
        std::pair<ic_id, cell_id> min_id{0xFF, 0xFF};
        std::pair<ic_id, cell_id> max_id{0xFF, 0xFF};
    };
    struct Temperature
    {
        TEMP_DATA temp;
        std::array<int32_t, CHANE_LENGTH> vref2;
        std::pair<uint16_t, uint16_t> temp_range{std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::min()};
        std::pair<ic_id, cell_id> min_id{0xFF, 0xFF};
        std::pair<ic_id, cell_id> max_id{0xFF, 0xFF};
    };
};

namespace ltc6811::params
{

};

namespace ltc6811::utils
{
    constexpr std::array<uint16_t, 256> table{
        0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa,
        0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d,
        0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4,
        0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x02be, 0xc727, 0xcc15, 0x098c, 0xda71, 0x1fe8, 0x14da, 0xd143,
        0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0, 0xdf0d, 0x1a94, 0x11a6, 0xd43f,
        0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8,
        0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11,
        0x057c, 0xc0e5, 0xcbd7, 0x0e4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286,
        0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9,
        0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e,
        0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0, 0x0d3a, 0xc8a3, 0xc391, 0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7,
        0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350,
        0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c,
        0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374, 0xd089, 0x1510, 0x1e22, 0xdbbb,
        0x0af8, 0xcf61, 0xc453, 0x01ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
        0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095};

    constexpr uint16_t CRC(const std::array<uint8_t, 6> &arr, size_t size)
    {
        uint16_t res = 0x10;
        uint16_t idx = 0x00;
        const uint8_t *data = arr.data();

        for (size_t i = 0; i < size; ++i)
        {
            idx = (res >> 7 ^ data[i]) & 0xFF;
            res = res << 8 ^ table[idx];
        }
        res = res << 1;
        return res;
    }

    constexpr uint16_t CRC(const std::array<uint16_t, 3> &arr, size_t size)
    {
        uint16_t res = 0x10;
        uint16_t idx = 0x00;
        const uint8_t *data = reinterpret_cast<const uint8_t *>(arr.data());

        for (size_t i = 0; i < size; ++i)
        {
            idx = (res >> 7 ^ data[i]) & 0xFF;
            res = res << 8 ^ table[idx];
        }
        res = res << 1;
        return res;
    }
};
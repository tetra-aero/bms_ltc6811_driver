#include <array>

#include <Arduino.h>

static constexpr uint32_t CHANE_LENGTH = 2;
static constexpr uint32_t CELL_NUM_PER_IC = 12;
static constexpr uint32_t THURMISTA_NUM_PER_IC = 5;

using CELL_DATA = std::array<std::array<uint16_t, CELL_NUM_PER_IC>, CHANE_LENGTH>;
using TEMP_DATA = std::array<std::array<int32_t, THURMISTA_NUM_PER_IC>, CHANE_LENGTH>;
using VOL_DATA = float;

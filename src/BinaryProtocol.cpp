#include "BinaryProtocol.h"

namespace core {
namespace protocol {
namespace crc32 {

// Standard CRC32 polynomial
constexpr uint32_t POLY = 0xEDB88320;
static uint32_t crc_table[256];
static bool crc_table_initialized = false;

void initialize_crc_table() {
    if (crc_table_initialized) return;
    
    for (int i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ POLY;
            } else {
                crc >>= 1;
            }
        }
        crc_table[i] = crc;
    }
    
    crc_table_initialized = true;
}

uint32_t calculate(const uint8_t* data, size_t length) noexcept {
    initialize_crc_table();
    
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc_table[index];
    }
    
    return crc ^ 0xFFFFFFFF;
}

} // namespace crc32
} // namespace protocol
} // namespace core

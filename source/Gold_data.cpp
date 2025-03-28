#include "Gold_data.hpp"
#include <cstdio>

Lrand<uint8_t> Gold_data::rand_data;
Lrand<uint8_t> Gold_data::Chunk::rand_data;

std::string Gold_data::str() const {
    std::string msg;
    char buffer[1024];

    for (const auto c : chunks) {
        if (device) {
            msg.append(" DEVICE");
        }
        snprintf(buffer, sizeof(buffer), " mem[%lx:%lx]=", c.addr, c.addr + c.data.size() - 1);
        msg.append(buffer);

        for (auto b : c.data) {
            snprintf(buffer, sizeof(buffer), "%02x", b);
            msg.append(buffer);
        }
    }

    return msg;
}

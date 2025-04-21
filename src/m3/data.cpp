/*
 * Copyright (c) 2025 Micro Architecture Santa Cruz
 * and Texer.ai. All rights reserved.
 */

#include "m3/data.hpp"
#include <cstdio>

Lrand<uint8_t> Data::rand_data;
Lrand<uint8_t> Data::Chunk::rand_data;

std::string Data::str() const {
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

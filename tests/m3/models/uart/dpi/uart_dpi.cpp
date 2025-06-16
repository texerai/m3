#include <memory>
#include <svdpi.h>
#include <stdio.h>
#include <string.h>

#include "model_instances.h"
#include "uart.h"

extern "C" void uart_tick(unsigned char out_valid, unsigned char *out_ready, char out_bits, unsigned char *in_valid,
    unsigned char in_ready, char *in_bits)
{
    marionette::ModelInstances& model_instances = marionette::ModelInstances::get();
    std::shared_ptr<uart_t> uart = model_instances.GetUartPtr();

    uart->tick(out_valid, out_ready, out_bits, in_valid, in_ready, in_bits);
}

// Copyright (c) 2023 Kabylkas Labs. All rights reserved.
#include "model_instances.h"

// Model libraries.
#include "boom_m3_t.h"
#include "dromajo_t.h"
#include "mm.h"
#include "mm_dramsim2.h"
#include "uart.h"

// Local libraries.
#include "tracer.h"

// C++ libraries.
#include <cassert>
#include <cstring>
#include <memory>
#include <string>

namespace marionette
{
    // Constants.
    const uint32_t kTracerBufferSize = 1024;

    // Implementation sturct.
    struct ModelInstances::ModelInstancesImpl
    {
        std::shared_ptr<uart_t> uart_ptr;
        std::shared_ptr<mm_t> dram_ptr;
        std::shared_ptr<dromajo_t> dromajo_ptr;
        std::shared_ptr<m3::boom_m3_t> m3_ptr;
        bool should_abort = false;
        std::string error_message;

        void InitUart()
        {
            uart_ptr = std::make_shared<uart_t>(nullptr, 0, false);
        }

        void InitDram(const Config& config)
        {
            dram_ptr = std::make_shared<mm_dramsim2_t>(config.ini_dir_path, 1 << config.id_bits, config.clock_hz);
            dram_ptr->init(config.mem_size, config.word_size, config.line_size);
            if (!config.image_path.empty())
            {
                dram_ptr->load_image(config.image_path.c_str());
            }
        }

        void InitDromajo(const Config& config)
        {
            // Form dromajo arguments.
            std::vector<std::string> dromajo_args;
            dromajo_args.push_back("./dromajo");
            dromajo_args.push_back("--trace");
            dromajo_args.push_back("0");
            dromajo_args.push_back(config.dromajo_config_path);

            dromajo_ptr = std::make_shared<dromajo_t>();
            dromajo_ptr->init(dromajo_args);
        }

        void InitBoomM3()
        {
            m3_ptr = std::make_shared<m3::boom_m3_t>();
            m3_ptr->init(2, dromajo_ptr);
        }
    };

    ModelInstances& ModelInstances::get()
    {
        static ModelInstances model_instances;
        return model_instances;
    }

    bool ModelInstances::InitializeModels(const Config& config)
    {
        bool ret = false;
        if (pimpl_ == nullptr)
        {
            pimpl_ = new ModelInstances::ModelInstancesImpl();
            pimpl_->InitUart();
            pimpl_->InitDram(config);
            pimpl_->InitDromajo(config);
            pimpl_->InitBoomM3();
        }
        return ret;
    }

    void ModelInstances::PerformEndOfCycleActions()
    {
        bool is_good = pimpl_->m3_ptr->serve_registered_events();
        if (!is_good)
        {
            pimpl_->should_abort = true;
        }
    }

    void ModelInstances::CloseModels()
    {
        pimpl_->m3_ptr->close();
    }

    bool ModelInstances::IsAborted(std::string& error_message)
    {
        error_message = pimpl_->error_message;
        return pimpl_->should_abort;
    }

    void ModelInstances::Abort(const std::string& error_message)
    {
        pimpl_->error_message = error_message;
        pimpl_->should_abort = true;
    }

    std::shared_ptr<uart_t> ModelInstances::GetUartPtr()
    {
        std::shared_ptr<uart_t> ret;
        if (pimpl_ != nullptr)
        {
            ret = pimpl_->uart_ptr;
        }
        return ret;
    }

    std::shared_ptr<mm_t> ModelInstances::GetDramPtr()
    {
        std::shared_ptr<mm_t> ret;
        if (pimpl_ != nullptr)
        {
            ret = pimpl_->dram_ptr;
        }
        return ret;
    }

    std::shared_ptr<dromajo_t> ModelInstances::GetDromajoPtr()
    {
        std::shared_ptr<dromajo_t> ret;
        if (pimpl_ != nullptr)
        {
            ret = pimpl_->dromajo_ptr;
        }
        return ret;
    }

    std::shared_ptr<m3::boom_m3_t> ModelInstances::GetM3Ptr()
    {
        std::shared_ptr<m3::boom_m3_t> ret;
        if (pimpl_ != nullptr)
        {
            ret = pimpl_->m3_ptr;
        }
        return ret;
    }

    bool ModelInstances::IsInit()
    {
        return pimpl_ != nullptr;
    }
};

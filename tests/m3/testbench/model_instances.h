// Copyright (c) 2023 Kabylkas Labs. All rights reserved.
#ifndef MODEL_INSTANCES_H_
#define MODEL_INSTANCES_H_

// C++ libraries.
#include <memory>

// Forward declarations.
class uart_t;
class mm_t;
class dromajo_t;
namespace m3 {
    class boom_m3_t;
}

namespace marionette
{
    struct Config
    {
        // DRAM configs.
        std::string ini_dir_path;
        std::string image_path;
        uint64_t mem_size = 0;
        uint64_t word_size = 0;
        uint64_t line_size = 0;
        uint64_t id_bits = 0;
        uint64_t clock_hz = 0;

        // Dromajo configs.
        std::string dromajo_config_path;
    };

    class ModelInstances
    {
    public:
        // Get a single globally visiable model instances.
        static ModelInstances& get();

        // Pointers to models.
        std::shared_ptr<uart_t> GetUartPtr();
        std::shared_ptr<mm_t> GetDramPtr();
        std::shared_ptr<dromajo_t> GetDromajoPtr();
        std::shared_ptr<m3::boom_m3_t> GetM3Ptr();

        // Initialize/close the models.
        bool InitializeModels(const Config& config);
        void PerformEndOfCycleActions();
        void CloseModels();
        bool IsInit();
        bool IsAborted(std::string& error_message);
        void Abort(const std::string& error_message);

        // Delete all constractors.
        ModelInstances(const ModelInstances&) = delete;
        ModelInstances& operator=(const ModelInstances&) = delete;

    private:
        ModelInstances() = default;
        struct ModelInstancesImpl;
        ModelInstancesImpl* pimpl_ = nullptr;
    };
}

#endif // MODEL_INSTANCES_H_

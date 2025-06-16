/*
 * Copyright (c) 2022 MicroArchitecture Santa Cruz & Kabylkas Labs. All rights reserved.
 */
#ifndef BRANCH_PREDICTOR_H_
#define BRANCH_PERDICTOR_H_

// C++ libraries.
#include <vector>
#include <stdint.h>
#include <string>

#define MARIONETTE_TRACE

namespace marionette {
  enum class BranchDirection {
    kUnknown,
    kTaken,
    kNotTaken
  };

  class BranchPredictor {
  public:
    void Initialize(const std::string& marionette_id, size_t table_size, size_t saturation_size, size_t ghr_size, bool use_ghr);
    BranchDirection GetBranchPrediction(uint64_t pc);
    void UpdateBHT(uint64_t pc, BranchDirection resolution);
    void FlushTables();
    void RecordMispredict();
    BranchPredictor() = default;

  private:
    // Global History Register.
    uint32_t ghr_ = 0;
    size_t ghr_size_ = 0;
    bool use_ghr_ = false;

    // Branch History Table.
    std::vector<uint16_t> bht_;
    size_t saturation_size_ = 0;
    size_t table_size_ = 0;

    // Others.
    std::string marionette_id_;
    bool is_init_ = false;
    uint64_t marionette_mispred_count_ = 0;
    uint64_t actual_mispred_count_ = 0;
    std::vector<BranchDirection> last_prediction_;
  };
}
#endif // !BRANCH_PREDICTOR_H_

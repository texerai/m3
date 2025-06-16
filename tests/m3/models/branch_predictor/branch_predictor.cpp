/*
 * Copyright (c) 2022 MicroArchitecture Santa Cruz & Kabylkas Labs. All rights reserved.
 */
#include "branch_predictor.h"

// C++ libraries.
#include <iostream>
#include <map>

namespace marionette {
  // Constants.
  static const uint32_t kLsbOffset = 1;
  static const std::map<BranchDirection, std::string> kBranchDirectionString = {
    { BranchDirection::kNotTaken, "NT" },
    { BranchDirection::kTaken, "T" },
    { BranchDirection::kUnknown, "Unknown" }
  };


  // Internally linked static aux functions.
  static uint32_t CalculateIndex(uint64_t pc, uint32_t table_size, uint32_t mixer, bool use_mixer) {
    // Remove LSB due to aligned instructions.
    uint32_t shifted_pc = pc >> kLsbOffset;

    // Fit the size.
    uint32_t index = shifted_pc % table_size;

    // Mix in information, such as global history.
    if (use_mixer) {
      index = index ^ mixer;
    }

    return index;
  }

  void BranchPredictor::Initialize(const std::string& marionette_id, size_t table_size, size_t saturation_size, size_t ghr_size, bool use_ghr) {
    marionette_id_ = marionette_id;
    table_size_ = table_size;
    saturation_size_ = saturation_size;
    ghr_size_ = ghr_size;
    use_ghr_ = use_ghr;

    for (uint32_t i = 0; i < table_size_; i++) {
      bht_.push_back(0);
      last_prediction_.push_back(BranchDirection::kNotTaken);
    }
    is_init_ = true;

    #ifdef MARIONETTE_TRACE
    std::cout << "MARIONETTE_TRACE: ";
    std::cout << marionette_id_ << ":";
    std::cout << "Initialize(): ";
    std::cout << "Table entries = " << table_size_ << "; ";
    std::cout << "Counter size = " << saturation_size_ << "; ";
    std::cout << "Using GHR = " << use_ghr_ << ";" << std::endl;
    #endif
  }

  BranchDirection BranchPredictor::GetBranchPrediction(uint64_t pc) {
    // Calculate the index.
    uint32_t index = CalculateIndex(pc, table_size_, ghr_, use_ghr_);
    uint32_t half_saturated = (1 << saturation_size_) / 2;

    BranchDirection predicted_direction = BranchDirection::kNotTaken;
    bool is_taken = bht_[index] >= half_saturated;
    if (is_taken) {
      predicted_direction = BranchDirection::kTaken;
    }

    // Save this prediction.
    last_prediction_[index] = predicted_direction;

    #ifdef MARIONETTE_TRACE
    std::cout << "MARIONETTE_TRACE: ";
    std::cout << marionette_id_ << ":";
    std::cout << "GetPrediction(): ";
    std::cout << std::hex << pc << "; ";
    std::cout << "Prediction = " << kBranchDirectionString.at(predicted_direction) << "; " << std::endl;
    #endif

    return predicted_direction;
  }

  void BranchPredictor::UpdateBHT(uint64_t pc, BranchDirection resolution) {
    // Calculate the index.
    uint32_t index = CalculateIndex(pc, table_size_, ghr_, use_ghr_);

    // Get the current count.
    int32_t counter = bht_[index];

    // Decrease if not taken, increase otherwise.
    if (resolution == BranchDirection::kNotTaken) {
      counter--;
    }
    else if (resolution == BranchDirection::kTaken) {
      counter++;
    }

    // Check the saturation limits.
    if (counter < 0) {
      bht_[index] = 0;
    }
    else if (counter == (1 << saturation_size_)) {
      bht_[index] = (1 << saturation_size_) - 1;
    }
    else {
      bht_[index] = counter;
    }

    // Record mispredictions.
    if (resolution != last_prediction_[index]) {
      marionette_mispred_count_++;
    }

    // Record branch global history.
    uint32_t new_ghr = static_cast<uint32_t>(ghr_ << 1) | static_cast<uint32_t>(resolution == BranchDirection::kTaken);
    ghr_ = new_ghr & ((1 << ghr_size_) - 1);

    #ifdef MARIONETTE_TRACE
    std::cout << "MARIONETTE_TRACE: ";
    std::cout << marionette_id_ << ":";
    std::cout << "UpdateBHT(): ";
    std::cout << "Branch resolution = " << kBranchDirectionString.at(resolution) << "; ";
    std::cout << "Branch PC = " << std::hex << pc << "; ";
    std::cout << "Index = " << std::dec << index << "; ";
    std::cout << "Misprediction count = (" << marionette_mispred_count_ << ", " << actual_mispred_count_ << "); ";
    std::cout << "Is mispredicted = " << (resolution != last_prediction_[index]) << ";" << std::endl;
    #endif
  }

  void BranchPredictor::FlushTables() {
    for (uint32_t i = 0; i < table_size_; i++) {
      bht_[i] = 0;
    }

    #ifdef MARIONETTE_TRACE
    std::cout << "MARIONETTE_TRACE: ";
    std::cout << marionette_id_ << ":";
    std::cout << "FlushTables(): " << std::endl;
    #endif
  }

  void BranchPredictor::RecordMispredict() {
    actual_mispred_count_++;
  }
}

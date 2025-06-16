# Copyright (c) 2022 MicroArchitecture Santa Cruz & Kabylkas Labs
cmake_minimum_required(VERSION 3.0)

set (ALL_H
	./branch_predictor.h
)

set (ALL_CPP
	./branch_predictor.cpp
)

add_library(branch_predictor_marionette STATIC ${ALL_H} ${ALL_CPP})

#!/bin/bash

# this is specific to my i7-12700H - use efficienty cores only,
# no hyperthreading, no turbo mode

taskset -c 0,2,4,6,8,10 ./solver_benchmark

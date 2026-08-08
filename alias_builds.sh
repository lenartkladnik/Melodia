#!/bin/bash

function _build_template() {
  scons target=$2 build=$3 version=$4 main=$5
  if [ $? -eq 0 ]; then
    if [ "$1" != "-n" ] && [ "$1" != "--no-execute" ]; then
      cd dist/debug/linux/$4
      if [ ! -z "$1" ]; then
        $1 ./Melodia
      else
        ./Melodia
      fi
      cd ../../../..
    fi
  fi
}

function build_debug_l0() {
  _build_template "$1" "linux" "debug-l0" "dev-build" "main.cpp"
}

function build_debug_l1() {
  _build_template "$1" "linux" "debug-l1" "dev-build" "main.cpp"
}

function build_debug() {
  build_debug_l1
}

alias build_all="scons"

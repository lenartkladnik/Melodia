#!/bin/bash

function _build_template() {
  scons target=$2 build=$3 version=$4 main=$5
  if [ $? -eq 0 ]; then
    if [ "$1" != "-n" ] && [ "$1" != "--no-execute" ]; then
      cd dist/debug/linux/$4
      if [ ! -z "$1" ]; then
        $@ ./Melodia
        cd ../../../..
      else
        ./Melodia
        cd ../../../..
      fi
    fi
  fi
}

function build_debug() {
  _build_template "$1" "linux" "debug" "dev-build" "main.cpp"
}

function build_download_dummy() {
  _build_template "--no-execute" "linux" "debug" "download-dummy-build" "download_dummy.cpp"
}

alias build_all="scons"

#!/bin/bash

echo "==== Starting build."

pushd .build;

/usr/bin/time -v clang++ -Wall \
                         -g \
                         -O2 \
                         -fno-exceptions \
                         --std=c++17 \
                         -I"../../sysroot/include/" \
                         -I"./code/public/include/profitDrain/" \
                         -I"./code/private/include/profitDrain/" \
                         -L"../../sysroot/lib/" \
                         -o "profitDrain" \
                         "../code/src/resistance_is_futile.cpp";
build_result=$?;

cp "./profitDrain" "../../sysroot/bin/";

popd;

echo "==== Build finished with code $build_result"

#!/usr/bin/env bash

set -euo pipefail

ndk-build -C module clean || exit 1
ndk-build -j$(nproc) -C module V=1 || exit 1

rm -rf magisk/zygisk
mkdir -p magisk/zygisk

for arch in arm64-v8a armeabi-v7a x86 x86_64
do
    cp module/libs/$arch/*.so magisk/zygisk/$arch.so
done

cp module/jni/target.yml magisk
cp module/jni/template.yml magisk

pushd magisk

version=`grep '^version=' module.prop | cut -d= -f2`
name=`grep '^id=' module.prop | cut -d= -f2`

rm -rf ../build
mkdir -p ../build

zip -r9 ../build/device-spoof-zygisk-$version.zip .
popd
#!/bin/sh
set -e

. ./meta-conf.sh

if [ ! -f llvm-project-$LLVM_VER.txz ]; then
	wget -O - https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VER/llvm-project-$LLVM_VER.src.tar.xz | tee llvm-project-$LLVM_VER.txz | tar -xJ
	cd llvm-project-$LLVM_VER.src
	patch -p1 <../llvm-$LLVM_VER.diff
	cd ..
elif [ ! -d llvm-project-$LLVM_VER.src ]; then
	tar -xJf llvm-project-$LLVM_VER.txz
	cd llvm-project-$LLVM_VER.src
	patch -p1 <../llvm-$LLVM_VER.diff
	cd ..
fi

mkdir -p build-llvm-$LLVM_VER
cd build-llvm-$LLVM_VER

cmake -DLLVM_ENABLE_PROJECTS='clang;lld' \
      -DLLVM_DEFAULT_TARGET_TRIPLE=$TARGET \
      -DLLVM_INSTALL_BINUTILS_SYMLINKS=Yes \
      -DLLVM_ENABLE_RUNTIMES= \
      -DCMAKE_INSTALL_PREFIX="$LLVM_SYSROOT" \
      -DCMAKE_BUILD_TYPE=Release -G Ninja \
      ../llvm-project-$LLVM_VER.src/llvm
ninja
ninja install

cd ..

ninja crt sysroot/lib/$ARCH/libc.a

mkdir -p build-compiler-rt-$LLVM_VER "$LLVM_SYSROOT"/include "$LLVM_SYSROOT"/lib/i686
cd build-compiler-rt-$LLVM_VER
cp -r "$SYSROOT"/include/* "$LLVM_SYSROOT"/include

cmake -DCOMPILER_RT_BUILD_BUILTINS=On \
      -DCOMPILER_RT_BUILD_LIBFUZZER=Off \
      -DCOMPILER_RT_BUILD_MEMPROF=Off \
      -DCOMPILER_RT_BUILD_PROFILE=Off \
      -DCOMPILER_RT_BUILD_SANITIZERS=Off \
      -DCOMPILER_RT_BUILD_XRAY=Off \
      -DCMAKE_AR="$LLVM_SYSROOT"/bin/llvm-ar \
      -DCMAKE_ASM_COMPILER_TARGET=$TARGET \
      -DCMAKE_C_COMPILER="$LLVM_SYSROOT"/bin/clang \
      -DCMAKE_C_COMPILER_TARGET=$TARGET \
      -DCMAKE_CXX_COMPILER_WORKS=True \
      -DCMAKE_EXE_LINKER_FLAGS="-L '$SYSROOT/lib/$ARCH/'" \
      -DCMAKE_NM="$LLVM_SYSROOT"/bin/llvm-nm \
      -DCMAKE_RANLIB="$LLVM_SYSROOT"/bin/llvm-ranlib \
      -DCOMPILER_RT_DEFAULT_TARGET_ONLY=On \
      -DCMAKE_SYSROOT="$SYSROOT" \
      -DCMAKE_INSTALL_PREFIX="$SYSROOT" \
      -DCMAKE_BUILD_TYPE=Release -G Ninja \
      ../llvm-project-$LLVM_VER.src/compiler-rt
ninja
ninja install
# XXX HACK
mv "$SYSROOT"/lib/linux/* "$SYSROOT"/lib/$ARCH
rmdir "$SYSROOT/lib/linux"


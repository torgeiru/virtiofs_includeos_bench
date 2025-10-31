{
  # Will create a temp one if none is passed
  buildpath ? "",

  # The unikernel to build
  unikernel ? "",

  # Enable ccache support. See overlay.nix for details.
  withCcache ? false,

  # Enable multicore suport.
  smp ? false,

  # IncludeOS development path
  includeos ? import (builtins.fetchGit {
    url = "https://github.com/torgeiru/IncludeOS";
    ref = "virtio_devices";
  }) {},
}:
let
  stdenv = includeos.stdenv;
  pkgs = includeos.pkgs;
  pkgsStatic = includeos.pkgsStatic;
in
pkgs.mkShell.override { inherit (includeos) stdenv; } rec {
  packages = [
    (pkgs.python3.withPackages (python-pkgs: [
      python-pkgs.pandas
      python-pkgs.matplotlib
      python-pkgs.seaborn
    ]))
    includeos.vmrunner
    stdenv.cc
    pkgs.buildPackages.cmake
    pkgs.buildPackages.nasm
    pkgs.qemu
    pkgs.virtiofsd
    pkgs.which
    pkgs.grub2
    pkgs.iputils
  ];

  buildInputs = [
    includeos
    includeos.chainloader
  ];

  shellHook = ''
    unikernel=$(realpath ${unikernel})
    echo -e "Attempting to build unikernel: \n$unikernel"
    if [ ! -d "$unikernel" ]; then
        echo "$unikernel is not a valid directory"
        exit 1
    fi

    export BUILDPATH=${buildpath}
    if [ -z "${buildpath}" ]; then
        export BUILDPATH="$(mktemp -d)"
        pushd "$BUILDPATH"
    else
        mkdir -p "$BUILDPATH"
        pushd "$BUILDPATH"
    fi

    cmake "$unikernel" -DARCH=x86_64 -DINCLUDEOS_PACKAGE=${includeos} -DCMAKE_MODULE_PATH=${includeos}/cmake \
                     -DFOR_PRODUCTION=OFF
    make -j $NIX_BUILD_CORES
  '';
}

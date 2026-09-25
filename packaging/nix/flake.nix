{
  description = "Execore Language Engine - Frontier C++2026 Hermetic Environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvmPackages = pkgs.llvmPackages_19;
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            llvmPackages.clang
            llvmPackages.bintools
            llvmPackages.lld
            llvmPackages.libllvm
            pkgs.gcc14
            pkgs.cmake
            pkgs.ninja
            pkgs.mold
            pkgs.ccache
            pkgs.python3
            pkgs.pre-commit
            pkgs.clang-tools
            pkgs.git
          ];

          shellHook = ''
            export CC=clang
            export CXX=clang++
            export CMAKE_GENERATOR=Ninja
            echo "🚀 Execore Frontier C++2026 Nix Shell Activated."
          '';
        };

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "execore";
          version = "2.0.0";
          src = ./../..;

          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
            llvmPackages.clang
            llvmPackages.lld
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DEXECORE_ENABLE_HARDENING=ON"
          ];
        };
      }
    );
}

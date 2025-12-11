{
  # https://www.reddit.com/r/NixOS/comments/1pfy4zz/modern_c23_development_shell_with_clang_modules/

  description = "A Nix-flake-based Udav development environment";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      toolchain = pkgs.llvmPackages_21;
    in
    {
      devShells.${system}.default =
        pkgs.mkShell.override
          {
            stdenv = toolchain.stdenv;
          }
          {
            packages = with pkgs; [
              # Compilation and LSP
              toolchain.clang-tools

              # Building C++
              cmake
              ninja
              conan

              # Lit and FileCheck
              toolchain.libllvm
              lit
            ];
            shellHook = ''
              GCC_VERSION="${pkgs.gcc.version}"
              GCC_INCLUDE="${pkgs.gcc-unwrapped}/include/c++/$GCC_VERSION"
              GCC_ARCH_INCLUDE="${pkgs.gcc-unwrapped}/include/c++/$GCC_VERSION/x86_64-unknown-linux-gnu"

              export CPLUS_INCLUDE_PATH="$GCC_INCLUDE:$GCC_ARCH_INCLUDE:$CPLUS_INCLUDE_PATH"
            '';
          };
    };
}

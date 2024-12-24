{
  description = "Devenv for Qt+CPP (linux)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qttools
            qtcreator
            kdePackages.qtbase
            kdePackages.qmake
            cmake
            gcc
            gnumake
          ];

          shellHook = ''
            echo "Qt version: $(${pkgs.qt6.qtbase}/bin/qmake --version)"
          '';
        };
      }
    );
}

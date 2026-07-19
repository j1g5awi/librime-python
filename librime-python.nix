{ lib, stdenvNoCC, python3, python3Packages }:
let
  pybind11 = python3Packages.pybind11;
in

stdenvNoCC.mkDerivation {
  pname = "librime-python";
  version = "0.0.2";

  src = ./.;  # 配合 flake 或直接 nix-build -E ...

  propagatedBuildInputs = [ python3 pybind11 ];

  installPhase = ''
    runHook preInstall
    mkdir $out
    cp --archive --verbose src/ $out
    install --mode=644 --verbose --target-directory=$out CMakeLists.txt  LICENSE  README.md
    runHook postInstall
  '';

  meta = with lib; {
    description = "Python Plug-in for the Rime Input Method Engine";
    homepage = "https://github.com/j1g5awi/librime-python";
    license = licenses.cc0;
    maintainers = [ ];
    platforms = platforms.linux;
  };
}

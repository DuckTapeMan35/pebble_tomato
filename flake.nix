{
  description = "A new Pebble app";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    pebble.url = "github:pebble-dev/pebble.nix";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      nixpkgs,
      pebble,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShell = pebble.pebbleEnv.${system} {
          packages = with pkgs; [
            bear
            just
          ];
        };
      }
    );
}

#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
controller_dir="$(dirname "$script_dir")"
app_name="SpotiLED Controller.app"
app_dir="$controller_dir/target/release/$app_name"

cargo build --manifest-path "$controller_dir/Cargo.toml" --release

rm -rf "$app_dir"
mkdir -p "$app_dir/Contents/MacOS"
cp "$controller_dir/target/release/controller" "$app_dir/Contents/MacOS/controller"
cp "$script_dir/Info.plist" "$app_dir/Contents/Info.plist"

codesign --force --deep --sign - "$app_dir"

echo "Built $app_dir"

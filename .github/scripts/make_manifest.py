"""Writes the ESP Web Tools manifest for the firmwares in a folder: python make_manifest.py <dir> <version>."""

import json
import pathlib
import sys

CHIPS = {"esp32": "ESP32", "esp32c3": "ESP32-C3", "esp32s3": "ESP32-S3"}

folder, version = pathlib.Path(sys.argv[1]), sys.argv[2]
builds = []
for chip, family in CHIPS.items():
    factory = f"xiaomi-ble-wake-{chip}.factory.bin"
    if not (folder / factory).exists():
        sys.exit(f"missing {factory}")
    builds.append({"chipFamily": family, "parts": [{"path": factory, "offset": 0}]})

manifest = {
    "name": "Xiaomi BLE wake",
    "version": version,
    "home_assistant_domain": "esphome",
    "new_install_prompt_erase": True,
    "builds": builds,
}
(folder / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
print(json.dumps(manifest, indent=2))

"""Points the firmware files at this checkout's component instead of GitHub's (CI and releases)."""

import pathlib
import sys

GITHUB = "  - source: github://dckiller51/esphome-xiaomi-ble-wake@main\n"
LOCAL = "  - source:\n      type: local\n      path: ../components\n"

for name in sys.argv[1:]:
    path = pathlib.Path(name)
    text = path.read_text()
    if GITHUB not in text:
        sys.exit(f"{name}: no GitHub source line to replace")
    path.write_text(text.replace(GITHUB, LOCAL))

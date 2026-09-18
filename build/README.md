# Firmware SDK (command-line)

Windows + Python 3 + Keil MDK ARMCC v5 (`Armcc` / `Armlink` / `Fromelf`).
Do not open a uVision project. Edit `build_config.json` if `ARMCC\Bin` is not at the default path.

From the SDK pack root:

```text
python build/build.py
python build/build.py -c
```

Or from this folder: `python build.py` / `python build.py -c`.

- `python build.py`: incremental (Keil Build).
- `python build.py -c`: delete `Output/` objects and `Output/firmware/` images, then compile and link everything (Keil Rebuild).

Which `.c` files are compiled:

- `source_dirs` in `build_config.json`: walk these folders (and subfolders) for `.c`.
- New `.c` under those folders is picked up automatically.
- `exclude`: skip a file (path or basename), same as unchecking it in Keil.
- Optional `sources`: extra single `.c` files outside `source_dirs`.

- `python build.py -v`: same build, but print full compiler/linker command lines on the console.

Console output is Keil-style (`compiling foo.c...`, `linking...`, `Program Size:`). The full command lines and toolchain dumps go to `Output/build.log`. Compile/link errors are printed on the console.

Layout:

```text
SDK/
  build/                 # this script, link.sct, build_config.json
  Output/                # .o, .axf, .map, build.log
    firmware/            # firmware.bin, firmware.hex
```

Upgrade:

- USB: use the HID IAP host tool (`pc_app/hid_iap_tool`) with `Output/firmware/firmware.bin`.
- SWD probe: use the separate flash host tool (`pc_app/jlink_flash_tool`). Not part of this compile pack.

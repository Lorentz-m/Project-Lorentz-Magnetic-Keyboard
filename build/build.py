# -*- coding: utf-8 -*-
"""Compile open sources with ARMCC, link prebuilt libs, emit bin/hex and a full log."""
from __future__ import print_function

import argparse
import datetime
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CFG_NAME = "build_config.json"
OUTPUT_DIR_NAME = "Output"
FIRMWARE_DIR_NAME = "firmware"
LOG_NAME = "build.log"


def pack_root(script_dir):
    if os.path.isdir(os.path.join(script_dir, "application")):
        return script_dir
    parent = os.path.dirname(script_dir)
    if os.path.isdir(os.path.join(parent, "application")):
        return parent
    sys.exit("cannot find application/ (run from the SDK pack)")


class BuildLog(object):
    def __init__(self, path):
        self.path = path
        self._fp = open(path, "w", encoding="utf-8")
        self.warnings = 0
        self.errors = 0

    def close(self):
        self._fp.close()

    def count(self, text):
        low = (text or "").lower()
        self.warnings += low.count("warning:")
        self.errors += low.count("error:")

    def file(self, text):
        if not text:
            return
        if not text.endswith("\n"):
            text = text + "\n"
        self._fp.write(text)
        self._fp.flush()
        self.count(text)

    def echo(self, text):
        if not text:
            return
        if not text.endswith("\n"):
            text = text + "\n"
        sys.stdout.write(text)
        sys.stdout.flush()

    def write(self, text):
        self.echo(text)
        self.file(text)


def load_config(root):
    path = os.path.join(root, CFG_NAME)
    if not os.path.isfile(path):
        sys.exit("missing %s" % path)
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def tool_path(armcc_bin, name):
    exe = os.path.join(armcc_bin, name)
    if os.path.isfile(exe):
        return exe
    exe_win = exe + ".exe"
    if os.path.isfile(exe_win):
        return exe_win
    return None


def collect_headers(root, includes):
    headers = []
    for inc in includes:
        d = os.path.join(root, inc.replace("/", os.sep))
        if not os.path.isdir(d):
            continue
        for dirpath, _, names in os.walk(d):
            for name in names:
                if name.lower().endswith(".h"):
                    headers.append(os.path.join(dirpath, name))
    return headers


def newer_than(path, ref_mtime):
    return os.path.isfile(path) and os.path.getmtime(path) > ref_mtime


def need_compile(src, obj, headers, force):
    if force or not os.path.isfile(obj):
        return True
    ot = os.path.getmtime(obj)
    if os.path.getmtime(src) > ot:
        return True
    for hdr in headers:
        if newer_than(hdr, ot):
            return True
    return False


def need_link(axf, objects, lib_paths, scatter_path, force):
    if force or not os.path.isfile(axf):
        return True
    ot = os.path.getmtime(axf)
    for path in objects + lib_paths + [scatter_path]:
        if newer_than(path, ot):
            return True
    return False


def clean_dir_files(root, dir_path, log, keep_names=()):
    removed = 0
    if not os.path.isdir(dir_path):
        return 0
    keep = set(n.lower() for n in keep_names)
    for name in os.listdir(dir_path):
        if name.lower() in keep:
            continue
        path = os.path.join(dir_path, name)
        if os.path.isfile(path):
            os.remove(path)
            log.file("remove %s" % rel_posix(root, path))
            removed += 1
    return removed


def norm_rel(rel):
    return rel.replace("\\", "/").strip().lstrip("./").lower()


def is_excluded(rel_n, name, exclude):
    return norm_rel(rel_n) in exclude or name.lower() in exclude


def scan_c_dir(root, rel_dir, exclude, selected, ignored, seen):
    d = os.path.normpath(os.path.join(root, rel_dir.replace("/", os.sep)))
    if not os.path.isdir(d):
        sys.exit("missing source_dirs: %s" % rel_dir.replace("\\", "/"))
    for dirpath, _, names in os.walk(d):
        for name in names:
            if not name.lower().endswith(".c"):
                continue
            path = os.path.join(dirpath, name)
            rel_n = rel_posix(root, path)
            key = os.path.normcase(os.path.abspath(path))
            if key in seen:
                continue
            if is_excluded(rel_n, name, exclude):
                ignored.append(rel_n)
                continue
            seen.add(key)
            selected.append(path)


def select_c_files(root, cfg):
    exclude = set()
    for item in cfg.get("exclude") or []:
        exclude.add(norm_rel(item))
        exclude.add(os.path.basename(item.replace("\\", "/")).lower())

    selected = []
    ignored = []
    seen = set()
    source_dirs = cfg.get("source_dirs") or []
    extra = cfg.get("sources") or []

    if not source_dirs and not extra:
        sys.exit("set source_dirs (or sources) in %s" % CFG_NAME)

    for rel_dir in source_dirs:
        scan_c_dir(root, rel_dir.strip(), exclude, selected, ignored, seen)

    for rel in extra:
        rel_n = rel.replace("\\", "/").strip()
        path = os.path.normpath(os.path.join(root, rel_n.replace("/", os.sep)))
        name = os.path.basename(rel_n)
        key = os.path.normcase(os.path.abspath(path))
        if is_excluded(rel_n, name, exclude):
            if rel_n not in ignored:
                ignored.append(rel_n)
            continue
        if not os.path.isfile(path):
            sys.exit("missing source: %s" % rel_n)
        if key in seen:
            continue
        seen.add(key)
        selected.append(path)

    selected.sort(key=lambda p: p.replace("\\", "/").lower())
    if not selected:
        sys.exit("no enabled .c files (check source_dirs/exclude in %s)" % CFG_NAME)
    return selected, ignored


def rel_posix(root, path):
    return os.path.relpath(path, root).replace("\\", "/")


def run_cmd(log, argv, cwd, verbose=False, echo_lines=None, echo_stdout=False):
    shown = subprocess.list2cmdline(argv)
    log.file("cwd: %s" % cwd)
    log.file("cmd: %s" % shown)
    if verbose:
        log.echo("cmd: %s" % shown)
    t0 = time.time()
    proc = subprocess.Popen(
        argv,
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )
    out, _ = proc.communicate()
    elapsed = time.time() - t0
    if out:
        if not out.endswith("\n"):
            out = out + "\n"
        log.file(out.rstrip("\n"))
    log.file("exit=%d  elapsed=%.3fs" % (proc.returncode, elapsed))
    if proc.returncode != 0:
        if out:
            log.echo(out.rstrip("\n"))
    elif echo_stdout and out:
        log.echo(out.rstrip("\n"))
    elif echo_lines and out:
        for line in out.splitlines():
            low = line.lower()
            if "warning:" in low or "error:" in low:
                log.echo(line)
                continue
            for prefix in echo_lines:
                if line.startswith(prefix):
                    log.echo(line)
                    break
    return proc.returncode, out or ""


def parse_compiler_banner(text):
    for line in (text or "").splitlines():
        if line.startswith("Component:"):
            return line.split(":", 1)[1].strip()
    return ""


def fmt_elapsed(sec):
    sec = int(round(sec))
    return "%02d:%02d:%02d" % (sec // 3600, (sec % 3600) // 60, sec % 60)


def parse_args(argv):
    ap = argparse.ArgumentParser(
        description="Compile open sources with ARMCC (default=incremental build)",
    )
    ap.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="Rebuild: delete objects then compile all (Keil Rebuild)",
    )
    ap.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="print full compiler/linker command lines on the console",
    )
    return ap.parse_args(argv)


def main(argv=None):
    args = parse_args(argv if argv is not None else sys.argv[1:])
    force = bool(args.clean)
    verbose = bool(args.verbose)
    mode = "rebuild" if force else "build"
    script_dir = HERE
    root = pack_root(script_dir)
    os.chdir(root)
    cfg = load_config(script_dir)
    armcc_bin = cfg.get("armcc_bin") or ""
    cpu = cfg.get("cpu") or "Cortex-M4.fp"
    includes = cfg.get("includes") or []
    libs = cfg.get("libs") or []
    scatter = cfg.get("scatter") or "link.sct"
    keep = cfg.get("keep") or ["Reset_Handler", "__Vectors"]

    build_dir = os.path.join(root, OUTPUT_DIR_NAME)
    firmware_dir = os.path.join(build_dir, FIRMWARE_DIR_NAME)
    os.makedirs(build_dir, exist_ok=True)
    os.makedirs(firmware_dir, exist_ok=True)
    log_path = os.path.join(build_dir, LOG_NAME)
    log = BuildLog(log_path)
    t_all = time.time()
    compiled = 0
    skipped = 0
    linked = 0
    try:
        log.file("=" * 72)
        log.file("%s start" % mode)
        log.file("=" * 72)
        log.file("mode: %s" % mode)
        log.file("flag: python build.py%s%s" % (" -c" if force else "", " -v" if verbose else ""))
        log.file("time: %s" % datetime.datetime.now().isoformat(sep=" ", timespec="seconds"))
        log.file("python: %s" % sys.version.replace("\n", " "))
        log.file("script: %s" % script_dir)
        log.file("root: %s" % root)
        log.file("armcc_bin: %s" % armcc_bin)
        log.file("cpu: %s" % cpu)

        log.write("Rebuild target" if force else "Build target")

        if force:
            log.write("cleaning...")
            clean_dir_files(root, build_dir, log, keep_names=(LOG_NAME,))
            clean_dir_files(root, firmware_dir, log)

        armcc = tool_path(armcc_bin, "Armcc")
        armlink = tool_path(armcc_bin, "Armlink")
        fromelf = tool_path(armcc_bin, "Fromelf")
        log.file("Armcc: %s" % (armcc or "NOT FOUND"))
        log.file("Armlink: %s" % (armlink or "NOT FOUND"))
        log.file("Fromelf: %s" % (fromelf or "NOT FOUND"))
        if not armcc or not armlink or not fromelf:
            log.write("FAIL: set armcc_bin in %s to ARMCC\\Bin" % CFG_NAME)
            return 1

        rc, vsn_out = run_cmd(log, [armcc, "--vsn"], root, verbose=verbose)
        if rc != 0:
            log.write("FAIL Armcc --vsn")
            return rc
        component = parse_compiler_banner(vsn_out)
        if component:
            log.write("*** Using Compiler '%s', folder: '%s'" % (component, armcc_bin))
        run_cmd(log, [armlink, "--vsn"], root, verbose=verbose)
        run_cmd(log, [fromelf, "--vsn"], root, verbose=verbose)

        c_files, ignored = select_c_files(root, cfg)
        log.file("c_files: %d" % len(c_files))
        for src in c_files:
            log.file("  %s" % rel_posix(root, src))
        if ignored:
            log.file("excluded: %d" % len(ignored))
            for rel in ignored:
                log.file("  skip %s (exclude)" % rel)

        inc_args = []
        for inc in includes:
            inc_args.extend(["-I", os.path.join(root, inc.replace("/", os.sep))])
        headers = collect_headers(root, includes)

        lib_paths = []
        for lib in libs:
            lib_path = os.path.join(root, lib.replace("/", os.sep))
            if not os.path.isfile(lib_path):
                log.write("FAIL: missing lib %s" % lib_path)
                return 1
            lib_paths.append(lib_path)

        objects = []
        for src in c_files:
            obj_name = os.path.splitext(os.path.basename(src))[0] + ".o"
            obj = os.path.join(build_dir, obj_name)
            if obj in objects:
                log.write("FAIL: duplicate object name %s" % obj_name)
                return 1
            objects.append(obj)
            if not need_compile(src, obj, headers, force):
                skipped += 1
                log.file("skip %s (up to date)" % rel_posix(root, src))
                continue
            compiled += 1
            argv = [
                armcc,
                "-c",
                "--cpu", cpu,
                "--apcs=interwork",
                "-g",
                "-O1",
                "--c99",
                "--gnu",
                "--split_sections",
                "--library_type=microlib",
                "-o", obj,
            ]
            argv.extend(inc_args)
            argv.append(src)
            log.write("compiling %s..." % os.path.basename(src))
            rc, _ = run_cmd(log, argv, root, verbose=verbose, echo_stdout=True)
            if rc != 0:
                log.write("FAIL compile %s" % rel_posix(root, src))
                return rc

        scatter_path = os.path.join(script_dir, scatter.replace("/", os.sep))
        if not os.path.isfile(scatter_path):
            scatter_path = os.path.join(root, scatter.replace("/", os.sep))
        if not os.path.isfile(scatter_path):
            log.write("FAIL: missing scatter %s" % scatter_path)
            return 1

        axf = os.path.join(build_dir, "firmware.axf")
        map_path = os.path.join(build_dir, "firmware.map")
        link_argv = [
            armlink,
            "--cpu", cpu,
            "--library_type=microlib",
            "--strict",
            "--summary_stderr",
            "--info", "summarysizes",
            "--info", "sizes",
            "--info", "totals",
            "--map",
            "--list", map_path,
            "--scatter", scatter_path,
            "--entry", "Reset_Handler",
            "-o", axf,
        ]
        for sym in keep:
            link_argv.append("--keep=%s" % sym)
        for obj in objects:
            link_argv.append(obj)
        for lib_path in lib_paths:
            link_argv.append(lib_path)

        bin_path = os.path.join(firmware_dir, "firmware.bin")
        hex_path = os.path.join(firmware_dir, "firmware.hex")
        do_link = need_link(axf, objects, lib_paths, scatter_path, force)
        if do_link:
            linked = 1
            log.write("linking...")
            rc, _ = run_cmd(
                log,
                link_argv,
                root,
                verbose=verbose,
                echo_lines=("Program Size:",),
            )
            if rc != 0:
                log.write("FAIL link")
                return rc
        else:
            log.file("skip link (up to date)")

        do_fromelf = do_link or (not os.path.isfile(bin_path)) or (not os.path.isfile(hex_path))
        if do_fromelf:
            log.write("FromELF: creating bin file...")
            rc, _ = run_cmd(log, [fromelf, "--bin", "--output", bin_path, axf], root, verbose=verbose)
            if rc != 0:
                log.write("FAIL fromelf bin")
                return rc
            log.write("FromELF: creating hex file...")
            rc, _ = run_cmd(log, [fromelf, "--i32", "--output", hex_path, axf], root, verbose=verbose)
            if rc != 0:
                log.write("FAIL fromelf hex")
                return rc
            run_cmd(log, [fromelf, "--text", "-z", axf], root, verbose=verbose)
        else:
            log.file("skip fromelf (up to date)")

        axf_disp = rel_posix(root, axf).replace("/", "\\")
        log.write('"%s" - %d Error(s), %d Warning(s).' % (axf_disp, log.errors, log.warnings))
        log.write("Build Time Elapsed:  %s" % fmt_elapsed(time.time() - t_all))
        log.file("compiled: %d" % compiled)
        log.file("skipped: %d" % skipped)
        log.file("linked: %d" % linked)
        return 0
    finally:
        log.close()


if __name__ == "__main__":
    sys.exit(main())

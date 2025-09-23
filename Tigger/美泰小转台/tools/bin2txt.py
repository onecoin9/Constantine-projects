#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将 bin 转为 TXT（与 C++ ProcessBinFileStep 输出一致）

支持：
- 从一个目录批量转换（如 history 或 cache）
- 仅转换一个 bin 文件

输出：output/<bin名>/dut_1..8.txt 与 dut_99.txt，首行表头：
gyro_x	gyro_y	gyro_z	acc_x	acc_y	acc_z	temperature	mix

注意：不会删除/移动源 bin（除非显式指定 --move-to-history 且提供源目录）。
"""
import argparse
import os
import struct
from pathlib import Path

HEADER_LINE = "gyro_x\tgyro_y\tgyro_z\tacc_x\tacc_y\tacc_z\ttemperature\tmix\n"

# TestFeedbackData 结构总长度（bytes），与 C++ 中解析一致
FRAME_SIZE = 284


def iter_frames_from_bin(f):
    while True:
        size_bytes = f.read(8)
        if not size_bytes or len(size_bytes) < 8:
            break
        # 长度字段按小端 qint64（与 Windows/C++ 行为一致）
        try:
            data_size = struct.unpack("<q", size_bytes)[0]
        except struct.error:
            break
        if data_size <= 0 or data_size > 16 * 1024 * 1024:
            # 合理性保护
            break
        packet = f.read(data_size)
        if len(packet) < data_size:
            break
        payload = packet[8:]
        if len(payload) >= FRAME_SIZE:
            yield payload


def parse_test_feedback(payload: bytes):
    off = 0
    testState, serialNumber, timestamp, dutActive, chipIndex = struct.unpack_from("<BIIHB", payload, off)
    off += 1 + 4 + 4 + 2 + 1

    duts = []
    for _ in range(8):
        gx, gy, gz, accx, accy, accz, mix, temp = struct.unpack_from("<iiiiiiih", payload, off)
        off += 4 * 7 + 2
        # 输出列顺序：温度在 mix 前（与 C++ 输出一致）
        duts.append((gx, gy, gz, accx, accy, accz, temp, mix))

    egx, egy, egz, eaccx, eaccy, eaccz, emix, etemp, ecounter = struct.unpack_from("<fffffffhh", payload, off)
    external = (egx, egy, egz, eaccx, eaccy, eaccz, etemp, emix)
    return testState, serialNumber, timestamp, dutActive, chipIndex, duts, external


def ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)


def process_one_bin(bin_path: Path, output_root: Path):
    base_name = bin_path.stem
    out_dir = output_root / base_name
    ensure_dir(out_dir)

    files = []
    try:
        # 1..8 路
        for i in range(1, 9):
            fp = open(out_dir / f"dut_{i}.txt", "w", encoding="utf-8", newline="\n")
            fp.write(HEADER_LINE)
            files.append(fp)
        # 外部陀螺 99 路
        fp_ext = open(out_dir / "dut_99.txt", "w", encoding="utf-8", newline="\n")
        fp_ext.write(HEADER_LINE)
        files.append(fp_ext)

        with open(bin_path, "rb") as f:
            for payload in iter_frames_from_bin(f):
                try:
                    _, _, _, dutActive, _, duts, external = parse_test_feedback(payload)
                except struct.error:
                    break
                # 写 1..8 路：仅当对应位为 1
                for i in range(8):
                    if (dutActive >> i) & 0x01:
                        gx, gy, gz, accx, accy, accz, temp, mix = duts[i]
                        files[i].write(f"{gx}\t{gy}\t{gz}\t{accx}\t{accy}\t{accz}\t{temp}\t{mix}\n")
                # 外部陀螺：x/y/z 任一非 0 则写
                egx, egy, egz, eaccx, eaccy, eaccz, etemp, emix = external
                if egx != 0.0 or egy != 0.0 or egz != 0.0:
                    files[8].write(f"{egx}\t{egy}\t{egz}\t{eaccx}\t{eaccy}\t{eaccz}\t{etemp}\t{emix}\n")
    finally:
        for fp in files:
            try:
                fp.close()
            except Exception:
                pass


def move_to_history(bin_path: Path, history_dir: Path):
    ensure_dir(history_dir)
    dst = history_dir / bin_path.name
    if dst.exists():
        try:
            dst.unlink()
        except Exception:
            pass
    try:
        bin_path.replace(dst)
    except Exception:
        import shutil
        shutil.copy2(bin_path, dst)
        try:
            bin_path.unlink()
        except Exception:
            pass


def main():
    ap = argparse.ArgumentParser(description="Convert bin files to TXT like ProcessBinFileStep.")
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("-s", "--source", help="源目录（如 history 或 cache），批量处理 *.bin")
    g.add_argument("-f", "--file", help="仅处理一个 bin 文件")
    ap.add_argument("-o", "--output", default="output", help="输出根目录，默认 output")
    ap.add_argument("--move-to-history", action="store_true", help="处理完将源 bin 移到 <源目录同级>/history（仅当使用 --source 时有效）")
    args = ap.parse_args()

    output_root = Path(args.output).resolve()
    ensure_dir(output_root)

    if args.file:
        bin_path = Path(args.file).resolve()
        if not bin_path.exists() or not bin_path.is_file():
            print(f"[ERROR] 文件不存在: {bin_path}")
            return 2
        print(f"[1/1] 处理 {bin_path.name} ...")
        process_one_bin(bin_path, output_root)
        print("完成。")
        return 0

    # 目录模式
    src_dir = Path(args.source).resolve()
    if not src_dir.exists() or not src_dir.is_dir():
        print(f"[ERROR] 源目录不存在: {src_dir}")
        return 2

    bin_files = sorted(src_dir.glob("*.bin"))
    if not bin_files:
        print("[WARN] 未找到任何 *.bin 文件。")
        return 0

    history_dir = src_dir.parent / "history"
    for i, bin_path in enumerate(bin_files, 1):
        print(f"[{i}/{len(bin_files)}] 处理 {bin_path.name} ...")
        process_one_bin(bin_path, output_root)
        if args.move_to_history:
            move_to_history(bin_path, history_dir)

    print("完成。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

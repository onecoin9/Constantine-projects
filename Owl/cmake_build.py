#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
APS CMake 构建脚本
自动配置、编译和安装 APS 项目
"""

import sys
import argparse
import platform
import subprocess
import shutil
import os
from pathlib import Path

# 导入独立的 CMakeBuilder 模块
sys.path.insert(0, str(Path(__file__).parent.parent))
from script.cmake_builder import CMakeBuilder


def deploy_qt_libraries(output_dir: Path) -> bool:
    """
    部署 Qt 库和插件
    
    Args:
        output_dir: 输出目录路径
        
    Returns:
        bool: 部署是否成功
    """
    print("\n" + "=" * 50)
    print("  Qt Library Deployment")
    print("=" * 50)
    
    # 查找 windeployqt
    windeployqt_paths = [
        Path(os.environ.get('Qt5_DIR', '')) / '../../../bin/windeployqt.exe',
        Path(os.environ.get('CMAKE_PREFIX_PATH', '')) / 'bin/windeployqt.exe',
        Path('C:/Qt/5.15.2/msvc2019_64/bin/windeployqt.exe'),
        Path('C:/Qt/5.15.1/msvc2019_64/bin/windeployqt.exe'),
        Path('D:/Qt/5.15.2/msvc2019_64/bin/windeployqt.exe'),
    ]
    
    windeployqt_exe = None
    for path in windeployqt_paths:
        if path.exists():
            windeployqt_exe = path
            break
    
    if not windeployqt_exe:
        print("WARNING: windeployqt not found. Qt libraries may not be properly deployed.")
        return False
    
    print(f"Found windeployqt: {windeployqt_exe}")
    
    # 检查 APS.exe 是否存在
    aps_exe = output_dir / "APS.exe"
    if not aps_exe.exists():
        print(f"ERROR: APS.exe not found at {aps_exe}")
        return False
    
    try:
        # 运行 windeployqt
        print("Running windeployqt...")
        cmd = [
            str(windeployqt_exe),
            "--no-translations",
            "--no-system-d3d-compiler", 
            "--no-opengl-sw",
            str(aps_exe)
        ]
        
        result = subprocess.run(cmd, cwd=output_dir, check=True, capture_output=True, text=True)
        print("windeployqt completed successfully.")
        
        # 整理插件目录
        print("Organizing Qt plugin directories...")
        
        # 创建插件子目录
        plugin_dirs = ["Plugins/styles", "Plugins/platforms", "Plugins/imageformats", "Plugins/iconengines"]
        for plugin_dir in plugin_dirs:
            plugin_path = output_dir / plugin_dir
            plugin_path.mkdir(parents=True, exist_ok=True)
        
        # 移动插件目录
        plugin_moves = [
            ("styles", "Plugins/styles"),
            ("platforms", "Plugins/platforms"), 
            ("imageformats", "Plugins/imageformats"),
            ("iconengines", "Plugins/iconengines")
        ]
        
        for src_dir, dst_dir in plugin_moves:
            src_path = output_dir / src_dir
            dst_path = output_dir / dst_dir
            
            if src_path.exists():
                print(f"Moving {src_dir} to {dst_dir}...")
                # 复制文件
                for item in src_path.iterdir():
                    if item.is_file():
                        shutil.copy2(item, dst_path)
                    elif item.is_dir():
                        shutil.copytree(item, dst_path / item.name, dirs_exist_ok=True)
                
                # 删除原目录
                shutil.rmtree(src_path)
        
        # 删除 bearer 目录
        bearer_path = output_dir / "bearer"
        if bearer_path.exists():
            print("Removing bearer directory...")
            shutil.rmtree(bearer_path)
        
        print("Qt deployment and plugin organization completed!")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"ERROR: windeployqt failed with return code {e.returncode}")
        print(f"Error output: {e.stderr}")
        return False
    except Exception as e:
        print(f"ERROR: Qt deployment failed: {e}")
        return False

def rcc_build(file_path: str, output_filepath: str) -> bool:
    """
    RCC 构建
    
    Args:
        file_path: 输入文件路径
        output_filepath: 输出文件路径
        
    Returns:
        bool: 构建是否成功
    """
    
    # 查找 rcc.exe
    rcc_paths = [
        Path(os.environ.get('Qt5_DIR', '')) / '../../../bin/rcc.exe',
        Path(os.environ.get('CMAKE_PREFIX_PATH', '')) / 'bin/rcc.exe',
        Path('C:/Qt/5.15.2/msvc2019_64/bin/rcc.exe'),
        Path('C:/Qt/5.15.1/msvc2019_64/bin/rcc.exe'),
        Path('D:/Qt/5.15.2/msvc2019_64/bin/rcc.exe'),
    ]

    rcc_exe = None
    for path in rcc_paths:
        if path.exists():
            rcc_exe = path
            break
    
    if not rcc_exe:
        print("WARNING: rcc not found. RCC build failed.")
        return False

    # 使用RCC编译.qrc文件
    rcc_build_cmd = "{rcc_exe} --binary {file_path} -o {output_filepath}".format(
        rcc_exe=rcc_exe, file_path=file_path, output_filepath=output_filepath)
    print("Running RCC build...")
    
    try:
        # 方法1: 使用 check=True 自动检查返回码
        result = subprocess.run(rcc_build_cmd, check=True, capture_output=True, text=True)
        print("RCC build completed successfully!")
        print(f"RCC output: {result.stdout}")
        
    except subprocess.CalledProcessError as e:
        print(f"ERROR: RCC build failed with return code {e.returncode}")
        print(f"Error output: {e.stderr}")
        return False
    except Exception as e:
        print(f"ERROR: RCC build failed: {e}")
        return False
    
    return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="APS CMake Build Script")
    parser.add_argument("--debug", action="store_true", 
                       help="Build in Debug mode (default: Release)")
    parser.add_argument("--clean", action="store_true",
                       help="Clean build directory before building")
    parser.add_argument("--build-dir", default="build",
                       help="Specify build directory (default: build)")
    args = parser.parse_args()


    # 创建构建器实例
    builder = CMakeBuilder(
        build_type="Debug" if args.debug else "Release",
        build_dir=args.build_dir,
        script_dir=Path(__file__).parent.absolute()
    )
    
    # 运行构建
    success = builder.run(clean=args.clean)
    
    if not success:
        sys.exit(1)


    # Qt 部署（如果启用）
    if platform.system() == "Windows":
        output_dir = Path(__file__).parent / "build" / "output"
        
        qt_success = deploy_qt_libraries(output_dir)
        if not qt_success:
            print("WARNING: Qt deployment failed, but build was successful!")
    
    # RCC 构建
    rcc_success = rcc_build(str(Path("Aprog") / "agclient.qrc"), str(output_dir / "agclient.rcc"))
    if not rcc_success:
        sys.exit(1)

if __name__ == "__main__":
    main()

import os
import sys

def load_file_list(file_path):
    """
    从文本文件中加载文件路径列表。
    """
    with open(file_path, 'r', encoding='utf-8') as file:
        return [line.strip() for line in file if line.strip()]

def get_actual_files(directory):
    """
    获取指定目录中的所有文件路径（相对路径）。
    """
    actual_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            relative_path = os.path.relpath(os.path.join(root, file), directory)
            actual_files.append(relative_path)
    return actual_files

def compare_files(expected_files, actual_files):
    """
    比对文件列表，返回丢失的文件和多余的文件。
    """
    expected_set = set(expected_files)
    actual_set = set(actual_files)

    missing_files = expected_set - actual_set
    extra_files = actual_set - expected_set

    return missing_files, extra_files

def main(file_list_path, target_directory):
    """
    主函数，接收文本文件路径和目标目录路径作为参数。
    """
    # 加载文本文件中的文件路径
    expected_files = load_file_list(file_list_path)
    print(f"文本文件中的文件数量: {len(expected_files)}")

    # 获取指定目录中的实际文件路径
    actual_files = get_actual_files(target_directory)
    print(f"指定目录中的文件数量: {len(actual_files)}")

    # 比对文件
    missing_files, extra_files = compare_files(expected_files, actual_files)

    # 输出结果
    if not missing_files and not extra_files:
        print("成功：文件完全一致！")
    else:
        print("失败：文件不一致！")
        if missing_files:
            print("丢失的文件：")
            for file in missing_files:
                print(f"  - {file}")
        if extra_files:
            print("多余的文件：")
            for file in extra_files:
                print(f"  - {file}")

if __name__ == "__main__":
    # 从命令行接收参数
    if len(sys.argv) != 3:
        print("用法: python compare_files.py <file_list.txt> <target_directory>")
        sys.exit(1)

    file_list_path = sys.argv[1]
    target_directory = sys.argv[2]

    # 调用主函数
    main(file_list_path, target_directory)
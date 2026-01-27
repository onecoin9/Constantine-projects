import subprocess
import os
import shutil
import configparser
import sys
import time
from lib.git.git_helper import git_helper as git_helper
from lib.cmd import cmd_base

class BuildPackager(cmd_base.CMDBase):
    def __init__(self, config_path='make_GitProject.ini', project_name=None):
        """
        初始化打包器
        :param config_path: 配置文件路径
        :param project_name: 要构建的项目名称 (Section名)
        """
        self.config = configparser.ConfigParser()
        self.config.read(config_path, encoding='utf8')
        
        if not project_name:
            raise ValueError("Must specify a project name")
            
        if project_name not in self.config:
            raise ValueError(f"Project '{project_name}' not found in configuration")

        # 读取通用配置
        if 'General' in self.config:
            self.base_dir = self.config['General'].get('base_dir', '.')
        else:
            self.base_dir = '.'
            
        # 读取项目特定配置
        proj_config = self.config[project_name]
        
        # 必须的字段，如果缺失会抛出异常
        self.local_path = os.path.join(self.base_dir, proj_config['local_path'])
        self.repo_url = proj_config['repo_url']
        self.repo_branch = proj_config.get('repo_branch', 'develop')
        self.repo_version = proj_config.get('repo_version', 'HEAD')
        
        # 可选字段
        self.build_path = os.path.join(self.local_path, proj_config.get('build_path', 'Build'))
        self.output_path = os.path.join(self.local_path, proj_config.get('output_path', 'output'))
        self.dest_path = proj_config.get('dest_path', '')
        self.build_script = proj_config.get('build_script', 'build.bat')

        # 初始化父类
        # 注意：cmd_base 可能需要一个字典作为简单的日志配置或参数传递
        # 这里为了兼容旧代码结构，构造一个类似的字典
        hash_para = {
            'repo_version': self.repo_version,
            'repo_type': 'git', # 现在固定为 git
            'build_script': self.build_script,
            'repo_url': self.repo_url,
            'repo_branch': self.repo_branch,
        }
        
        # 确保父类 CMDBase 能够正确初始化，如果它比较简单可以直接调用
        try:
            cmd_base.CMDBase.__init__(self, hash_para)
        except Exception as e:
            # 如果父类初始化很简单或者不需要参数，可以尝试不带参数，或者简单打印日志
            print(f"Warning: CMDBase init failed or skipped: {e}")
            self.log = self._get_simple_logger()

    def _get_simple_logger(self):
        import logging
        logging.basicConfig(level=logging.INFO)
        return logging.getLogger("BuildPackager")

    def _ensure_clean_git_state(self):
        """确保本地 Git 仓库干净（丢弃所有本地修改）"""
        git_dir = os.path.join(self.local_path, '.git')
        if not os.path.exists(self.local_path) or not os.path.exists(git_dir):
            return

        self.log.info("检测到本地仓库，正在清理本地修改以避免冲突...")
        try:
            # 1. 重置索引和工作区
            # git reset --hard
            subprocess.run(['git', 'reset', '--hard'], cwd=self.local_path, 
                          check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            # 2. 清理未跟踪文件
            subprocess.run(['git', 'clean', '-fd'], cwd=self.local_path,
                          check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            # 3. 确保切换到正确的分支
            self.log.info(f"切换到分支: {self.repo_branch}")
            subprocess.run(['git', 'checkout', self.repo_branch], cwd=self.local_path,
                          check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            self.log.info("本地仓库清理及分支切换完成")
        except subprocess.CalledProcessError as e:
            self.log.warning(f"本地仓库清理失败: {e}，后续 Git 操作可能会遇到合并冲突")

    def clean_build_directory(self):
        self.log.info(f"清理构建目录内容: {self.build_path}")
        if not os.path.exists(self.build_path):
            self.log.info("构建目录不存在，无需清理")
            return True

        try:
            for item in os.listdir(self.build_path):
                item_path = os.path.join(self.build_path, item)
                if os.path.isfile(item_path) or os.path.islink(item_path):
                    os.unlink(item_path)
                elif os.path.isdir(item_path):
                    shutil.rmtree(item_path)

            self.log.info("构建目录内容已清理")
            return True
        except Exception as e:
            self.log.error(f"清理构建目录时出错: {e}")
            return False

    def run_build_script(self, script_name, timeout=600):
        script_path = os.path.join(self.local_path, script_name)
        self.log.info(f"Executing build script: {script_path}")
        
        cmd = script_path
        use_shell = False
        
        # 判断脚本类型，构建合适的命令
        if script_name.endswith('.py'):
            cmd = [sys.executable, script_path]
        elif script_name.endswith('.bat'):
            # 方案：使用 cmd.exe /c "echo. | script.bat"
            # 这样可以在脚本执行时自动将回车符传给 pause，实现跳过
            cmd = f'cmd.exe /c "echo. | "{script_path}""'
            use_shell = False # 直接执行 cmd.exe
        
        try:
            # 启动进程，合并 stderr 到 stdout 以便统一按顺序查看
            proc = subprocess.Popen(
                cmd,
                shell=use_shell,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, 
                cwd=self.local_path,
                text=True,
                encoding='gbk', # Windows bat 常用 GBK
                errors='replace',
                bufsize=1 # 行缓冲
            )

            start_time = time.time()

            while True:
                # 检查超时
                if time.time() - start_time > timeout:
                    proc.kill()
                    self.log.error(f"Execution timed out after {timeout} seconds.")
                    return False

                # 使用 poll() 检查进程状态
                return_code = proc.poll()
                
                # 尝试读取一行输出
                line = proc.stdout.readline()
                
                # 如果有输出则打印
                if line:
                    self.log.info(line.strip())

                # 如果没有输出 且 进程已结束，则跳出循环
                if not line and return_code is not None:
                    break
                    
                # 如果没有输出 但 进程还在运行
                if not line:
                    time.sleep(0.1)

            if proc.returncode != 0:
                self.log.warning(f"Script {script_name} exited with non-zero status code: {proc.returncode}")
                return False

            self.log.info("Build script completed.")
            return True

        except Exception as e:
            self.log.error(f"Failed to execute script {script_name}: {e}")
            return False

    def execute(self):
        self.log.info("=" * 50)
        self.log.info(f"项目路径: {self.local_path}")
        self.log.info(f"仓库地址: {self.repo_url} ({self.repo_branch})")
        self.log.info("=" * 50)

        # 强制清理本地变更
        self._ensure_clean_git_state()

        # 1. 拉取代码
        params = {
            'file_path': self.local_path,
            'repo_url': self.repo_url,
            'repo_branch': self.repo_branch,
            'repo_version': self.repo_version
        }

        try:
            handler = git_helper(params)
            handler.init()
        except Exception as e:
            self.log.error(f"Git 操作失败: {e}")
            return False
            
        # 2. 清理旧构建 (可选)
        # self.clean_build_directory()

        # 3. 执行构建脚本
        if self.build_script and self.build_script.lower() != 'none':
             if not self.run_build_script(self.build_script):
                 self.log.error("构建脚本执行失败")
                 return False
        else:
            self.log.info("未配置构建脚本，跳过构建步骤")

        # 4. 移动产物
        if self.dest_path:
             self.move_package()
        
        self.log.info("流程执行完成")
        self.log.info("=" * 50)
        return True


    def move_package(self):
        if not self.dest_path:
            return False
            
        self.log.info(f"准备移动产物从 {self.output_path} 到 {self.dest_path}")
        
        if not os.path.exists(self.output_path):
            self.log.error(f"输出目录不存在: {self.output_path}")
            return False

        if not os.path.exists(self.dest_path):
            try:
                os.makedirs(self.dest_path)
            except Exception as e:
                self.log.error(f"无法创建目标目录: {e}")
                return False

        try:
            # 兼容低版本 Python 的复制策略
            # 遍历源目录的所有内容
            for item in os.listdir(self.output_path):
                src = os.path.join(self.output_path, item)
                dst = os.path.join(self.dest_path, item)
                
                # 如果目标已存在，先删除
                if os.path.exists(dst):
                    if os.path.isdir(dst):
                        shutil.rmtree(dst)
                    else:
                        os.unlink(dst)
                
                # 执行复制
                if os.path.isdir(src):
                    shutil.copytree(src, dst)
                else:
                    shutil.copy2(src, dst)
                    
            self.log.info(f"已将内容拷贝至 {self.dest_path}")
            return True
        except Exception as e:
            self.log.error(f"拷贝文件时出错: {e}")
            return False

def get_available_projects(config_path):
    config = configparser.ConfigParser()
    config.read(config_path, encoding='utf8')
    # 排除 General 和 Paths Settings 等非项目段落
    excluded = ['General', 'Paths', 'Settings', 'DEFAULT']
    return [s for s in config.sections() if s not in excluded]

if __name__ == "__main__":
    ini_file = 'make_GitProject.ini'
    if not os.path.exists(ini_file):
        print(f"Error: Configuration file '{ini_file}' not found.")
        sys.exit(1)

    projects = get_available_projects(ini_file)
    
    if not projects:
        print("No projects found in configuration file.")
        sys.exit(1)

    print("Available Projects:")
    for idx, proj in enumerate(projects):
        print(f"{idx + 1}. {proj}")
    
    selected_indices = input("\nEnter project numbers to build (comma separated, e.g. 1,3) or 'all': ").strip()
    
    target_projects = []
    if selected_indices.lower() == 'all':
        target_projects = projects
    else:
        try:
            indices = [int(i.strip()) - 1 for i in selected_indices.split(',')]
            for i in indices:
                if 0 <= i < len(projects):
                    target_projects.append(projects[i])
        except ValueError:
            print("Invalid input.")
            sys.exit(1)
            
    if not target_projects:
        print("No valid projects selected.")
        sys.exit(1)
        
    for proj_name in target_projects:
        print(f"\n>>> Starting process for project: {proj_name}")
        packager = BuildPackager(config_path=ini_file, project_name=proj_name)
        packager.execute()
        print(f"<<< Finished process for project: {proj_name}\n")
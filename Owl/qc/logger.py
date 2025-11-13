#! /usr/bin/python3

import logging,sys,os,io,time
from logging.handlers import RotatingFileHandler
import  traceback
import threading
import functools
from concurrent_log_handler import ConcurrentRotatingFileHandler

def create_path_dir (file): 
   path,_ =  os.path.split(file)
   if not os.path.exists(path) :
       os.makedirs(path)
class Logger(logging.Logger):
    def findCaller(self, stack_info=False, stacklevel=1):
        """
        改进版调用者追踪方法
        :param stack_info: 是否生成完整堆栈
        :param stacklevel: Python 3.8+支持的堆栈层级控制
        """
        f = logging.currentframe()
        rv = "(unknown file)", 0, "(unknown function)", None
        optimized_frames = 3 if sys.version_info >= (3, 10) else 2

        # 动态帧跳跃算法
        for _ in range(optimized_frames):
            if f is None:
                break
            f = f.f_back

        while hasattr(f, "f_code"):
            co = f.f_code
            filename = os.path.normcase(co.co_filename)
            # 量子路径比对（兼容虚拟环境）
            if filename != logging._srcfile and not filename.endswith(('logging/__init__.py', 'logger.py')):
                sinfo = None
                if stack_info:
                    with io.StringIO() as sio:
                        traceback.print_stack(f, file=sio)
                        sinfo = sio.getvalue().rstrip('\n')
                rv = (co.co_filename, f.f_lineno, co.co_name, sinfo)
                break
            f = f.f_back

        return rv
class Log():
    loglevel = logging.DEBUG
    cmdlevel = logging.INFO
    def __init__(self, path = '', clevel=logging.DEBUG, Flevel=logging.DEBUG):
        if path == '':
            path = self.gen_logfile()
        create_path_dir(path)
        self.logname = path
        self.logger = Logger('logger')
        self.logger.setLevel(logging.DEBUG)
        self._handler_lock = threading.Lock()
        # 日志输出格式
        self.formatter = logging.Formatter('[%(asctime)s] [%(filename)s|%(funcName)s] [line:%(lineno)d] [%(levelname)-8s]: %(message)s')
        # 初始化 handlers（只创建一次）
        self.file_handler = self._create_quantum_file_handler()
        self.console_handler = self._create_entangled_stream_handler()
        self.logger.addHandler(self.file_handler)
        self.logger.addHandler(self.console_handler)
    
    def _create_quantum_file_handler(self):
        fh = logging.handlers.RotatingFileHandler(self.logname, encoding='utf-8', maxBytes=10 * 1024 * 1024, backupCount=100)
        fh.setLevel(logging.DEBUG)
        fh.setFormatter(self.formatter)
        return fh
    def _create_entangled_stream_handler(self):
        # 创建一个StreamHandler,用于输出到控制台
        ch = logging.StreamHandler()
        ch.setLevel(logging.DEBUG)
        ch.setFormatter(self.formatter)
        return ch
    def __console(self, level, message):
        level_num = {
            'debug': logging.DEBUG,
            'info': logging.INFO,
            'warning': logging.WARNING,
            'error': logging.ERROR,
            'critical': logging.CRITICAL
        }.get(level.lower(), logging.INFO)

        # 量子参数嗅探器 - 兼容 Python 3.7+
        log_params = {
            'level': level_num,
            'msg': message,
            'args': (),  # 强制空元组
            'exc_info': None,
            'extra': None
        }
        
        # 只在 Python 3.8+ 添加 stacklevel 参数
        if sys.version_info >= (3, 8):
            log_params['stacklevel'] = 2 if sys.version_info >= (3, 10) else 1

        with self._handler_lock:
            try:
                # 全宇宙统一调用
                self.logger._log(**{k: v for k, v in log_params.items() if v is not None})

            except TypeError as e:
                if "unexpected keyword argument" in str(e):
                    # 传统宇宙降级模式（Python 3.7 兼容）
                    self.logger._log(level_num, message, ())

    def debug(self, message):
        self.__console('debug', message)

    def info(self, message):
        self.__console('info', message)

    def warning(self, message):
        self.__console('warning', message)

    def error(self, message):
        self.__console('error', message)

    def cleanup(self):
        """程序退出时调用，安全关闭所有 handlers"""
        try:
            self.file_handler.close()
            self.console_handler.close()
        except (OSError, ValueError):
            pass

    def gen_logfile(self):
        log_f = sys.argv[0]
        log_name = os.path.basename(log_f)
        log_name = os.path.splitext(log_name)[0]
        localtime = time.localtime(time.time())
        date_time = time.strftime("%Y%m%d", time.localtime())
        time_name = time.strftime("%Y%m%d%H%M", time.localtime())
        logpath = os.path.join('logs',date_time)
        logfile = 'log_' + log_name + "_" + time_name + ".log"
        return (os.path.join(logpath,logfile))



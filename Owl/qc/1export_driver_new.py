#!/usr/bin/python
#import sqlite3
import os
import time
import re
import shutil
from lib.file.database.database_c import DB_file as db
from lib.file.html.html_c import Html_file as html
from lib.file import file
from lib.file.ko.ko_c import Ko_file as ko
from lib.libbase import Base as libbase
def main():
    #发布驱动数据
    # local_path=''
    #驱动发布根目录
    # export_path        = "Q:\Aprog_Export\AP8000_Export"
    export_path        = "Z:/企业空间/研发平台部/02_测试部/07_驱动版本/01_AP8000/03_Process/AP8000_Export"
    #帮助文件html上一级目录
    help_path          = 'I:/AP8000/Help_S4'
    #源驱动目录
    src_driver_dir = 'Q:/Aprog_QC'
    # src_driver_dir     = 'Z:/企业空间/研发平台部/02_测试部/07_驱动版本/04_DriverKo/AP8000_QCTest'
    #history文件类型
    # history_filename   = 'S:/AP8x000_Release_S3/package_source/chipdb/AP8000_Release_History.txt'
    history_filename   ='Z:/企业空间/研发平台部/02_测试部/07_驱动版本/01_AP8000/01_Release/package_source/chipdb/AP8000_Release_History.txt'

    # release_db_file    = 'S:/AP8x000_Release_S3/package_source/chipdb/DB18U.dbs'
    release_db_file    = 'Z:/企业空间/研发平台部/02_测试部/07_驱动版本/01_AP8000/01_Release/package_source/chipdb/DB18U.dbs'

    # modify_driver_path = 'S:/AP8x000_Release_S3/package_source/chipdb/bin'
    modify_driver_path = 'Z:/企业空间/研发平台部/02_测试部/07_驱动版本/01_AP8000/01_Release/package_source/chipdb/bin'

    # jbc_path           = 'S:/AP8x000_Release_S3/package_source/chipdb/bin'
    jbc_path = 'Z:/企业空间/研发平台部/02_测试部/07_驱动版本/01_AP8000/01_Release/package_source/chipdb/bin'
    #记录日志文件
    # log_filename       = os.path.join(export_path,'epxort_log.log')
    log_filename = os.path.join(export_path, 'epxort_log.log')
    #=============================用户配置修改项=======================================
    #+++++++++++++++++++++++根据各自本地目录修改+++++++++++++++++++++++++++    
    #源数据库路径
    src_db_path        = 'D:/testdb'
    src_jbc_path       = 'D:/version/Aprog-V1.2.66(20210322)/bin'
    #++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    #+++++++++++++++++++++++根据驱动类型修改+++++++++++++++++++++++++++++++
    ''' 
        驱动新增加类型，driver_type取值:modify,add及其它，
             modify是修改的驱动，
             add是新增的驱动，其它值自动判断是否新增,如果自动判断运行比较慢
    '''
    driver_type      = 'add'
    #++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    #+++++++++++++++++++++++新增的驱动数据库名称+++++++++++++++++++++++++++++ 
    #新增发布驱动数据库名称，不带扩展名  
    #dbfiles = ['C8051F569','TMS320F280023','HUSB362','AG568N','STM32F205VGT','PIC16F1779','S32K314EHT1','AT32F435CGU','ING91870C','CY8C4014SX']
    dbfiles = ['G8CMS79Fxx']

    #++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    #+++++++++++++++++++++++修改的驱动名称 +++++++++++++++++++++++++++++++++     
    #修改驱动名称
    modify_dbfiles = ['Hi3286CV100']
    #++++++++++++++++++++++修改HELP路径++++++++++++++++++++++++++++++++
    
    # modify_html_path = 'S:/AP8x000_Release_S3/package_source/Aprog_help_release/V1.04.64(20201231)/html'
    modify_html_path = 'S:/AP8x000_Release_S3/package_source/Aprog_help_release/V1.04.64(20201231)/html'

    #=================================================================================
    export_driver = Driver({'logfile':log_filename})
    export_driver.set_driver_type(driver_type)  #设置驱动类型
    export_driver.set_export_path(export_path)  #设置发布根路径
    export_driver.set_help_path(help_path)      #设置HTML路径
    export_driver.set_driver_path(src_driver_dir) #设置源驱动路径
    export_driver.set_jbc_path(jbc_path)             #设置JBC文件路径
    #export_driver.set_src_jbc_path(src_jbc_path)     #设置JBC文件路径
    export_driver.set_history_file(history_filename) #设置history文件路径
    export_driver.set_release_dbfile(release_db_file) #设置最新数据库文件
    export_driver.history_file.read_file()           #读取history文件的内容 

    if driver_type == 'add':
        export_driver.set_export_path(export_path)  # 设置发布根路径
        for dbfile in dbfiles :
            export_driver.set_export_dbfile(os.path.join(src_db_path , dbfile + '.dbs'))             
            export_driver_file(export_driver)            #发布驱动文件
            modify_history(export_driver)                #修改history内容
            export_driver.clear()
    elif driver_type =='modify':
         for modify_dbfile in modify_dbfiles:
            flag = export_driver.check_modifydb_exist(os.path.join(src_db_path , modify_dbfile + '.dbs'))
            if not flag:
                export_driver.log.info("请确认数据库是否正确，你导入的数据在正式版本中并不存在，有问题请咨询谭美娟处理")
                return 0
            export_driver.set_export_dbfile(os.path.join(src_db_path , modify_dbfile + '.dbs'))
            export_driver.set_modify_html_path(modify_html_path)
            #export_driver.set_driver_file(export_driver.get_driver_path() + driver_name + '.ko')
            export_driver_file(export_driver,modify_driver_path)
            modify_history(export_driver)
            rs = export_driver.check_release_chipinfo()
            assert  rs== True,"更新芯片的数据库记录与正式版本数据库记录不一致"
            export_driver.clear()

    write_history(export_driver)                     #修改history文件
    export_driver.log.info("拷贝成功驱动数量：" + str(export_driver.sucess_count))
    export_driver.log.info("拷贝失败驱动数量：" + str(len(export_driver.fail_drivers)))
    export_driver.log.info("以下驱动文件拷贝失败:")
    export_driver.log.info(export_driver.fail_drivers)
    export_driver.log.info("拷贝失败驱动数量：" + str(len(export_driver.fail_jbcs)))
    export_driver.log.info("以下JBC文件拷贝失败:")  
    export_driver.log.info(export_driver.fail_jbcs)
'''
Driver 类主要保存获取导出驱动的参数
属性：
    属性名                 类型          描述
    html_file_names  :    list      保存当前db中的所有html，ko等信息
    verion           :    string    保存发布驱动的版本（仅作创建目录使用），内部使用
    export_db_file   :    DB_file   导出驱动的数据库文件
    release_db_file  :    DB_file   前一个版本正式发布的数据库
    html_file        :    Html_file 导出当前驱动的html文件
    doc_file         :    doc_file  导出当前驱动的docx文件
    db_log_file      :    File      导出驱动的日志文件
    ko_file          :    File      导出驱动的KO文件
    ext_para_file    :    File      导出驱动的EXT_PARA文件
    xml_file         :    File      导出驱动的XML文件
    chk_file         :    File      导出驱动的CHK文件
    history_file     :    File      版本发布的的history文件
    export_path      :    string    导出驱动根路径
    help_html_path   :    string    源HTML存储路径
    help_doc_path    :    string    源DOCX存储路径
    driver_path      :    string    源Aprog存储路径
    driver_path_bin  :    string    源驱动存储路径
    fail_drivers     :    list      没有COPY的驱动名称列表
    sucess_count     :    int       拷贝成功的计数
方法：
   方法名                描述
   set_export_dbfile    设置导出驱动的数据库文件
   set_html_file        设置当前驱动的html文件
   set_driver_file      设置导出驱动的KO,EXT_PARA,XML,CHK文件
   set_history_file     设置版本发布的的history文件
   set_relase_dbfile    设置前一个版本正式发布的数据库
   set_export_path      设置导出驱动根路径
   set_help_path        设置源HTML和源DOCX存储路径
   set_driver_path      设置源驱动存储上一级目录

   get_driver_path      获取源驱动的目录
   get_export_version   获取版本号
   get_export_root_path 获取导出驱动的根目录
   get_export_path      获取导出驱动的目录
   get_html_file_name   获取当前DB所有的HTML信息

   create_export_dir    创建导出驱动的目录树
'''
class Driver(libbase):
    def __init__(self,para = {}):
        libbase.__init__(self,para)
        self.html_file_names = {}
        self.version =  'V1.0'
        self.fail_drivers = []
        self.sucess_count = 0
        self.fail_jbcs    = []
        self.same_ko_dir = []
    def clear(self):
        self.html_file_names = {}
        self.version =  'V1.0'
        self.fail_drivers = []
        self.fail_jbcs    = []
        self.same_ko_dir = []   

    def set_export_version(self,verison):
        self.version =  verison
    '''
    功能描述：设置导出驱动的根目录
    参数：
        参数名         类型         描述
        export_path   string    导出驱动的根目录 
    '''
    def set_export_path(self,export_path):
        self.export_path = export_path
    '''
    功能描述：设置源HTML和源DOCX存储路径
    参数：
        参数名         类型         描述
        help_path   string    源HTML和DOCX存储的上一级目录 
    '''
    def set_help_path(self,help_path):
        self.help_html_path = os.path.join (help_path , 'html/')
        self.help_doc_path = os.path.join(help_path , 'docfiles/')
    '''
    功能描述：设置源驱动存储上一级目录
    参数：
        参数名         类型         描述
        driver_path   string    源驱动存储上一级目录 
    '''
    def set_driver_path(self,driver_path):
        self.driver_path = driver_path
        
    '''
    功能描述：设置源驱动存储上一级目录
    参数：
        参数名         类型         描述
        jbc_path   string    源驱动存储上一级目录 
    '''
    def set_jbc_path(self,jbc_path):
        self.jbc_path = jbc_path
        
    '''
    功能描述：设置驱动类型
    参数：
        参数名         类型         描述     取值范围
        driver_type   string    驱动类型     auto,add,modify
    '''
    def set_driver_type(self,driver_type):
        self.driver_type = driver_type 
        
    def get_driver_type(self):
        return self.driver_type
    '''    
    功能描述：设置导出驱动的数据库文件及LOG文件
    参数：
        参数名         类型         描述
        db_filename   string    导出驱动的数据库文件名,完整路径
    '''

    def find_ko_files(self,log_content):
        # 使用正则表达式匹配所有.ko文件名
        ko_files = re.findall(r'\b(\w+\.ko)\b', log_content)
        return ko_files

    def set_export_dbfile(self,db_filename):
        self.export_db_file = db({"filename":db_filename})
        self.export_db_file.open ()
        self.get_html_file_name()
        filename,_ = os.path.splitext(self.export_db_file.get_file_name())
        db_log_filename = filename + "_export.log"
        with open(db_log_filename, 'r', encoding='utf-8') as f:
            log_content = f.read()
        ko_file = self.find_ko_files(log_content)
        if len(ko_file) == 1:
            self.db_log_file = file.File({"filename": db_log_filename})
            self.db_log_file.open()
        elif len(set(ko_file))==1:
            self.db_log_file = file.File({"filename": db_log_filename})
            self.db_log_file.open()
        else:
            assert len(set(ko_file)) == 1, "该db数据不符合，存在多个不同名的驱动，请检查数据"


    '''    
    功能描述：检查要要修改的驱动对应的数据库是否在S盘存在
    参数：
        参数名         类型         描述
        db_filename   string    导出驱动的数据库文件名,完整路径
        没有试多条会怎么样，不好意思
    '''
    def check_modifydb_exist(self,db_filename):
        self.export_db_file = db({"filename":db_filename})
        self.export_db_file.open()
        export_data = self.get_dbfile_list()
        flag = True
        for data in export_data:
            chipinfo = {}
            chipinfo['chipname'] = data[2]
            chipinfo['package'] = data[4]
            chipinfo['adapter'] = data[3]
            flag = self.release_db_file.chip_is_exist(chipinfo)
        return flag

    def get_jbc_list(self):
        jbc_all_list =[]
        jbc1_list = self.export_db_file.get_all_jbc_file()
        jbc2_list = self.export_db_file.get_all_jbc2_file()
        for jbcs in jbc1_list + jbc2_list :
            if jbcs[-1] not in jbc_all_list:
                jbc_all_list.append(jbcs[-1])
        return jbc_all_list
    
    def copy_jbcs(self):
        for jbc_file in self.get_jbc_list():
            jbc_file_o = file.File({'filename':os.path.join(self.get_driver_path(),jbc_file)})
            is_suc = jbc_file_o.copy_file(self.jbc_path)
            if not is_suc:
                self.fail_jbcs.append(jbc_file)


    '''
    功能描述：设置前一个版本正式发布的数据库
    参数：
        参数名         类型         描述
        db_filename   string    前一个版本正式发布的数据库文件名,完整路径
    '''
    def set_release_dbfile(self,db_filename):
        self.release_db_file = db({"filename":db_filename})
        self.release_db_file.open()

    '''
    功能描述：设置版本发布的的history文件
    参数：
        参数名         类型         描述
        history_file   string    设置版本发布的的history文件名,完整路径
    '''
    def set_history_file(self,history_file):
        self.history_file = file.File({"filename":history_file})
        self.history_file.open()

    '''
    功能描述：设置导出驱动的KO,EXT_PARA,XML,CHK文件
    参数：
        参数名         类型         描述
        driver_file   string    发布的KO文件名,完整路径
    '''
    def set_driver_file(self,driver_file):
        self.log.info("driver_file: %s"%(driver_file))
        self.ko_file = ko({"filename":driver_file})
        self.ko_file.open()
        self.ko_file.read_file()
        
        extfile = driver_file.replace('ko','extdata')
        self.ext_para_file =   file.File({"filename":extfile}) 
        chkfile = driver_file.replace('ko','chk')
        self.chk_file =   file.File({"filename":chkfile}) 
        xmlfile = driver_file.replace('ko','xml')
        self.xml_file =   file.File({"filename":xmlfile})
        
        self.gen_export_dir()
        #找到是新的SVN目录，放到self.newest_dir
        self.gen_svn_is_newest()
        #self.export_driver_dir = self.ko_file.get_main_name()
           
    def get_ko_name_from_file(self,filename):
        strs = filename.split('_SVN')
        ko_name = ('_SVN').join(strs[0:-1])
        return ko_name
    def gen_export_dir(self):
        files = os.listdir(self.export_path)
        self.svn_no = self.ko_file.get_svn_number()

        localtime = time.localtime(time.time())
        time_name = time.strftime("%Y%m%d%H%M%S", time.localtime())
        self.export_driver_dir = self.ko_file.get_main_name() + '_' + 'SVN'+ str(self.svn_no) + '_' + time_name
        if self.export_driver_dir not in self.same_ko_dir :
            self.same_ko_dir.append(self.export_driver_dir)
        #files.reverse()
        #self.same_ko_dir = {}
        ko_num = []
        for f in files:
            if self.export_driver_dir == f:
                ko_num.append(int(f.split('_')[-2].split('SVN')[-1],16))
            if self.ko_file.get_main_name() == self.get_ko_name_from_file(f):
                self.same_ko_dir.append(f)
        if len(ko_num) != 0:
            ko_num.reverse()
            self.export_driver_dir = self.export_driver_dir +'_'+ hex(ko_num.reverse()[0] + 1)[2::]
        self.log.info("self.export_driver_dir: %s"%(self.export_driver_dir))


    '''
    功能描述：设置当前驱动的html文件
    参数：
        参数名         类型         描述
        html_file_name   string    源HTML文件名,完整路径
    '''
    def set_html_file(self,html_file_name):
        if html_file_name != '' :
            self.html_file = html({"filename":html_file_name})
            doc_name = self.html_file.get_main_name() + '.docx'
            self.doc_file = html({"filename":os.path.join(self.help_doc_path , doc_name)})
    '''
    功能描述：设置修改驱动的HTML放置路径
    参数：
        参数名         类型         描述
        html_file_name   string    源HTML文件名,完整路径
    '''
    def set_modify_html_path(self,path):
        self.modify_html_path = path

    '''
    功能描述：获取源驱动的目录
    参数：
        参数名         类型         描述
    返回值：
        源驱动的目录 string 
    '''
    def get_driver_path(self):
        return os.path.join(self.driver_path , 'bin/')
    '''
    功能描述：获取源驱动的目录
    参数：
        参数名         类型         描述
    返回值：
        源驱动的目录 string 
    '''
    def get_jbc_path(self):
        return self.jbc_path

    '''
    功能描述：获取导出驱动的版本号
    参数：
        参数名         类型         描述
    返回值：
        获取导出驱动的版本号 string 
    '''
    def get_export_version(self):
        return self.version
    '''
    功能描述：获取导出驱动的根路径
    参数：
        参数名         类型         描述
    返回值：
        获取导出驱动的根路径 string 
    '''
    def get_export_root_path(self):
        return  self.export_path
    '''
    功能描述：获取导出驱动的路径
    参数：
        参数名         类型         描述
    返回值：
        获取导出驱动的路径 string 
    '''
    def get_export_path(self):
        return os.path.join(self.export_path , self.export_driver_dir)
    '''
    功能描述：获取当前DB所有的HTML信息
    参数：
        参数名         类型         描述
  
    '''
    def get_html_file_name(self):
        self.export_db_file.get_html_name(self.html_file_names)

    '''
    功能描述：获取当前DB芯片信息
    参数：
        参数名         类型         描述

    '''
    def get_dbfile_list(self):
        return self.export_db_file.get_chip_list()

    def get_html_path(self):
        if self.driver_type == 'add':
           #设置导出驱动的帮助目录
           return(os.path.join(self.get_export_path() , "help/"))
        else:
           return (self.modify_html_path)

    '''
    功能描述：创建导出驱动的目录树
        目录结构
        驱动名称_版本_年月日时分秒
            bin
            fpga
            code
                驱动名称_版本
            help
    '''
    def create_export_dir(self):
        os.chdir(self.export_path)
        self.log.info(self.export_driver_dir)
        if not os.path.exists(self.export_driver_dir):
            os.mkdir(self.export_driver_dir)
        os.chdir(self.export_driver_dir)
        subdirs = ['bin','fpga','code','help']
        for subdir in subdirs:
            if not os.path.exists(subdir):
                os.mkdir(subdir)
        os.chdir('code')
        if not os.path.exists(self.ko_file.get_main_name()):
            os.mkdir(self.ko_file.get_main_name())
    def gen_svn_is_newest(self):
        svn_nos = []
        if len(self.same_ko_dir) != 1:
            for f in self.same_ko_dir:
                svn_nos.append(int((f.split('SVN')[-1]).split('_')[0]))
        svn_nos.sort()
        if (len(svn_nos) > 0) and (int(self.svn_no) < int(svn_nos[-1])):
            for f in self.same_ko_dir:
                if str(svn_nos[-1]) in f:
                    self.newest_dir = f
        else:
            self.newest_dir = self.export_driver_dir
    def check_release_chipinfo(self):
        self.log.info("开始检查导出芯片与正式版本芯片的数据库是否相同")
        export_record_list = self.export_db_file.get_chip_record_list()

        for export_record in export_record_list:
            release_record = self.release_db_file.get_chip_record_list(chip_name=export_record[0],adapter_name=export_record[1],manu_name=export_record[2])
            if (len(release_record) == 1) and (export_record != release_record[0]):
                self.log.info("导出数据记录: %s" % (export_record))
                self.log.info("正式版本数据记录: %s" % (release_record))
                return False
        self.log.info("检查导出芯片与正式版本芯片的数据库结束")
        return True
    '''
    功能描述：根据release数据库和芯片数据判断是否是新芯片
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象
        str_chipinfo    string    包括驱动名称等的字符串  Eastsoft EC04 QFN20(3x3) ECxx.ko MUX64.jbc QFN20(3x3)-S13         
    返回值：
        True/False
        如果手工指定： 'modify'返回True，‘add’返回False
        如果在数据库中找到返回True，没有找到返加False
    '''
        
def isexist(export_driver,str_chipinfo):
    if export_driver.driver_type == 'modify':
        return True
    elif export_driver.driver_type == 'add' : 
        return False
    else:
        l_chipinfo = str_chipinfo.split()
        d_chipinfo = {
        'chipname': l_chipinfo[1],
        'package':l_chipinfo[2],
        'kofile':l_chipinfo[3],
        'adapter':l_chipinfo[5]
        }
        return export_driver.release_db_file.driver_isexsit(d_chipinfo)


    '''
    功能描述：修改history文件
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象

    返回值：
        N/A
    '''

def modify_history(export_driver):
    export_driver.db_log_file.read_file()
    insert_context = export_driver.db_log_file.get_file_context("Adapter","===")
    #self.log.info(insert_context)
    for chipinfo in insert_context.split('\r\n'):
        if isexist(export_driver,chipinfo) :
            export_driver.history_file.insert_file_context(chipinfo,"3.GUI modifcations:")
        else:
            export_driver.history_file.insert_file_context(chipinfo,"2.Update Devices: ")
    '''
    功能描述：将修改的history信息写入history
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象

    返回值：
        N/A
    '''
def write_history(export_driver):
    export_driver.history_file.open('w+')
    export_driver.history_file.write_file()


    '''
    功能描述：拷贝驱动文件，包括KO,EXT_PARA,XML,CHK
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象

    返回值：
        N/A
    '''
def copy_driver_file(export_driver,path = ''):
    bin_dir =  os.path.join(export_driver.get_export_path() , 'bin/')
    if path != '' :
        bin_dir = path
    else:
        bin_dir =  os.path.join(export_driver.get_export_path() , 'bin/')
    num = 1
    if (export_driver.get_driver_type() == 'modify') or  (export_driver.get_driver_type() == 'add' and export_driver.export_driver_dir == export_driver.newest_dir)  :
        export_driver.log.info('开始拷贝')
        filename = os.path.join(bin_dir,export_driver.ko_file.get_file_name())
        export_driver.ko_file.copy_file(bin_dir)
        Q_ko_para_file=os.path.join(export_driver.get_driver_path(),export_driver.ko_file.get_file_name())
        Q_ko_para_file_MD5 =file.File({'filename':Q_ko_para_file})
        mm=Q_ko_para_file_MD5.get_file_md5()
        ko_para_file_name_MD5 = file.File(
            {'filename': os.path.join(bin_dir, export_driver.ko_file.get_file_name())})
        nn = ko_para_file_name_MD5.get_file_md5()
        assert mm == nn, "ko文件拷贝完必须和拷贝前保持一致"

        if os.path.isfile(os.path.join(export_driver.get_driver_path(),export_driver.ext_para_file.get_file_name())):
            export_driver.ext_para_file.copy_file(bin_dir)
            Q_ext_para_file=os.path.join(export_driver.get_driver_path(),export_driver.ext_para_file.get_file_name())
            Q_ext_para_file_MD5=file.File({'filename':Q_ext_para_file})
            bb=Q_ext_para_file_MD5.get_file_md5()
            ext_para_file_name_MD5 = file.File(
                {'filename': os.path.join(bin_dir, export_driver.ext_para_file.get_file_name())})
            aa=ext_para_file_name_MD5.get_file_md5()
            # print(ext_para_file_name_MD5)
            # export_driver.log.info("%s MD5值文件不存在！！！" % (aa))
            num += 1
            assert aa==bb,"extend文件拷贝完必须和拷贝前保持一致"
        else:
            export_driver.log.info("%s 文件不存在！！！"%(export_driver.ext_para_file.get_file_name()))
        if os.path.isfile(os.path.join(export_driver.get_driver_path(),export_driver.chk_file.get_file_name())):
            export_driver.chk_file.copy_file(bin_dir)
            Q_chk_para_file=os.path.join(export_driver.get_driver_path(),export_driver.chk_file.get_file_name())
            Q_chk_para_file_MD5=file.File({'filename':Q_chk_para_file})
            cc=Q_chk_para_file_MD5.get_file_md5()
            chk_para_file_name_MD5 = file.File(
                {'filename': os.path.join(bin_dir, export_driver.chk_file.get_file_name())})
            dd = chk_para_file_name_MD5.get_file_md5()
            assert cc==dd,"chk文件拷贝完必须和拷贝前保持一致"
            # print(ext_para_file_name_MD5)
            # export_driver.log.info("%s MD5值文件不存在！！！" % (aa))
            num += 1
        else:
            export_driver.log.info("%s 文件不存在！！！"%(export_driver.chk_file.get_file_name()))
        if os.path.isfile(os.path.join(export_driver.get_driver_path(),export_driver.xml_file.get_file_name())):
            export_driver.xml_file.copy_file(bin_dir)
            Q_xml_para_file = os.path.join(export_driver.get_driver_path(), export_driver.xml_file.get_file_name())
            Q_xml_para_file_MD5 = file.File({'filename': Q_xml_para_file})
            ee = Q_xml_para_file_MD5.get_file_md5()
            chk_para_file_name_MD5 = file.File(
                {'filename': os.path.join(bin_dir, export_driver.xml_file.get_file_name())})
            ff = chk_para_file_name_MD5.get_file_md5()
            assert ee == ff, "chk文件拷贝完必须和拷贝前保持一致"
            num += 1
        else:
            export_driver.log.info("%s 文件不存在！！！"%(export_driver.xml_file.get_file_name()))
        # export_driver.get_jbc_list()
        # export_driver.copy_jbcs()
        # 检查当前路径是否有文件
        files = os.listdir(bin_dir)
        export_driver.log.info('当前驱动文件数量为：%s' % (len(files)))
        if len(files) != num and export_driver.get_driver_type() == 'add':
            export_driver.log.info('%s 驱动拷贝失败' % (export_driver.ko_file.get_file_name()))
            export_driver.fail_drivers.append(export_driver.ko_file.get_file_name())
        else:
            export_driver.sucess_count += 1
        # else:
        #     dst_ko = ko({"filename":filename})
        #     dst_ko.open()
        #     dst_ko.read_file()
        #     if (export_driver.ko_file.get_svn_number() > dst_ko.get_svn_number()):
        #         export_driver.ko_file.copy_file(bin_dir)
        #         if os.path.isfile(export_driver.ext_para_file.get_file_name()):
        #             export_driver.ext_para_file.copy_file(bin_dir)
        #         if os.path.isfile(export_driver.chk_file.get_file_name()):
        #             export_driver.chk_file.copy_file(bin_dir)
        #         if os.path.isfile(export_driver.xml_file.get_file_name()):
        #             export_driver.xml_file.copy_file(bin_dir)
        #         # export_driver.get_jbc_list()
        #         # export_driver.copy_jbcs()
        #         export_driver.sucess_count += 1
        #     else:
        #         self.log.info("Q盘驱动更新，请确认是否覆盖，如果覆盖请删除Q盘驱动！！！")
        export_driver.same_ko_dir.remove(export_driver.newest_dir)
        if (export_driver.get_driver_type() == 'add') and (len(export_driver.same_ko_dir) != 0) :
            export_driver.log.info('当前驱动存在旧的驱动删除文件')
            for path in export_driver.same_ko_dir:
                export_driver.log.info('删除驱动的路径为：%s' % (path))
                delete_old_ko_file(export_driver,os.path.join(export_driver.export_path,path))

    else:
        export_driver.log.info("当前驱动版本不是最新,无需拷贝驱动！！！")
def delete_old_ko_file(export_driver,path = ''):
    os.chdir(os.path.join(path,'bin'))
    files = os.listdir()
    for f in files:
        f = os.path.join(path,'bin',f)
        export_driver.log.info('删除文件：'+ f)
        if os.path.isfile(f) and '.jbc' not in f :
            os.remove(f)

    '''
    功能描述：拷贝数据库文件和日志文件
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象

    返回值：
        N/A
    '''    
def copy_db_file(export_driver):
    export_driver.export_db_file.copy_file(export_driver.get_export_path())
    export_driver.db_log_file.copy_file(export_driver.get_export_path())
    files=os.listdir(export_driver.get_export_path())
    has_valid_files  = any(file.endswith(('.dbs', '.log')) for file in files)
    if not has_valid_files:
        raise ValueError("列表不包含 .dbs 或 .log 文件，请检查！")
    else:
        print("列表正确，包含 .dbs 和 .log 文件。")
    '''
    功能描述：导出驱动和帮助文件
    参数：
        参数名           类型         描述                        举例
        export_driver   Driver    Driver对象

    返回值：
        N/A
    '''  
def export_driver_file(export_driver,path = ""):
    driver_fullname_list = []
    html_name_list = []
    export_driver.get_jbc_list()
    for index,driver_file_fullname in enumerate(export_driver.html_file_names['driver_names']):
        #判断驱动文件是否导出过
        if driver_file_fullname not in driver_fullname_list :
            #设置源驱动文件
            export_driver.set_driver_file(export_driver.get_driver_path() + driver_file_fullname)
            if not is_new_version(export_driver.ko_file,export_driver.get_export_path()):
                export_driver.fail_drivers.append(export_driver.ko_file)
                return False
            if export_driver.driver_type == 'add':
                #创建发布驱动目录
                export_driver.create_export_dir()
                #拷贝数据库和日志文件到导出驱动目录
                copy_db_file(export_driver)
            
            #拷贝驱动文件到导出驱动目录
            copy_driver_file(export_driver,path)
           
            driver_fullname_list.append(driver_file_fullname)
        #获取html文件列表
        html_file = export_driver.html_file_names['html_files'][index]

        if html_file != '' :
            #判断HTML是否已经导出，注：如果不同的驱动相同的帮助文件会漏导出
            if html_file not in  html_name_list :
                #设置HTML文件
               export_driver.set_html_file(export_driver.help_html_path + html_file)
               dst_path = export_driver.get_html_path()

               #拷贝帮助html文件到导出驱动的帮助目录
               export_driver.html_file.copy_file(dst_path)
               #拷贝帮助docx文件到导出驱动的帮助目录
               export_driver.doc_file.copy_file(dst_path)
               html_name_list.append(html_file)
            clean_old_html(export_driver, html_file)


def clean_old_html(export_driver,html_file):
    AA = export_driver.export_path
    GG = os.walk(export_driver.export_path)
    SS = os.listdir(export_driver.export_path)
    for dir in os.listdir(export_driver.export_path):
        help_path = os.path.join(export_driver.export_path,dir,"help")
        # 如果是当前文件夹 跳过
        if not os.path.exists(help_path):
            continue
        if os.path.samefile(help_path, export_driver.get_html_path()):
            continue
        html_path = os.path.join(export_driver.export_path,dir,"help",html_file)
        if os.path.exists(html_path):
            try:
                deldir(help_path)
                export_driver.log.info(f"已删除旧版 HTML: {html_path}")
            except Exception as e:
                export_driver.log.error(f"删除失败 {html_path}: {str(e)}")

def deldir(directory):
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)  # 删除文件或符号链接
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)  # 删除整个目录
        except Exception as e:
            print(f'未能删除 {file_path}. 原因: {e}')
def is_new_version(src_ko,path):
    ko_file = src_ko.get_file_name()
    dst_ko_name = os.path.join(path,ko_file)
    if not os.path.exists(dst_ko_name):
        return True
    else:
        dst_ko = ko({'filename':dst_ko_name})
        src_ko_svn = get_svn_number(src_ko)
        dst_ko_svn = get_svn_number(dst_ko)
        if (src_ko_svn and dst_ko_svn) and (int(src_ko_svn) <= int(dst_ko_svn)):
            print(src_ko.get_file_path() + "/"+ ko_file + " src_ko_svn:" + dst_ko_svn)
            print(dst_ko.get_file_path() + "/"+ ko_file + " dst_ko_svn:" + dst_ko_svn)
            #copyfaildrivers.append(old_ko_name)
            print (ko_file + "拷贝失败！新KO文件的SVN号小于等于目标路径下KO的SVN，请检查！！！")
            return False
        elif ((not src_ko_svn) and dst_ko_svn):
            #copyfaildrivers.append(old_ko_name)
            print (src_ko_svn.get_file_path() + ko_file + "拷贝失败！新KO文件的获取不到SVN,旧的KO文件获取到了SVN号，请检查！！！")
            return False
        else:
            print(src_ko.get_file_path() + "/"+ ko_file + "src_ko_svn:" + str(src_ko_svn))
            print(dst_ko.get_file_path() + "/"+ ko_file + "dst_ko_svn:" + str(dst_ko_svn))
            #sucess_count += 1
            return True
            #copy_ko_file(src_ko_f,dst_dir)
def get_svn_number(ko_file):
    ko_file.open('rb')
    ko_file.read_file()
    ko_svn = ko_file.get_svn_number()
    return ko_svn       
main()


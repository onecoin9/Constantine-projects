#ifndef AC_FILEUTIL_H
#define AC_FILEUTIL_H

#include <string>
#include "ACLib.h"

/* 文件的应用函数集合 */
BEGIN_AC_FILESYSTEM


/**
 * @brief 获取当前执行路径
 * @param void
 * @return 返回说明
 *     -std::string	当前程序路径
 */
AC_COMMONLIB_API std::string GetCurrentPath();



/**
 * @brief 给定一个文件全路径，返回文件名称，不包含后缀
 * @param[IN] file_path		文件路径
 * @return 返回说明
 *     -std::string	文件的名称
 */
AC_COMMONLIB_API std::string GetFileName(IN const std::string& file_path);



/**
 * @brief 给定一个文件全路径，返回该文件的目录
 * @param[IN] file_path		文件路径
 * @return 返回说明
 *     -std::string	文件的目录
 */
AC_COMMONLIB_API std::string GetFilePath(IN const std::string& file_path);



/**
 * @brief 判断文件是否存
 * @param[IN] file_path		文件路径
 * @return 返回说明
 *     -true	存在
 *     -false	不存在
 */
AC_COMMONLIB_API bool IsFileExist(IN const std::string& file_path);



/**
 * @brief 创建文件
 * @param[IN] file_path		若文件路径不存在，则创建文件，若存在则不创建
 * @return 返回说明
 *     -true	创建成功，存在也为成功
 *     -false	创建失败
 */
AC_COMMONLIB_API bool CreateFile(IN const std::string& file_path);



/**
 * @brief 删除文件
 * @param[IN] file_path		若文件路径不存在，则创建文件，若存在则不创建
 * @return 返回说明
 *     -true	创建成功，存在也为成功
 *     -false	创建失败
 */
AC_COMMONLIB_API bool DeleteFile(IN const std::string& file_path);



/**
 * @brief 拷贝文件，目的文件存在，则直接覆盖
 * @param[IN] fold		被拷贝的文件路径
 * @param[IN] fnew		拷贝至的文件路径
 * @return 返回说明
 *     -true	拷贝成功
 *     -false	拷贝失败
 */
AC_COMMONLIB_API bool CopyFile(IN const std::string& fold, IN const std::string& fnew);



/**
 * @brief 移动文件，目的文件存在，则直接覆盖
 * @param[IN] fold		被移动的文件路径
 * @param[IN] fnew		移动至的文件路径
 * @return 返回说明
 *     -true	移动成功
 *     -false	移动失败
 */
AC_COMMONLIB_API bool MoveFile(IN const std::string& fold, IN const std::string& fnew);


END_AC_FILESYSTEM

#endif
#ifndef AC_TRANSLATIONCTRL_H
#define AC_TRANSLATIONCTRL_H
#include <string>
using std::string;

void InitTranslation(const std::string& cur_path);

string TranslateErrMsg(int err_code, bool* ret = nullptr);

#endif

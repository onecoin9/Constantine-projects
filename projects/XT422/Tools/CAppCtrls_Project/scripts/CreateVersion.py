import time
struct = time.localtime()
strt = time.strftime('%Y-%m-%d %H:%M:%S')
strv="V1.39"

strversion="#define SOFT_VERSION (\"{version}\")\n".format(version=strv)
strdate="#define SOFT_DATE (\"{date}\")\n".format(date=strt)

strInfo="Version: {s1}, Date: {s2}".format(s1=strv,s2=strt)

f1=open("../include/Version.h",'w',encoding='UTF-8')
f1.write('#pragma once\n\n')
f1.write(strversion)
f1.write(strdate)
f1.flush()
f1.close()
print(strInfo)

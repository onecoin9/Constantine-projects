#ifndef _ICRCCHECK_H_
#define _ICRCCHECK_H_

#include "ACTypes.h"

class ICrcCheck {
public:
	virtual void ReInit()=0;
	virtual std::string GetName()
	{
		return std::string("None");
	};
	/// <summary>
	/// 传入计算的Buffer和字节数，可以反复被调用
	/// </summary>
	/// <param name="Buf">请求计算校验值的Buffer</param>
	/// <param name="Size">计算的字节数</param>
	virtual void CalcSubRoutine(uint8_t* Buf, uint32_t Size) = 0;
	/// <summary>
	/// 获取结果，最后被调用，传入的pChecksum空间由外部分配，函数内部填充，需要注意不同的CRC传入的字节数据
	/// </summary>
	/// <param name="pChecksum">校验值被填充的地方</param>
	/// <param name="Size">允许填充的字节数</param>
	virtual void GetChecksum(uint8_t* pChecksum,int32_t Size)=0;
};


#endif 
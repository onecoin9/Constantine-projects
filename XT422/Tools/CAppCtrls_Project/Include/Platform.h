#ifndef _PLATFORM_H_
#define _PLATFORM_H_


#define SysMalloc(_Size) malloc(_Size)
#define SysSafeFree(_Ptr) do{if(_Ptr)free(_Ptr);_Ptr=NULL;}while(0)

#endif 
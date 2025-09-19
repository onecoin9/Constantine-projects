#ifndef __XT_TRIM_PARAM_H__
#define __XT_TRIM_PARAM_H__

#include <stdint.h>

#pragma pack(1)

typedef struct{
    /*params boundaries are valid: [min, max]! */
    uint32_t icc_min;//工作电流下限   
    uint32_t icc_max;//工作电流上限 uA

    /*dc trim limitation */
    uint16_t dc_basic_min;   //tirm前直流电压最小值
    uint16_t dc_basic_max;   //tirm前直流电压最大值
    uint16_t dc_p2p_max;     //trim前直流峰峰值mV

    uint16_t dc_trim_min;    //trim后的直流电压最小值
    uint16_t dc_trim_max;    //trim后的直流电压最大值
    uint16_t dc_trim_best;   //trim后的直流电压最佳值mV

    /*ac limitation */
    uint32_t ac_trim_min;   //trim后周期脉冲频率最小值
    uint32_t ac_trim_max;   //trim后周期脉冲频率最大值
    uint32_t ac_trim_best;  //trim后周期脉冲频率最佳值Hz

    uint16_t ac_avg_min;    //正弦交流电压平均值最小值
    uint16_t ac_avg_max;    //正弦交流电压平均值最大值
    uint16_t ac_p2p_min;    //正弦交流电压峰峰值最小值
    uint16_t ac_p2p_max;    //正弦交流电压峰峰值最大值mV
    uint32_t ac_freq_min;   //正弦交流频率最小值
    uint32_t ac_freq_max;   //正弦交流频率最大值Hz
}xt_trim_param_t;

typedef struct
{
    uint8_t addr;//寄存器地址
    uint8_t start_bit;//起始位
    uint8_t width_bit;//长度，bit
    uint8_t write_back;//是否写回初始值
}xt_reg_bit_t;//寄存器定义

typedef struct{
    uint8_t dc6;//在输出控制寄存器中写入该值来输出锁相环直流电压（对应表中的拟写入值）
    uint8_t dc5;//在输出控制寄存器中写入该值来输出驱动直流电源
    uint8_t ac27;//在输出控制寄存器中写入该值来输出周期脉冲电压
    uint8_t ac4;//在输出控制寄存器中写入该值来输出正弦交流电
}xt_output_ctrl_value_t;

typedef union{
    struct{
        uint32_t trim_dc6:1;//是否进行直流电源trim（dc6 对应表格中拟写入值是6的那一项，以下类似）
        uint32_t trim_dc5:1;
        uint32_t trim_ac27:1;
        uint32_t trim_ac4:1;
        uint32_t trim_eoc:1;//有的芯片需要写EOC位（end of config）
        uint32_t res:27;
    };
    uint32_t value;
}xt_trim_en_t;

typedef struct{
    xt_reg_bit_t output_ctrl;   //trim control register， 输出控制寄存器
    xt_reg_bit_t dc_trim;       //trim DC set register， 直流trim寄存器
    xt_reg_bit_t ac_en;         //trim AC enable register， 周期脉冲输出使能寄存器
    xt_reg_bit_t ac_trim;       //trim AC set register，周期脉冲trim寄存器
    xt_reg_bit_t eoc;           //trim EOC register
}xt_trim_reg_t;

typedef struct{
    uint32_t power_on_delay_ms;//启动延时
    uint32_t t1_dc_stable_ms;//dc输出稳定延时
    uint32_t t1_ac_stable_ms;//ac输出稳定延时
    uint32_t delay_after_program_ms;//发送烧录指令后延时
    uint32_t reg_operation_delay_us;//寄存器操作间隔延时
}xt_delay_t;

//最终下发这个结构体
typedef struct{
    xt_trim_en_t t1_trim_en;        //trim step enable
    xt_trim_reg_t t1_trim_regs;      //trim regs & bits
    xt_output_ctrl_value_t t1_output_ctrl_value;
    xt_trim_param_t t1_trim_params;    //limitations and trim target for t1
    xt_delay_t delay_set;
}xt_trim_t;

#pragma pack()

#endif

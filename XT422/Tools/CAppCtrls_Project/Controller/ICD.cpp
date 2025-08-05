#include "..\Include\ICD.h"
#include "ICD.h"


/// <summary>
/// 将命令转为对应的CmdID
/// </summary>
/// <param name="strCmd"></param>
/// <returns></returns>

eSubCmdID CICD::TransStrCmd2CmdID(QString strCmd)
{
    eSubCmdID strCmdID = (eSubCmdID)0;
    if (strCmd == "FIBER2SSD") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_FIBER2SSD;
    }
    else if (strCmd == "FIBER2SKT") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_FIBER2SKT;
    }
    else if (strCmd == "FIBER2DDR") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_FIBER2DDR;
    }
    else if (strCmd == "SSD2FIBER") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_SSD2FIBER;
    }
    else if (strCmd == "SSD2SKT") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_SSD2SKT;
    }
    else if (strCmd == "SKT2FIBER") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_SKT2FIBER;
    }
    else if (strCmd == "DDR2FIBER") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_DDR2FIBER;
    }
    else if (strCmd == "SSD2DDR") {
        strCmdID = eSubCmdID::SubCmd_DataTrans_SSD2DDR;
    }
    else if (strCmd == "DDR2SSD"){
        strCmdID = eSubCmdID::SubCmd_DataTrans_DDR2SSD;
    }
    return strCmdID;
}

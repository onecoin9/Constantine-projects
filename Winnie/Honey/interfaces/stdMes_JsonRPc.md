# Plan 
- [ ] AG06的XT422代码环境添加Stdmes.DLL需要的环境；
- [x] Stdmes.dll调试环境搭建；
- [ ] 修改StdMes.dll，加入Json_RPC数据交互方式，通过修改MesApp配置中`MultiAprog路径`（`MultiAprog.exe路径` 或 `APS.exe路径`）来选择唤醒程序；
- [x] 在MesApp程序中进行stdMes调试；
- [x] StdMes_compatiable优化恢复为O2



## StdMES的JSON_RPC设计

- [x] TCP作为通信协议，长连接（或短连接进行命令交互），服务端异常退出检测；
- [x] APS.exe启动后，stdMes才允许进行tcp连接请求；APS.exe程序退出后，stdMes断开tcp；
- [x] JSON_RPC客户端注册回调函数，对Server端的主动上报进行响应；
- [x] 将MesInfc关于dll的接口抽象出来，派生得到ACSvcInfc类的处理，暂时保持原有逻辑不变，派生JsonRpcMesInfc类实现JsonRpc的处理，后续在抽象类复用相同代码；



# Change

- [x] MesStartService：AG06放stdMes.dll的目录下需要有一个MultiCfg.ini文件，参考MultiAprog.exe
- [x] stdMes.dll库调试：直接双击打开MesApp.exe程序（不在vs中打开），然后打开stdmes.dll的vs工程，点击`调试 -> 附加到进程 -> 选择MesApp.exe`，stdMes.dll编译要把优化禁用，要不然有一些地方断点打不到；



# <font color='red'>Notes</font>

- [x] 注意修改库不能影响DLL的二进制兼容性（不能修改函数接口或者新增接口在类的位置）



# 配置文件

```tex
└── Build/
    ├── data/
    │   └── ErrCodeMsg.ini	
    ├── mes/
    │   ├── AdpEnConfig.json
    │   ├── ExcelMes.ini
    │   ├── shinry.ini
    │   ├── SKTSwapConfig.json
    │   └── StdMes.ini
    ├── MultiCfg.ini
    ├── StdMES.dll
    ├── StdMES.exp
    ├── StdMES.lib
    └── StdMES.pdb
```



# Json RPC接口

## 专业名词

| 专业名词       | 含义                                              |
| -------------- | ------------------------------------------------- |
| SKT （socket） | 站点的座子                                        |
| BPU            | 座子所属的母座，比如 SKT0和SKT1座子属于BPU0母座上 |
|                |                                                   |



## InitSiteEnv

### Request

```json
{
    "jsonrpc": "2.0",
    "method": "InitSiteEnv",
    "params": {
    },
    "id": "001"
}
```



### ACK

```json
{
    "jsonrpc": "2.0",
    "result": {
        "message": "InitSiteEnv accepted."
    },
    "id": "001"
}
```



### Response(Async)

```Json
{
   "jsonrpc" : "2.0",
   "method" : "SetInitResult",
   "params" : {
       "siteCnt" : "2",
       "siteResult" : [
            {
                "DevSN" : "A06U24120700013",
                "ResultMsg" : "Success"
            },
            {
                "DevSN" : "A06U24120700013",
                "ResultMsg" : "Failed"
            }
       ]
   }
}
```



## GetProjectInfo

### Request

```Json
{
   "id" : 3,
   "jsonrpc" : "2.0",
   "method" : "GetProjectInfo",
   "params" : null
}
```



### ACK

```Json
{
   "id" : 3,
   "jsonrpc" : "2.0",
   "result" : {
      "message" : "Project info retrieved.",
      "projects" : [
         {
            "key" : "D:/COMPANY_PRO/AP8000V2/trunk/AIPE/Build/task/task.eapr",
            "pair_first_string" : "0xffff"
         }
      ]
   }
}
```





## GetProjectInfoExt

### Request

```Json
{
   "id" : 4,
   "jsonrpc" : "2.0",
   "method" : "GetProjectInfoExt",
   "params" : {
      "project_url" : "D:/COMPANY_PRO/AP8000V2/trunk/AIPE/Build/task/task.eapr"
   }
}
```





### ACK

```json
 {
   "id" : 4,
   "jsonrpc" : "2.0",
   "result" : {
      "message" : "GetProjectInfoExt",
      "projects" : {
         "AdpName" : "BGA153(11.5x13)-P050-G16-01",
         "CheckSum" : "0X00000400",
         "SocketNum" : 16,
         "Type" : "eMMC",
         "doCmdSequenceArray" : [
            {
               "CmdID" : "1801",
               "CmdRun" : "Erase",
               "CmdSequences" : [
                  {
                     "ID" : "801",
                     "Name" : "Erase"
                  },
                  {
                     "ID" : "803",
                     "Name" : "BlankCheck"
                  }
               ],
               "CmdSequencesGroupCnt" : 2
            },
            {
               "CmdID" : "1803",
               "CmdRun" : "Blank",
               "CmdSequences" : [
                  {
                     "ID" : "803",
                     "Name" : "BlankCheck"
                  }
               ],
               "CmdSequencesGroupCnt" : 1
            },
            {
               "CmdID" : "1800",
               "CmdRun" : "Program",
               "CmdSequences" : [
                  {
                     "ID" : "807",
                     "Name" : "Erase If BlankCheck Failed"
                  },
                  {
                     "ID" : "803",
                     "Name" : "BlankCheck"
                  },
                  {
                     "ID" : "800",
                     "Name" : "Program"
                  },
                  {
                     "ID" : "802",
                     "Name" : "Verify"
                  }
               ],
               "CmdSequencesGroupCnt" : 4
            },
            {
               "CmdID" : "1802",
               "CmdRun" : "Verify",
               "CmdSequences" : [
                  {
                     "ID" : "802",
                     "Name" : "Verify"
                  }
               ],
               "CmdSequencesGroupCnt" : 1
            },
            {
               "CmdID" : "1804",
               "CmdRun" : "Secure",
               "CmdSequences" : [
                  {
                     "ID" : "804",
                     "Name" : "Secure"
                  }
               ],
               "CmdSequencesGroupCnt" : 1
            },
            {
               "CmdID" : "1806",
               "CmdRun" : "Read",
               "CmdSequences" : [
                  {
                     "ID" : "806",
                     "Name" : "Read"
                  }
               ],
               "CmdSequencesGroupCnt" : 1
            },
            {
               "CmdID" : "1901",
               "CmdRun" : "Self",
               "CmdSequences" : [],
               "CmdSequencesGroupCnt" : 0
            }
         ],
         "pro_chipdata" : {
            "bDebug" : true,
            "bottomBoard" : "",
            "chipAdapter" : "BGA153(11.5x13)-P050-G16-01",
            "chipAdapter2" : "NULL",
            "chipAdapter3" : "NULL",
            "chipAdapterID" : "48A9",
            "chipAlgoFile" : "Drv-eMMC.drv",
            "chipAppFile" : "burn.app",
            "chipBufferSize" : 0,
            "chipBufferSizeHigh" : 0,
            "chipChipInfo" : "bbbb.html",
            "chipCurSbk" : "NULL",
            "chipDrvParam" : 1,
            "chipFPGAFile" : "MUX64.jbc",
            "chipFPGAFile2" : "G8eMMC-TGeneric180.jbc",
            "chipHelpFile" : "NULL",
            "chipId" : "0",
            "chipIdACXML" : "",
            "chipModifyInfo" : "",
            "chipMstkoFile" : "Mst-eMMC.drv",
            "chipName" : "eMMC-TGeneric-1.8",
            "chipOperCfgJson" : "{\"baseOper\":{\"blank\":true,\"blockProg\":true,\"erase\":true,\"function\":false,\"illegalBit\":false,\"prog\":true,\"read\":true,\"secure\":true,\"verify\":true},\"bitsOper\":{\"bit12\":false,\"bit16\":false,\"bit4\":false,\"bit8\":false},\"checkSumOper\":{\"crc16\":false,\"crc32\":false,\"sum\":false,\"wordSum\":false},\"fileLoadOper\":{\"bigEndian\":false,\"wordAddress\":false},\"otherOper\":{\"EEPROM\":false,\"IDCheck\":false,\"addressRelocate\":false,\"compare\":false,\"emptyBuffer\":false,\"enableSN\":false,\"insection\":false,\"loopFun\":false,\"masterCopy\":false,\"online\":false,\"pin\":false,\"protect\":false,\"unTest\":true,\"vccVerify\":true}}",
            "chipOperateConfigMask" : 0,
            "chipPackage" : "BGA153(11.5x13)",
            "chipProgType" : 83886083,
            "chipSbkId" : 0,
            "chipSectorSize" : 1048576,
            "chipStatus" : "",
            "chipType" : "eMMC",
            "manufacture" : "AlleMMCManu",
            "nVersion" : 0
         },
         "pro_url" : "D:/COMPANY_PRO/AP8000V2/trunk/AIPE/Build/task/task.eapr"
      }
   }
}
```





## SiteScanAndConnect

### Request

#### 1. 连接所有站点

```Json
{
   "id" : 1,
   "jsonrpc" : "2.0",
   "method" : "SiteScanAndConnect",
   "params" : null
}

```



#### 2. 连接指定站点

```ini
#在APS.exe目录的mes/stdmes.ini文件中修改连接指定站点

[config]
SitesGroup= Site03 | Site02 | Site01
```

```Json
{
   "id" : 1,
   "jsonrpc" : "2.0",
   "method" : "SiteScanAndConnect",
   "params" : {
      "siteList" : [
         {
            "siteAlias" : "Site03"
         },
         {
            "siteAlias" : "Site 02"
         },
         {
            "siteAlias" : "Site 01"
         }
      ]
   }
}
```





### ACK

```Json
{
   "id" : 1,
   "jsonrpc" : "2.0",
   "result" : {
      "message" : "Scan initiated successfully. Device discovery notifications will be sent."
   }
}
```



### Response(Async)

```json
 {
   "jsonrpc" : "2.0",
   "method" : "DeviceDiscovered",
   "params" : {
      "scanDevList" : [
         {
            "device" : {
               "chainID" : 1,
               "dpsFpgaVersion" : "0x2002285A",
               "dpsFwVersion" : "V2.2.0006A",
               "firmwareVersion" : "2.02.031",
               "firmwareVersionDate" : "20250111",
               "fpgaLocation" : "Normal",
               "fpgaVersion" : "0x2030121A",
               "ip" : "192.168.20.223",
               "isLastHop" : true,
               "linkNum" : -1,
               "mac" : "00:3A:22:00:01:02",
               "mainBoardInfo" : {
                  "hardwareOEM" : "ACVIEW",
                  "hardwareSN" : "A06U24120700012",
                  "hardwareUID" : "CCE3DF00A00C0672",
                  "hardwareVersion" : "0100-010000-010000-010000"
               },
               "muAppVersion" : "2.02.029",
               "muAppVersionDate" : "20240111",
               "muLocation" : "Normal",
               "port" : "8080",
               "siteAlias" : "Site03"
            },
            "ipHop" : "192.168.20.223:0"
         }
      ]
   }
}
```



## GetAllSitesAdpEn

### Request

```Json
{
   "id" : 6,
   "jsonrpc" : "2.0",
   "method" : "GetAllSitesAdpEn",
   "params" : {}
}
```



### ACK

```Json
{
   "id" : 2,
   "jsonrpc" : "2.0",
   "result" : {
      "message" : "GetAllSitesAdpEn success."
   }
}
```



### Response(ASync)

```json
{
   "jsonrpc" : "2.0",
   "method" : "GetAllSitesAdpEnResult",
   "params" : {
      "AdpEnMap" : [
         {
            "AdpEn" : 65535,
            "DevSN" : "A06U24120700012"
         }
      ]
   }
}
```



## LoadProject

### Request

```Json
 {
   "id" : 2,
   "jsonrpc" : "2.0",
   "method" : "LoadProject",
   "params" : {
      "path" : "C:\\Users\\acrov\\Desktop\\新建文件夹\task.actask"
   }
}
}
```





### ACK

```Json
 {
   "id" : 2,
   "jsonrpc" : "2.0",
   "result" : {
      "message" : "LoadProject"
   }
}
```



### Response(Async)

```Json
{
   "jsonrpc" : "2.0",
   "method": "LoadProjectResult",
   "params" : {
      "data" : "success"
   }
}
```



## DoCustom

### Request

```Json
{
    "jsonrpc": "2.0",
    "method": "DoCustom",
    "params": {
        "DevSN": "A06U24120700012",
        "PortID": 0,
        "CmdFlag": 0,
        "CmdID": 0x417,
        "SKTEn": 65535,
        "BPUID": 8,
        "data": {
        }
    },
    "id": "001"
}
```



### Ack

```Json
{
    "jsonrpc": "2.0",
    "result": {
        "message": "DoCustom request accepted.",
    },
    "id": "001"
}
```



### Response(Async)

```json
{
   "jsonrpc" : "2.0",
   "method" : "setDoCustomResult",
   "params" : {
       "DevSN": "A06U24120700012",
       "cmdID" : 0x417,
       "data" : {}
   }
}
```



### 子指令集

#### GetSktInfo (CmdID = 0x436)

##### Request

```
查询BPU的数量，3 = 0000 0011
每一位表示一个BPU，一个BPU有两个SKT
1表示启用，0表示未启用
Bit0~Bit7 = BPU0~BPU7
```

```Json
 {
   "id" : 6,
   "jsonrpc" : "2.0",
   "method" : "DoCustom",
   "params" : {
      "BPUID" : 8,
      "CmdFlag" : 0,
      "CmdID" : 1078,
      "DevSN" : "A06U24120700013",
      "PortID" : 0,
      "SKTEn" : 65536,
      "data" : {
         "BPUEn" : 255
      }
   }
}
```



##### Reponse(Async)

```Json
{
    "PrjSktId" : 32783, 
    "SiteInfo" : [
		{ 
            "AdapterCnt" : 8, 
          	"SiteSN" : "A80U17060100757",
			"SlotInfo" : [
				{ "CurrentCnt" : 1151, 
                 "FailCnt" : 48, 
                 "ID" : 34810, 
                 "Index" : 1, 
                 "LicenseFlag" : 0, 
                 "LimitedCnt" : 60100, 
                 "SN" : "7D0000032EA7C148"
				},
                { "CurrentCnt" : 77993, 
                 "FailCnt" : 1627, 
                 "ID" : 32783, 
                 "Index" : , 
                 "LicenseFlag" : 0, 
                 "LimitedCnt" : 160120, 
                 "SN" : "E60000000EC34448"
				}
                ,
                {...}
			]
		},
        {...}
	]
}
```





## DoJob

### Request

```Json
{
    "jsonrpc": "2.0",
    "method": "DoJob",
    "params": {
        "strIP": "192.168.1.100",
        "nHopNum": 1,
        "PortID": 0,
        "CmdFlag": 0,
        "CmdID": 0x417,
        "SKTEn": 1,
        "BPUID": 8,
        "operation": "program",
        "docmdSeqJson": {
        }
    },
    "id": "001"
}
```



### Ack

```Json
{
    "jsonrpc": "2.0",
    "result": {
        "message": "DoJob request accepted.",
        "status": "accepted", 
        "taskId": "87654321-4321-4321-4321-cba987654321"
    },
    "id": "001"
}
```



### Response(Async)

```json
{
   "jsonrpc" : "2.0",
   "method" : "SetDoJobResult",
   "params" : {
       	"DevSN" : "A06U24120700013",
      	"cmd" : "Program",
       	"data" : {}
   }
   "DevSN" : "A06U24120700012",
   "strcmd" : "Program"
}
```



#### data

##### strcmd == "Program"

```Json
{
   "jsonrpc" : "2.0",
   "method" : "setDoJobResult",
   "params" : {
      "DevSN" : "A06U24120700013",
      "cmd" : "Program",
      "data" : {
         "AdpCnt" : 16,
         "AdpResultInfo" : [
            {
               "sktIdx" : 1,
               "status" : "Success"
            },
            {
               "sktIdx" : 2,
               "status" : "Success"
            },
            {
               "sktIdx" : 3,
               "status" : "Success"
            },
            {
               "sktIdx" : 4,
               "status" : "Success"
            },
            {
               "sktIdx" : 5,
               "status" : "Success"
            },
            {
               "sktIdx" : 6,
               "status" : "Success"
            },
            {
               "sktIdx" : 7,
               "status" : "Success"
            },
            {
               "sktIdx" : 8,
               "status" : "Success"
            },
            {
               "sktIdx" : 9,
               "status" : "Success"
            },
            {
               "sktIdx" : 10,
               "status" : "Success"
            },
            {
               "sktIdx" : 11,
               "status" : "Success"
            },
            {
               "sktIdx" : 12,
               "status" : "Success"
            },
            {
               "sktIdx" : 13,
               "status" : "Success"
            },
            {
               "sktIdx" : 14,
               "status" : "Success"
            },
            {
               "sktIdx" : 15,
               "status" : "Success"
            },
            {
               "sktIdx" : 16,
               "status" : "Success"
            }
         ]
      }
   }
}
```



# 服务端主动上报


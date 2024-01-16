# query-pdb-ex

query-pdb-ex是一个基于 [zouxianyu/query-pdb](https://github.com/zouxianyu/query-pdb) 的修改版。

修改的部分如下：

1. 增加了mongodb作为数据库缓存，减少后端io消耗。

2. 简化了请求接口，所有解析数据一次性全部返回，降低开发复杂度。
   
   

# 测试接口

测试服务器为  http://query-pdb.szdyg.cn/    https://query-pdb.szdyg.cn/

请求参数为

name：pdb的name

msdl：pdb的guid+pdb的age

query：查询的结构名称



### symbol 接口

发送POST 请求到 https://query-pdb.szdyg.cn/symbol

```json
{
    "name": "ntkrnlmp.pdb",
    "msdl": "8F0F3D677778391600F4EB2301FFC7A51",
    "query": [
        "KdpStub",
        "MmAccessFault",
        "xxxxxx"
    ]
}
```

返回

```json
{
    "KdpStub": 3773768,
    "MmAccessFault": 2454256
}
```

不存在的结构（xxx），服务器不会返回。



### struct接口

发送POST 请求到 https://query-pdb.szdyg.cn/struct

```json
{
    "name": "ntkrnlmp.pdb",
    "msdl": "8F0F3D677778391600F4EB2301FFC7A51",
    "query": [
        "_KPROCESS",
        "xxxxxx"
    ]
}
```



```json
{
    "_KPROCESS": {
        "ActiveGroupsMask": 636,
        "ActiveProcessors": 368,
        "AddressPolicy": 912,
        "Affinity": 80,
        "AutoAlignment": 632,
        "BasePriority": 640,
        "CacheIsolationEnabled": 632,
        "CheckStackExtents": 632,
        "ContextSwitches": 872,
        "CpuPartitionList": 1056,
        "CycleTime": 864,
        "DeepFreeze": 632,
        "DeepFreezeStartTime": 72,
        "DirectoryTableBase": 40,
        "DisableBoost": 632,
        "DisableQuantum": 632,
        "EndPadding": 1072,
        "ExtendedFeatureDisableMask": 1032,
        "Flags": 643,
        "FreezeCount": 888,
        "Header": 0,
        "IdealGlobalNode": 836,
        "IdealNode": 772,
        "IdealProcessor": 708,
        "InstrumentationCallback": 984,
        "KernelTime": 892,
        "KernelWaitTime": 1000,
        "LastRebalanceQpc": 1016,
        "MultiGroup": 632,
        "PerProcessorCycleTimes": 1024,
        "PpmPolicy": 632,
        "PrimaryGroup": 1040,
        "ProcessFlags": 632,
        "ProcessListEntry": 848,
        "ProcessLock": 64,
        "ProcessTimerDelay": 68,
        "ProfileListHead": 24,
        "QuantumReset": 641,
        "ReadyListHead": 344,
        "ReadyTime": 900,
        "ReservedFlags": 632,
        "SchedulingGroup": 880,
        "SecureState": 992,
        "Spare1": 838,
        "Spare2": 913,
        "Spare3": 1042,
        "StackCount": 840,
        "SwapListEntry": 360,
        "ThreadListHead": 48,
        "ThreadSeed": 644,
        "TimerVirtualization": 632,
        "UserCetLogging": 1048,
        "UserDirectoryTableBase": 904,
        "UserTime": 896,
        "UserWaitTime": 1008,
        "VaSpaceDeleted": 632,
        "Visited": 642
    }
}

```



### enum 接口

发送POST 请求到 https://query-pdb.szdyg.cn/enum



```json
{
    "name": "ntkrnlmp.pdb",
    "msdl": "8F0F3D677778391600F4EB2301FFC7A51",
    "query": [
        "_OBJECT_INFORMATION_CLASS",
        "xxxxxx"
    ]
}
```

返回

```json
{
    "_OBJECT_INFORMATION_CLASS": {
        "MaxObjectInfoClass": 7,
        "ObjectBasicInformation": 0,
        "ObjectHandleFlagInformation": 4,
        "ObjectNameInformation": 1,
        "ObjectSessionInformation": 5,
        "ObjectSessionObjectInformation": 6,
        "ObjectTypeInformation": 2,
        "ObjectTypesInformation": 3
    }
}
```





## 私有部署

请修改docker-compose.yml

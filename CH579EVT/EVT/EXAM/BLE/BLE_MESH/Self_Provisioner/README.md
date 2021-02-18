# 概述
这是一个通用配置者例子

## 关于MESH库
在这个例子中，使用BLE_MESH_PRV.LIB作为默认库。


|功能|使能|
|-|-|
|转发|Y|
|朋友|Y|
|低功耗|N|
|代理|N|
|配置通过广播|Y|
|配置通过连接|N|
|配置者|Y|
|客户端模型|Y|

提供本例的目的是以最基本的功能出发，快速了解其工作机制。

此例程默认自我入网，并配置为周期发布模式，当收到来自未配网的设备后，将配网该设备并
配置节点为订阅模式。

此例程可以与Generic OnOff Models结合使用。
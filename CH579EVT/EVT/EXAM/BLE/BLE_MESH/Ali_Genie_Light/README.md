# 概述

该例用于指导如何接入天猫精灵并实现配网控制功能

## 注意
    你可以使用本例子，来验证其功能。建议自行注册相关产品ID以防冲突。

## 功能概述
    设备处于未配网状态，并按照阿里定义的方式周期性向外广播数据。
    设备将在10分钟未配网后进入静默广播状态，并禁止配网。
    设备在配网后将上报本地状态，本地LED状态可通过按键触发，LED状态
    将会上报到精灵端。可通过按键触发本地复位，本地复位后将重新进入
    未配网状态。

## 字段定义
    应当在阿里精灵开发平台，注册选择BLE_MESH设备，并下载三元组，下载
    三元组可参考阿里精灵平台文档。

    三元组中，包括设备产品ID，设备MAC地址以及设备密钥。
    应当准确填写这些信息，否则将会配网失败。

### 注意
    蓝牙MESH静态RAM分配应当在蓝牙MESH（bt_mesh_init）前调用，
    如果使用默认配置，应当使用MESH_CFG_DEFAULT作为first Params.
    并为此分配相对应的RAM空间，可参考各相关例程参考，如果调用
    该函数返回非零值，代表内存不足，应当适当加大内存重试（二分法）。

    通常情况下，默认配置已经满足大多数的场景，对于某些特定场景下，
    应使用`MESH_CFG_COMMON`去配置底层消息缓存，重放攻击记录表大小以及
    IV更新分频。

    重放攻击表的大小应当根据具体的网络环境而确定，节点可能拒绝消息：
    1：节点重放攻击表满。
    2：节点收到不大于之前序号的消息

    蓝牙MESH配网及配置信息包括（网络层密钥、设备密钥、地址等）都将在
    FLASH中存储，默认存储在DATA FLASH第一个扇区，并默认占用3个扇区。

    可通过`MESH_CFG_STORE`配置相关信息。应当注意不与蓝牙FLASH数据存储
    地址冲突，默认情况下蓝牙存储BLE_SNV_ADDR=0x3EE00,并BLE_SNV=FALSE关闭。

    其他相关配置参数可通过mesh_cfg_pub.h配置。
# 适用于GD32F4xx的FreeRTOS工程

## 简介

FreeRTOS内核小巧，多任务功能齐全，很好的实时性等特点；相对于非RTOS的多任务工程，具有RTOS的项目代码逻辑更加清晰，实时性更好等优点。该项目的目的是移植FreeRTOS到GD32F4xx芯片上，并编写适用于FreeRTOS的设备驱动，可方便应用于用户的项目开发中。

## 开发环境

| 名称                      | 版本  |
| --- | --- |
| 测试Keil                  | 5.34  |
| GD32F4xx_Firmware_Library | 3.0.2 |
| FreeRTOS | 202112.00 |

## 片上外设驱动状态

| 外设 | 状态 |
| ---- | ---- |
| SPI Master | 支持 |
| UART | 支持 |
|I2C |计划中 |
|USB |未支持 |

## 外部外设驱动状态

| 外设  | 状态 |
| ----- | ---- |
| W5500 | 支持 |

## 文件夹介绍

├─component											组件

│		├─common										通用文件

│		├─driver											适用于FreeRTOS的驱动

│		├─freertos										FreeRTOS内核源码

│		└─GD32F4xx_Firmware_Library	GD32的SDK

├─keil_project											Keil工程文件

└─src															项目文件，用户的工程文件放在此文件夹

## 许可

本项目可任意复制、修改、使用，因为使用到FreeRTOS和GD32的SDK，请遵照相应模块的许可使用。

## 技术交流

QQ群: 228049587

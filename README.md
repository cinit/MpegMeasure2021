# 基于互联网的摄像测量系统

### 赛题

2021年 TI杯 全国大学生电子设计竞赛 赛题D 基于互联网的摄像测量系统

题目参见 [电赛官网](https://www.nuedc-training.com.cn/index/news/details/new_id/257)

### 复现方式

1. 准备两个 USB 摄像头, 分别接在树莓派 A 节点和树莓派 B 节点上 (A 和 B 节点也可用 Jetson Nano 代替)
2. 在 A 和 B 节点上编译安装 [JpegCamServer](https://github.com/cinit/JpegCamServer) 图传服务器
3. 准备一个 Jetson Nano 作为中心节点, 编译安装本项目
4. 配置 A 节点 IP 地址为 192.168.24.205, B 节点 IP 地址为 192.168.24.206, 中心节点 IP 地址为 192.168.24.204 (中心节点 IP 地址也可用同一网段其他 IP 代替)
5. 按赛题要求搭建场地, 并将三个节点接入同一交换机
6. 在中心节点 Jetson Nano 的 USB 接口上插上一个 USB 串口(TX/RX 不需要连接, 可以用任意串口, 有 /dev/ttyUSB0 就行), 如果没有串口, 可以修改 [main.cpp](./main.cpp)
   第 65 行跳过串口检测
7. 在各个节点运行各个程序, 在中心节点按 M 开始测量 (中心节点 Jetson Nano 连一个键盘)

### 程序依赖

- OpenCV (C++ 库, 可以不带 contrib)
- cmake, g++ 等编译工具

### 源代码结构

```
MpegMeasure2021
├── binder                      # 串口通信
│   ├── HwManager.cpp
│   ├── HwManager.h
│   ├── LinuxSerial.cpp
│   ├── LinuxSerial.h
│   ├── SerialInterface.cpp
│   └── SerialInterface.h
├── CMakeLists.txt
├── main.cpp                    # 主程序
├── MeasureSession.cpp          # 测量会话
├── MeasureSession.h
├── MeasureView.cpp             # 没有用到
├── MeasureView.h
├── mmtcp                       # A, B 节点图传代码
│   ├── MmTcpClassic.cpp
│   ├── MmTcpClassic.h
│   ├── MmTcpV2.cpp             # 使用的是这个, 其他的都不用
│   ├── MmTcpV2.h
│   ├── TcpClientSocket.cpp
│   ├── TcpClientSocket.h
│   ├── TcpServerSocket.cpp
│   └── TcpServerSocket.h
├── README.md
├── Recognition.cpp             # 识别激光笔
├── Recognition.h
├── ui
│   ├── Fixedsys.c              # Fixedsys FON 字体
│   ├── Fixedsys.h
│   ├── VgaFont.cpp             # 解析 FON 字体
│   ├── VgaFont.h
│   ├── Widgets.cpp             # 没用到
│   └── Widgets.h
└── utils
    ├── Time.cpp                # 时间工具函数等
    └── Time.h
```

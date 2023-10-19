当前目录文件说明：
├─libir sdk release：sdk库文件
│  ├─include：libir SDK的所有头文件
│  ├─linux库：linux下的动静态库
│  └─windows库：64/32位对应的动态/静态库
│
├─libir_sample文件：示例工程文件
│  ├─cmd_sample：单USB实例程序源码。（可出图及发命令）
│  ├─double_camera_sample：多USB实例程序源码。（可出图及发命令）
│  ├─libs：linux下的动静态库、windows下64位的静态库
│  │	└─include：libir SDK的所有头文件
│  ├─temp_sample：单USB实例程序源码。（可出图及测温）
│  ├─update_fw_sample：单USB实例程序源码。（可更新固件）
│  ├─win_include：windows下openCV及pthread等标准库的头文件
│  ├─x64
│  │	└─include：windows下64位的动态库、各个实例的应用程序
│  ├─windows下的sln工程
│  └─sample_version.h： sample示例程序的版本号
│
├─Windows Driver：驱动
│  ├─Infiray_Falcon001_5830.zip：5830驱动
│  └─Infiray_Falcon001_5830.zip：5840驱动
│
├─用户开发标定
│  ├─测温与锅盖标定
│  ├─环境变量修正
│  ├─盲元标定
│  └─README.txt
│
├─模组SDK接口V1.5.xlsx：依赖库的接口说明文档
├─linux USB接口开发说明文档 Interface development instructions.docx
├─README.txt
└─USB依赖关系Interface dependency.xlsx

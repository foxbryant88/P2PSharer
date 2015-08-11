一个通过P2P技术共享视频文件下载的项目，此项目是一个毕业设计，功能可能不是很完善。
其中使用了acl库，acl库请到https://github.com/zhengshuxin/acl下载最新的。

编译说明：
1.编译器：VS2013
2.acl目录与P2PShare放在同一目录下
3.acl中lib_acl、lib_acl_cpp属性调整如下：
  .平台工具集：Visual Studio 2013 - Windows XP (v120_xp)
  .C/C++ -> 代码生成 -> 运行库:多线程 DLL (/MD)
4.安装MFC MBCS DLL，参见：https://msdn.microsoft.com/library/dn251007.aspx









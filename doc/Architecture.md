##关于
此项目属于 GIT 分布式项目 Miracle 的一部分，开源版本的 svnsrv 没有使用核心路由模块，而是读取 router.toml 配置文件得到路由信息。

本文中的 svn 协议是指 Subversion 版本控制软件数据传输协议，基于 TCP, 协议范式遵循 ABNF, 服务器默认监听端口为 3690 端口，后端默认为 svnserve。

##编码
svnsrv 基于 C++ 开发，大部分编码使用了 C++11 标准支持的特性，如基于范围的循环，自动推到，智能指针等，


##移植


##VM
On a 64-bit system, these arenas are 64M mappings, and the maximum number of arenas is 8 times the number of cores.

Change use mallopt(); But not required

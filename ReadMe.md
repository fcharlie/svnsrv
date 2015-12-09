#svnsrv
此项目属于 Miracle 项目的一部分，现在提取出来先使用到 GIT 分布式第一版中。代理服务器主要的功能和 Miracle 中的 svnsrv 基本一致，
但是，核心路由替换成了路由文件，即 router.toml 。

一般而言，路由表的格式如下：
Host address 是默认机器的地址，port 是后端服务的的地址，这里通常是3690， Host.Content 是数组，如果为空，或者查询用户 MagicPath
区间查不到，那么将选择默认的地址也就是 Host 下的 address.   
```toml
[Host]
description="svnsrv router table toml feature"
author="Force Charlie"
update=2015-12-09T15:19:00Z
address="192.168.1.12"
port=3690
version=1
subver=0
old_version=0
old_subver=0
[[Host.Content]]
range="A"
address="192.168.1.12"
enbale=true
[[Host.Content]]
range="B~D"
address="192.168.1.13"
enbale=true
[[Host.Content]]
range="E~Zz"
address="192.168.1.14"
enbale=true
[[Host.Content]]
range="aA~zz"
address="192.168.1.15"
enbale=true
```


##Build
svnsrv 依赖 Boost ，使用 C++ 11 编译器，所以你必须要安装 GCC 4.8 或者是 clang 3.6 更高的版本。

构建非常简单：
>cd src && make

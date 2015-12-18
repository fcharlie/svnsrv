#svnsrv
此项目属于 Miracle 项目的一部分，现在提取出来先使用到 GIT 分布式第一版中。代理服务器主要的功能和 Miracle 中的 svnsrv 基本一致，但是，核心路由替换成了路由文件，即 router.toml 。     

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

在 Ubuntu 上安装依赖：    
```sh
sudo apt-get install libboost-dev libboost-system-dev libboost-thread-dev 
```
构建非常简单：
>cd src && make



##Run
svnsrv 配置文件：   
```toml
#svnsrv.toml
#Subversion Protocol Daemon
[Title]
Description="svnsrv a subversion client access proxy server,backend git repository"
Last="2015-11-12"
Author="Force.Charlie"

[Service]
PoolSize=32

[Daemon]
PidFile="/tmp/svnsrv.pid"

[Logger]
Access="/tmp/svnsrv.access.log"
Error="/tmp/svnsrv.error.log"

[Router]
RangeFile="router.toml"

[Network]
#if IPv6 0::0
Address="0.0.0.0"
Port=3690
Domain="git.oschina.net"
DomainFilter=false
# Compression 0~9 default 0
Compression=0
Tunnel=false
```

路由文件路径就是 svnsrv.toml 中 Router RangeFile 指定的路径,前文有指出。


命令行信息：    
```
OVERVIEW: svnsrv subversion proxy service
Usage: svnsrv [options] <input>
OPTIONS:
  -D [--daemon]      run svnsrv as a daemon
  -c [--config]      set svnsrv config file,file is toml format
  -d [--debug]       run svnsrv as debug mode
  -h [-?|--help]     print svnsrv usage information and exit
  -v [--version]     print svnsrv version and exit
  -q [--quiet]       print svnsrv version short mode
  -s [--signal]      send a signal to svnsrv,argument: stop|restart
```

以守护进程的方式运行：      
>svnsrv -D

指定配置文件：    
>svnsrv -c /etc/svnsrv.toml

停止守护进程：
>svnsrv -s stop

重启守护进程：   
>svnsrv -s restart

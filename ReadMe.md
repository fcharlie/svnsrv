#svnsrv
svnsrv 是 Subversion Protocol Dynamic Proxy Server (基于 svn 协议的动态代理服务器)，通过对用户请求的解析，
得到资源目标的机器，即时与资源服务器建立连接，实现客户端与资源服务器的数据交换。

##构建
svnsrv 运行在 linux 或者 Windows 上， 依赖 Boost ，使用 C++ 11 编译器。  
在 linux 上，你必须要安装 GCC 4.8 或者是 clang 3.6 更高的版本，在 Windows 上，需要 Visual Studio 2013 或者更高版本。   

###Ubuntu 15.10     
安装依赖      
```sh
sudo apt-get install libboost-dev libboost-system-dev libboost-thread-dev
```
编译    
>cd src && make

###Windows 7 以上版本
如果安装有 Visual Studio 2013 或者 2015 (建议社区版) 进入 src/msbuild ，双击 svnsrv.sln ，选择菜单 运行即可。

也可以在 powershell 中运行 build.ps1

##设置路由
Gitlab 类似的代码托管系统使用的是 Magic Path 划分用户，也就是使用用户用户名的前2个 ANSII 字符。我们可以将用户名转化为 16 bit 长的数字从而转变为区间。           
一般而言，路由表的格式如下：          
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
port=3690
[[Host.Content]]
range="B~D"
address="192.168.1.13"
enbale=true
port=3691
[[Host.Content]]
range="E~Zz"
address="192.168.1.14"
enbale=true
[[Host.Content]]
range="aA~zz"
address="192.168.1.15"
enbale=true
```

Host.address 是默认机器的地址，port 是后端服务的的地址，这里通常是3690，如果要设置每一个 svn 服务的端口，需要在指定的数组中设置端口,
Host.Content 是数组，如果为空，将使用默认的存储机器和端口。

用户 magic path 如果无法匹配所有区间，那么将选择默认的地址也就是 Host 下的 address.         

##运行
在运行 svnsrv 前，先得设置号 svnsrv.toml 配置文件，svnsrv.toml 搜索路径为进程所在目录，Home 目录下的 .svnsrv/ 。    
svnsrv 配置文件：   
```toml
#svnsrv.toml
#Subversion Protocol Daemon
[Title]
Description="svnsrv a subversion client access proxy server,backend git repository"
Last="2016-01-25"
Author="Force.Charlie"

[Service]
PoolSize=32

[Daemon]
AllowRestart=false
PidFile="/tmp/svnsrv.pid"

[Logger]
Access="/tmp/svnsrv.access.log"
Error="/tmp/svnsrv.error.log"

[Router]
RangeFile="router.toml"

[Network]
# Timeout=6000
#if IPv6 0::0
Address="0.0.0.0"
Port=3690
Domain="git.oschina.net"
DomainFilter=false
# Compression 0~9 default 0
Compression=0
Tunnel=false

```
路由文件路径就是 svnsrv.toml 中 Router.RangeFile 指定的路径,前文有指出。

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

##许可协议
svnsrv 作者 Force Charlie, 许可协议为 MIT       
© 2016. OSChina.NET. All Rights Reserved.    

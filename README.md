# TftpFileTransfer

经常有课程作业与文件传输有关，所以我实现了一个TFTP的文件传输程序作为基础，可以修改来满足各种需求。为了方便使用，这个程序没有区分客户端与服务端。默认是使用ipv6协议的。

## TFTP支持

该项目不是TFTP的完整实现， 同时有一些细节与RFC不同。

**支持的功能：**

- 大部分TFTP报文的构造与解析，参见[RFC1350](https://tools.ietf.org/html/rfc1350)与[RFC2347](https://tools.ietf.org/html/rfc2347)。
- octet的读请求与写请求处理。

**不支持的功能：**

- 对于netascii与mail类型报文的处理。
- OACK报文的构造与解析。
- DATA与ACK报文的计时重传。

**与RFC的区别：**

- 默认的控制端口为10000。

## 如何使用

### 编译

```
mkdir build/
cd build/ && cmake ..
make
```

### 运行

有的时候需要在本地运行两个实例测试，此时可以通过参数修改控制端口，默认为10000。

```
build/src/tftp 10001
```

启动之后，可以输入命令来开始传输。

```
send [filename] [ip] [port]
get [filename] [ip] [port]
```

例如：

```
send test.jpg ::1 10001
get test.jpg ::1 10001
```




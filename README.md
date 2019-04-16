# blaze: A multi-thread Linux C++ network library 

## 简介

blaze是仿照muduo[1]实现的一个基于Reactor模式的多线程C++网络库。简化的部分如下：
- 去掉了muduo/base中的基础库，采用C++11的新特性而非自己实现。
- 多线程依赖于C++11提供的std::thread，而不是重新封装POSIX thread API。
- 原子操作使用C++11提供的std::atomic。
- 重新实现了BlockingQueue、BoundedBlockingQueue、CountDownLatch等线程安全容器
## 示例

一个简单的echo服务器如下：

```C++
class EchoServer : public noncopyable
{
public:
  EchoServer(EventLoop* loop, const InetAddress& addr)
    : loop_(loop),
      server_(loop, addr),
  {
    // set echo callback
    server_.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
  }
  void Start() { server_.Start(); }
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
  {
    // echo message
    conn->Send(buf);
  }
private:
  EventLoop* loop_;
  TcpServer server_;
};
```

这个实现非常简单，只需关注`OnMessage`回调函数，它将收到消息发回客户端。然而，该实现有一个问题：若客户端只发送而不接收数据（即只调用`write`而不调用`read`），则TCP的流量控制（flow control）会导致数据堆积在服务端，最终会耗尽服务端内存。为解决该问题我们引入高/低水位回调：

```c++
class EchoServer: public noncopyable
{
public:
  ...
  void OnConnection(const TcpConnectionPtr& conn)
  {
    if (conn->Connected())
      conn->SetHighWaterMarkCallback(
            std::bind(&EchoServer::OnHighWaterMark, this, _1, _2), 1024);
  }
  void OnHighWaterMark(const TcpConnectionPtr& conn, size_t mark)
  {
    INFO("high water mark %lu bytes, stop read", mark);
    conn->StopRead();
  }
  void OnWriteComplete(const TcpConnectionPtr& conn)
  {
    if (!conn->IsReading()) {
      INFO("write complete, start read");
      conn->StartRead();
    }
  }
  ...
};
```

新增了3个回调：`OnConnection`，`OnHighWaterMark`和`OnWriteComplete`。当TCP连接建立时`OnConnection`会设置高水位回调值（high water mark）；当send buffer达到该值时，`onHighWaterMark`会停止读socket；当send buffer全部写入内核时，`onWriteComplete`会重新开始读socket。

除此以外，还需要给服务器加上定时功能以清除空闲连接。实现思路是让服务器保存一个TCP连接的`std::map`，每隔几秒扫描一遍所有连接并清除超时的连接，代码在[这里](./example/echo.cc)。

然后，我们给服务器加上多线程功能。实现起来非常简单，只需加一行代码即可：

```c++
EchoServer：：void start()
{
  // set thread num here
  server_.setNumThread(2);
  server_.start();
}
```

最后，main函数如下：

```c++
int main()
{
  EventLoop loop;
  // listen address localhost:9877
  InetAddress addr(9877);
  // echo server with 4 threads and timeout of 10 seconds
  EchoServer server(&loop, addr, 4, 10s);
  // loop all other threads except this one
  server.start();
  // quit after 1 minute
  loop.runAfter(1min, [&](){ loop.quit(); });
  // loop main thread
  loop.loop();
}
```

## 参考

[[1]](https://github.com/chenshuo/muduo) Muduo is a multithreaded C++ network library based on the reactor pattern.

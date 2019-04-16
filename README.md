# blaze: A multi-thread Linux C++ network library 

## 简介

blaze 是仿照 muduo[1] 实现的一个基于 Reactor 模式的多线程 C++ 网络库。改动的部分如下：
- 去掉了 muduo/base 中的基础库，利用 C++11 的新特性而非自己实现。
- 多线程依赖于 C++11 提供的 std::thread，而不是重新封装 POSIX thread API。
- 原子操作使用 C++11 提供的 std::atomic。
- 重新实现了 BlockingQueue、BoundedBlockingQueue、CountDownLatch 等线程安全容器
- 新增了ThreadGuard类，防止joinable thread对象析构时调用 std::terminate
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
  void Start() 
  { 
    server_.Start(); 
  }
  
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
class EchoServer : public noncopyable
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
    LOG_INFO << "high water mark " << mark << " bytes, stop read";
    conn->StopRead();
  }
  
  void OnWriteComplete(const TcpConnectionPtr& conn)
  {
    if (!conn->IsReading()) {
      LOG_INFO << "write complete, start read";
      conn->StartRead();
    }
  }
  ...
};
```

新增了3个回调：`OnConnection`，`OnHighWaterMark`和`OnWriteComplete`。当TCP连接建立时`OnConnection`会设置高水位回调值（high water mark）；当 output buffer 达到该值时，`OnHighWaterMark`会停止读socket；当 Output buffer全部写入内核时，`OnWriteComplete`会重新开始读socket。


然后，我们给服务器加上多线程功能。实现起来非常简单，只需加一行代码即可：

```c++
void EchoServer::SetThreadNum(size_t n)
{
  // set thread num here
  server_.SetNumThread(n);
}
```

最后，main函数如下：

```c++
int main()
{
  EventLoop loop;
  // listen address localhost:9877
  InetAddress server_addr(9877);
  // echo server with 4 threads and timeout of 10 seconds
  EchoServer server(&loop, server_addr);
  server.SetThreadNum(4)
  server.Start();
  // quit after 1 minute
  loop.RunAfter(60, [&](){ loop.quit(); });
  // loop main thread
  loop.Loop();
}
```
blaze还提供了定时器功能:
利用std::set管理定时器，实现较为简单
std::set提供了lower_bound功能，可以较快地找到已到期的定时器：
```c++
void PrintTid()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("pid = %d, tid = %s\n", getpid(), ss.str().data());
}

void Print(const char* msg)
{
    printf("msg %s %s\n", Timestamp::Now().ToString().c_str(), msg);
    if (++cnt == 20)
    {
        g_loop->Quit();
    }
}

void Cancel(TimerId timer)
{
    g_loop->CancelTimer(timer);
    // 自注销
    printf("cancelled timer at %s\n", Timestamp::Now().ToString().c_str());
}

int main()
{
    PrintTid();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;
        Print("main");
        loop.RunAfter(1, [](){Print("once1");});
        loop.RunAfter(1.5, [](){Print("once1.5");});
        loop.RunAfter(2.5, [](){Print("once2.5");});
        loop.RunAfter(3.5, [](){Print("once3.5");});
        TimerId t45 = loop.RunAfter(4.5, [](){Print("once4.5");});
        loop.RunAfter(4.2, [t45](){Cancel(t45);});
        loop.RunAfter(4.8, [t45](){Cancel(t45);});
        loop.RunEvery(2, [](){Print("every2");});
        TimerId t3 = loop.RunEvery(3, [](){Print("every3");});
        loop.RunAfter(9.001, [&t3](){Cancel(t3);});
        loop.Loop();
        printf("main loop exits");
    }
    sleep(1);
    {
        EventLoopThread loop_thread;
        EventLoop* loop = loop_thread.StartLoop();
        loop->RunAfter(2, PrintTid);
        sleep(3);
        printf("thread loop exits");
    }
}
```

## 参考

[[1]](https://github.com/chenshuo/muduo) Muduo is a multithreaded C++ network library based on the reactor pattern.

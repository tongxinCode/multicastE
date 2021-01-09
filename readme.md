# c_transfer

## notes

    1.the data of rates tested in the annotation is tested locally which means it's not really via real nic.
    2.send() with MSG_ZEROCOPY
        Linux 内核在 2017 年的 v4.14 版本接受了来自 Google 工程师 Willem de Bruijn 在 TCP 网络报文的通用发送接口 send() 中实现的 zero-copy 功能 (MSG_ZEROCOPY) 的 patch，通过这个新功能，用户进程就能够把用户缓冲区的数据通过零拷贝的方式经过内核空间发送到网络套接字中去，这个新技术和前文介绍的几种零拷贝方式相比更加先进，因为前面几种零拷贝技术都是要求用户进程不能处理加工数据而是直接转发到目标文件描述符中去的。Willem de Bruijn 在他的论文里给出的压测数据是：采用 netperf 大包发送测试，性能提升 39%，而线上环境的数据发送性能则提升了 5%~8%，官方文档陈述说这个特性通常只在发送 10KB 左右大包的场景下才会有显著的性能提升。一开始这个特性只支持 TCP，到内核 v5.0 版本之后才支持 UDP。
        目前来说，这种技术的主要缺陷有：只适用于大文件 (10KB 左右) 的场景，小文件场景因为 page pinning 页锁定和等待缓冲区释放的通知消息这些机制，甚至可能比直接 CPU 拷贝更耗时；因为可能异步发送数据，需要额外调用 poll() 和 recvmsg() 系统调用等待 buffer 被释放的通知消息，增加代码复杂度，以及会导致多次用户态和内核态的上下文切换；MSG_ZEROCOPY 目前只支持发送端，接收端暂不支持。
    3.for optimization, follow this procedure:
        a.better reading order
        b.do the less things in the loop , get the more rate
        c.use mmap at least
        d.use sendfile or splice for tcp;
          use splice for tcp;
        e.use the least kernal for MSG_ZEROCOPY

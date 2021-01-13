# multicast extreme tool

used to test the extreme rate in the network

tested successfully in 
    Linux master 3.10.0-1062.12.1.el7.x86_64.
    gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-36)
    boost c++ 1.75

## notes

    1.the program is for the multicast using udp.
    2.if you want to find out the optimization methods of other protocol, search the ZeroCopyTest folder.
    3.the program is based on the basic client/server model, there is a mini example in the ClientServerModel folder.

## run it

    make main

## clean it

    make clean

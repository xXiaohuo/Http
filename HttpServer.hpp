#ifndef __HTTPSERVER_HPP__
#define __HTTPSERVER_HPP__
#include<iostream>
#include"ProtocolUtil.hpp"
#include<unistd.h>
class HttpServer{
   public:


     HttpServer(int port_)
         :port(port_)
         ,listen_sock(-1)
    {
      

    };

     void InitServer(){ 
     
         listen_sock=SocketApi::Socket();
      
         SocketApi::Bind(listen_sock,port);
        
         SocketApi::Listen(listen_sock);
    }
     

     //启动服务器
     void Start(){

         for(;;){

            std::string peer_ip;
            int peer_port;
            //申请空间为了后序线程的传入参数
            int *sockp = new int;
            *sockp=SocketApi::Accept(listen_sock,peer_ip,peer_port);
            //判断是否有连接到来
            if(*sockp >= 0){
                std::cout<<peer_ip<<":"<<peer_port<<std::endl;
             //创建线程去处理新的请求
                pthread_t tid;             
                pthread_create(&tid,NULL,Entry::HandlerRequest,(void*)sockp);
                 

            }
                
         }

    }
     
        ~HttpServer(){
           if(listen_sock >= 0){
           
                close(listen_sock);
         }  
           

    }



     private:
        int listen_sock;
        int port;
};


#endif




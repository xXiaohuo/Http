#include <iostream>
#include <stdlib.h>
#include"HttpServer.hpp"

static void Usage(std::string proc)
    {

    std::cout<<"Usage :"<<proc<<"port"<<std::endl;

    }
//终端输入可执行程序加 端口
//./HttpServer 8000
int main(int argc,char *argv[])
{
    //终端输入作为参数
    if(argc!=2){
        Usage(argv[0]);
        exit(1);
    }
    HttpServer *ser = new HttpServer(atoi(argv[1]));
    ser->InitServer();
    ser->Start();


    return 0;
}



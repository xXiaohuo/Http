#ifndef __PROTOCOLUTIL_HPP__
#define  __PROTOCOLUTIL_HPP__ 
#include<netinet/in.h>
#include <iostream>
#include<strings.h>
#include<unordered_map>
#include<sys/types.h>
#include <sys/socket.h>
#include<string>
#include<sstream>
#include<fcntl.h>
#include<vector>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<algorithm>
#include<arpa/inet.h>
#include<pthread.h>
#include<stdlib.h>
#include <sys/sendfile.h>
#include<stdio.h>
#include<sys/stat.h>
#define BACKLOG 5
#define NORMAL 0
#define WARNING 1
#define ERROR 2
#define BUFF_NUM 1024
#define WEBROOT "wwwroot"
#define HOMEPAGE "index.html"
const char *ErrLevel[]{
    "Normal",
        "Warning",
        "Error",
};

//__FILE __  __LINE__

void log(std::string msg,int level ,std::string file,int line)
{

    std::cout<<"["<<file<<":"<<line<<"]"<<msg<<"["<<ErrLevel[level]<<"]"<<std::endl;

}

#define  LOG(msg,level) log(msg, level, __FILE__,__LINE__);

class Connect;
class Util{
    public:
       static void MakeKV(std::string s, std::string &k, std::string &v)
       {
           std::size_t pos = s.find(": ");
           k = s.substr(0, pos);
           v = s.substr(pos+2);
       }    
       static std::string IntToString(int &x)
       {
           std::stringstream ss;
           ss << x;
           return ss.str();
       }
       static std::string CodeToDesc (int code)
       {
           switch(code)
           {
              case 200:
                  return "OK";
              case 404: 
                  return "Not Found";
              default:  
                  break;
           }    
           return "UnKnow";
       }
       static std::string SuffixToContent(std::string &suffix)
       {
           if(suffix == ".css")
           {
               return "text/css";
           }
           if(suffix == ".js")
           {
               return "application/x-javascript"; 

           }
           if(suffix == ".html"||suffix == ".htm")
           {
               return "text/html";

           }
           if(suffix == ".jpg") 
           {
               return "application/x-jpg";
           } 
           if(suffix == ".png")
           {
               return "image/png";   
           }

           return "text/html";
       }
};

class SocketApi{
    public:
        static int Socket()
        {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if(sock < 0)
            {
                LOG("socket error!",ERROR);
                exit(2);
            }
            int opt = 1;
            //设置地址复用
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            return sock;
        }
        static void Bind(int sock,int port)
        {
            struct sockaddr_in local;
            bzero(&local,sizeof(local));
            local.sin_family=AF_INET;
            //端口号      
            local.sin_port=htons(port);
            //ip
            local.sin_addr.s_addr=htonl(INADDR_ANY);

            if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
            {
                LOG("Bind error!",ERROR);
                exit(3);
            }
        }

        static void Listen(int sock)
        {
            if(listen(sock,BACKLOG) < 0)
            {
                LOG("Listen Error!",ERROR);
                exit(4); 
            }
        }
        //创建接收套接字
        static int Accept(int listen_sock,std::string &ip,int &port){
            // struct sockaddr_in {
            //     222   __kernel_sa_family_t  sin_family; /* Address family   */
            //     223   __be16    sin_port; /* Port number      */                                           
            //     224   struct in_addr  sin_addr; /* Internet address   */
            //   225 

            //  }
            struct sockaddr_in peer;
            //创建套接字地址
            // 求套接字地址的大小
            // accept(socket,address,addr_len)
            socklen_t len=sizeof(peer);

            int sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
            //判断是否接受成功

            if(sock < 0)
            { 
                LOG("accept error",WARNING);
                return -1;
            }
            //将peer里面的Port转化成int 和ip进行转化成字符串
            //htons:端口转化
            //ntohs：字节序转化
            //htonl：32位int转化 ip转化
            //ntohl:32位的字节序转化
            port=ntohs(peer.sin_port);//sin_port本来就是int类型，为了同一字节序
            ip=inet_ntoa(peer.sin_addr);
            return sock;   
        }
};
class Http_Response{
     public:  
        //行
        std::string status_line;
        //头
        std::vector<std::string> response_header;
        //空行
        std::string blank;
        //正文
        std::string response_text;
     private:
        int code;
        std::string path;
        int recource_size;
     public:
        Http_Response():blank("\r\n"),code(200),recource_size(0)
        {

        }    
        int &Code()
        {
            return code;
        }
        void SetPath(std::string &path_)
        {
            path =path_;
    
        }
        std::string &Path()
        {
            return  path;
        }
        void SetRecourceSize(int rs)
        {
            recource_size = rs;
        }
        int RecourceSize()
        {
            return recource_size;        
        }
        void MakeStatusLine()
        {
            status_line = "HTTP/1.0";
            status_line += " ";
            status_line +=Util::IntToString(code);
            status_line += " ";
            status_line += Util::CodeToDesc(code);
            status_line += "\r\n";
            LOG("Make Status Line Done!",NORMAL);
        }

        void MakeResponseHeader()
        {
            //构建报头属性
            std::string line;
            std::string suffix;
            //Content-Type
            line="Content-Type: ";
            std::size_t pos=path.rfind('.');
            if(std::string::npos!=pos)
            {
              //suffix judeje Content-Type
              suffix=path.substr(pos);
              //lower
              transform(suffix.begin(),suffix.end(),suffix.begin(),::tolower);
            }
            //suffix transfrom Content-Type
            line+=Util::SuffixToContent(suffix);
            line+="\r\n";
            response_header.push_back(line);     
            //Conetent-Length  
            line  = "Connent-Length: "; 
            line += Util::IntToString(recource_size);
            line += "\r\n";
            response_header.push_back(line);
            line += "\r\n";
            response_header.push_back(line);
            LOG("Make Response Header  Done!",NORMAL);
        }  
        //构建正文
        // void MakeResponseText()
       // {
            //先打开文件，读取文件，然后把文件内容发送过去
            //这里引入sendfile读取文件高效
            //他没有从内核拷贝到用户的过程，直接从内核中拷贝
            //有两个文件描述符一个读一个获取
            //ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
                        

      // }    
        ~Http_Response()
        { }   


};

class Http_Request{
    public:
        //行
        std::string request_line;
        //头
        std::vector<std::string> request_header;
        //空行
        std::string blank;
        //正文
        std::string request_text;
    private:
        //解析字段
        std::string method;
        std::string uri;//path ? arg
        std::string version;
        std::string path;
        std::string  query_string;
        std::unordered_map<std::string, std::string> header_kv;
       // int  recource_size;
        bool cgi;
    public:
        Http_Request():path(WEBROOT),blank("\r\n")
        {}
        //对一行字符进行解析
        void RequestLineParse()
        {
            std:: stringstream ss(request_line);
            ss>>method>>uri>>version;
            //将方法转为大写 进行后序判读
            transform(method.begin(),method.end(),method.begin(),::toupper);
        }
        //对uri进行解析
        void UriParse()
        {
            if (method=="GET")
            {
                std::size_t pos=uri.find('?');
                if(pos !=std::string::npos)
                {
                    cgi = true;   
                    path  += uri.substr(0,pos);
                    query_string = uri.substr(pos+1);
                }
                else
                {
                    path += uri;// wwwroot/ab/c/s在目录前加上web的根目录
                }
            }
            else
            {//post
                cgi = true;
                path += uri;

            }
            if(path[path.size()-1] == '/')
            //用户可能请求的时候/
            //那么给他返回首页即可，每个目录下都有一个首页
            {

                path += HOMEPAGE; 
            }
        }
         
        //请求报头解析
        void HeaderParse()
        {   
            std::string k,v;
            for (auto it = request_header.begin();it != request_header.end();it++)
            {
                Util::MakeKV(*it, k, v);
                header_kv.insert({k,v});  
            }    
        }
        //判端方法是否合法 post get
        bool IsMethodLegal()
        {
            if(method != "GET" && method != "POST")
            {
                return false;
            }
            return true;
        }
        //判断路径是否合法
        int  IsPathLegal(Http_Response *rsp)//wwwroot/a/b/c/d.html 
        {
            //stat查看文件属性，如果文件存在返回属性
            //如果不存在返回-1
            //可以判断是路径是否正确
            int rs = 0; 
            struct stat st;
            if(stat(path.c_str(),&st) < 0)
            {
               return 404;
            }
            else
            { 
                 rs = st.st_size;           
                //成功了判断是否为文件夹
               //stat里面的宏stat中st—mode文件类返回文件的类型
               //stat中的S_ISDIR判断是否为目录
               if(S_ISDIR(st.st_mode))
               {
                  path += "/";
                  path += HOMEPAGE;
                  stat(path.c_str(),&st);
                  rs = st.st_size;           
               }
               else if((st.st_mode & S_IXUSR) ||  //用户可执行
                       (st.st_mode & S_IXGRP) ||  //组可执行
                       (st.st_mode & S_IXGRP))    //其他用户可执行
               //或者是可执行的程序
               //stat的宏判断是否为可执行的程序
               //用文件的类型和可执行宏按位与一下
               {
                  cgi = true; //可执行的交给cgi运行然后结果交给服务器
               }
               else
               {
                   // TODO

               }
            }
            //设置response的path
            rsp->SetPath(path);
            rsp->SetRecourceSize(rs); 
            LOG("Path is OK!",NORMAL);
            return 0;
        } 
    
        bool IsNeedRecv()
        {
            return method == "POST" ? true : false;
        }
        bool IsCgi()
        {
            return cgi;
        }
        //报文的长度
        int ConnentLength()
        {
            //把报文长度输出转为整型
            int content_length = -1;
            std::string sl = header_kv["Content-Length"].c_str();
            std::stringstream ss(sl);
            ss >> content_length;
            return content_length;
        }

        
        ~Http_Request()
        {}
};  


//数据读取
class Connect{
    private:
        int sock;   
    public:
        Connect(int sock_):sock(sock_)
        {}
        int RecvOneLine(std::string & line_)
        {
            //定以接受的buff
            char buff[BUFF_NUM];
            //一个字符一个的读
            char c = 'x';
            int i = 0;
            //循环读取，当一行读完且buff没有满
            while(c != '\n' && i < BUFF_NUM - 1)
            {
                ssize_t s = recv(sock,&c,1,0);
                if(s > 0)
                {
                    if(c == '\r')
                    {
                        //判断下一是否为\n,窥探读取一下
                        recv(sock,&c,1,MSG_PEEK);
                        if(c == '\n')
                        {
                            //将\r\n转为\n
                            recv(sock,&c,1,0);
                        }
                        else
                        {
                            //将\r 转为\n                      
                            c = '\n';
                        }
                    }
                    //\r \n \r\n
                    buff[i++]=c;
                }
                else
                {
                    break;
                }
            }    
            buff[i] = 0;
            line_ = buff;
            return i;
        }
        void RcvRequestHeader(std::vector<std::string> & v)
        {
            std::string line;
            while(line != "\n")
            {
                RecvOneLine(line);
                if(line != "\n")
                {    
                  v.push_back(line);
                }  
            }    
            LOG("Header Recv OK!",NORMAL);

        }
        //读取正文
        void RecvText(std::string  &text,int content_length)
        {
            char c;
            for(auto i = 0; i < content_length; i++)
            {
                //从sock中读每次读一个
                recv(sock, &c, 1, 0);
                //读了放在text中
                text.push_back(c);
            }    
        }
        void SendStatusLine(Http_Response *rsp)
        {
            
            std::string &sl =rsp->status_line;
            send(sock,sl.c_str(),sl.size(),0);    
            LOG("Send Response Status Line OK!",NORMAL);    
        }

        void SendHeader(Http_Response *rsp)//add \n
        {
            //发送报头
            std::vector<std::string> &v=rsp->response_header;
            for(auto it = v.begin(); it != v.end(); it++)
            {
                send(sock, it->c_str(), it->size(), 0);     
            }    
            LOG("Send Response Header OK!",NORMAL);    

        }   
        void SendText(Http_Response *rsp)
        {
            std::string &path = rsp->Path();
            int fd = open(path.c_str(),O_RDONLY);
            if(fd < 0)
            {
                LOG("open file error!", WARNING);    
                return;                                
            }
            sendfile(sock,fd,NULL,rsp->RecourceSize());
            LOG("Send Response Text OK!",NORMAL);    
            close(fd);
        }
        ~Connect()
        {
             close (sock);
        }
};
class Entry{
    public:     
        //构建非cgi的方式
        static void PocessNonCgi(Connect *conn,Http_Request *rq,Http_Response *rsp) 
        {
           //构建响应行
           rsp->MakeStatusLine();
           //构建响应报头
           rsp->MakeResponseHeader();
           //构建正文
           // rsp->MakeResponseText();
           //构建好的响应发送出去
           conn->SendStatusLine(rsp);
           conn->SendHeader(rsp);//add \n
           conn->SendText(rsp);
        

        }
        //读完请求正文，开始响应
        static void  ProcessResponse(Connect *conn,Http_Request *rq,Http_Response *rsp)
        {
           //分析请求构建响应，发送到网络中   
           //分析请求是否为cgi方式处理
           if(rq->IsCgi())
           {
               
            //　PocessCgi(); 
           } 
           else{
              LOG("MakeResponse Use Non Cgi",NORMAL);
              PocessNonCgi(conn, rq, rsp); 
           }

        }
        //处理请求函数
        static void *HandlerRequest(void*arg)
        {
            pthread_detach(pthread_self());
            int *sock=(int*)arg;
#ifdef _DEBUG_            
            //for test 
            char buf[10240];
            read(*sock,buf,sizeof(buf));
            std::cout<<"####################"<<std::endl;
            std::cout<<buf<<std::endl;
            std::cout<<"####################"<<std::endl;
#else       
            //创建对象读取数据
            Connect *conn = new Connect(*sock);
            //读取一行数据
            Http_Request *rq = new Http_Request;
            Http_Response *rsp = new Http_Response;
            
            conn->RecvOneLine(rq->request_line);
            
            //错误码
            int & code = rsp->Code();

            //对请求行进行解析
            rq->RequestLineParse();

            //判断请求方法 post getopt 
            if(!rq->IsMethodLegal()) 
            {
                code = 404;
                LOG("Rquest Method Is Not Legal",WARNING);
                goto end;
            }

            //获取路径
            rq->UriParse();
           
            //判断路径是否合法
            if(rq->IsPathLegal(rsp) != 0)//不合法
            {
                code = 404;
                LOG("file is not exist!",WARNING);
                goto end;  
            }
            
            //开始获取报头
            conn->RcvRequestHeader(rq->request_header);
            
            //报头的解析
            //对应的键值对放入unordermap中
            rq->HeaderParse();
            //判断是否需要继续读
            //读取正文
            //读取正文那么就要需要conent-length
            if(rq->IsNeedRecv())
            {
                LOG("POST Method,Need Recv Begin!",NORMAL);
                //读到哪里，长度
                conn->RecvText(rq->request_text,rq->ConnentLength());

            }    
            LOG("Http Request Recv Done, OK!",NORMAL);
            //读完请求正文，开始响应
            ProcessResponse(conn,rq,rsp);

end:        

        delete conn;
        delete rq;
        delete rsp;
        delete sock;
#endif
            //关闭套接字
        return (void*)0;

        }

};


#endif 

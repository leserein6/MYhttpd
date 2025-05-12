#include<bits/stdc++.h>
#include<ws2tcpip.h>
//网络通信需要包含的头文件、需要加载的库文件
#include<WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
using namespace std;
void error_die(const char* str)
{
	perror(str);//打印错误元音
	exit(1);//结束程序
}
//实现网络的初始化
//返回值：套接字
//端口
//参数：port 表示端口 是一个unsigned short类型
//如果*port的值是0，那么就自动分配一个可用的端口
int startup(unsigned short* port) 
{
	//1.网络通信的初始化
	WSADATA data;
	int ret = WSAStartup(MAKEWORD(1,1),//1.1版本的协议
		&data);
	if (ret) {//ret!=0
		error_die("WSAStartup");
	}
	//2.创建套接字
	int server_socket=socket(PF_INET,//套接字的类型
		SOCK_STREAM,
		IPPROTO_TCP);
	if (server_socket == -1)
	{
		error_die("套接字");
	}
	//3.设置端口可复用
	int opt = 1;
	setsockopt(server_socket,
		SOL_SOCKET, SO_REUSEADDR,
		(const char*)& opt, sizeof(opt));
	if (ret == -1) {
		error_die("setsockopt");
	}
	//配置服务器端的网络地址
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;//网络地址类型
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//0
	//绑定套接字
	//bind 一开始报错 因为命名冲突了 明确的指定为Windows socket的bind函数就好
	if (::bind(server_socket,(struct sockaddr*)&server_addr,
		sizeof(server_addr)) <0 )
	{
		error_die("bind");
	}
	// 动态分配端口
	int nameLen = sizeof(server_addr);
	if (*port == 0)
	{
		if (getsockname(server_socket,
			(struct sockaddr*)&server_addr,
			&nameLen) < 0)
		{
			error_die("getsockname");
		}
		*port = server_addr.sin_port;
	}
	//创建监听队列
	if (listen(server_socket, 5) < 0)
	{
		error_die("listen");
	}
	return server_socket;
}
int main() {
	unsigned short  port = 0;//如果是0 动态端口
	int server_sock = startup(&port);
	printf("httpd服务已经启动，正在监听%d 端口...", port);

}
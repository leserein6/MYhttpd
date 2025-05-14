#include<bits/stdc++.h>
#include<ws2tcpip.h>
//网络通信需要包含的头文件、需要加载的库文件
#include<WinSock2.h>
#include<sys/types.h>
#include<sys/stat.h>
#pragma comment(lib,"WS2_32.lib")
using namespace std;
//void PRINTF(const char* str)
//{
//	printf("[%s - %d]%s", __func__, __LINE__, str);
//}
#define PRINTF(str) printf("[%s - %d]"#str"=%s", __func__, __LINE__, str);
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
//处理用户请求的线程函数
//接收浏览器的WEB请求
//服务器准备好后，就可以接受来自前端浏览器的网页访问请求了
/*浏览器访问网站的完整流程
* 1.浏览器发起网页访问请求 发送GET请求包
* 2.服务器发送响应包，发送被请求的网页
* 3.浏览器根据收到的网页内容，再次发送GET请求包，申请获取网页中包含的图片、JS、CSS等文件
* 4.服务器发响应包，发送被请求的资源
* 5.用户在网页中填写数据，点击提交，发送POST请求
* 6.发送POST请求的相应包
*/
//因为可能有多个用户同时发起请求，为了更快的处理网页请求，使用多线程技术
//从指定的客户端套接字 读取一行数据 ，保存到参数buff中
//返回实际读取到的字节数
int get_line(int sock, char* buff, int size)
{
	char c = 0;
	int i = 0;
	while (i<size-1 && c!='\n') {
		int n = recv(sock, &c, 1,0);
		if (n > 0) { 
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);//检查下一个字符是不是\n
				if (n > 0 && c == '\n')
				{
					recv(sock, &c, 1, 0);
				}
				else {
					c = '\n';
				}
			}
			buff[i++] = c;

		}
		else {
			c = '\n';
		}

	}
	buff[i] = 0;//'\0
	return i;
}
void unimplement(int client)
{
	//向指定的套接字，发送一个提示 还没有实现的错误页面

}
void not_found(int client)
{

}
DWORD WINAPI accept_request(LPVOID arg)
{
	char buff[1024];//1K
	int client = (SOCKET)arg;//客户端套接字
	//读一行数据
	int numchars = get_line(client, buff, sizeof(buff));
	//printf("[%s - %d]%s", __func__, __LINE__, buff);
	//printf("读到： %s\n", buff);
	PRINTF(buff);//[accept_request-53]buff="EGT....."
	char method[255];
	int j = 0;
	int i = 0;
	while (!isspace(buff[j]) && i<sizeof(method)-1)
	{
		method[i++] = buff[j++];
	}
	method[i] = 0;//'0'
	PRINTF(method);
	
	// 检查请求的方法， 本服务器是否支持
	if (strcmp(method, "GET") && strcmp(method, "POST"))
	{
		//向浏览器返回一个错误提示页面
		unimplement(client);
		return 0;
	}
	//解析资源文件的路径
	//www.rock.com/abc/test.html
	//GET /abc/test.html HTTP/1.1\n
	char url[255];//存放请求的资源的完整路径
	i = 0;
	while (isspace(buff[j]) && j < sizeof(buff))
	{
		j++;//跳过空格 以及  反斜杠
	}
	while (!isspace(buff[j]) && i < sizeof(url) - 1 && j<sizeof(buff))
	{
		url[i++] = buff[j++];
	}
	url[i] = 0;
	PRINTF(url);
	//url / 
	//htdocs/index.html
	char path[512] = "";
	sprintf_s(path, "htdocs%s", url);
	if(path[strlen(path)-1]=='/')strcat_s(path, "index.html");
	PRINTF(path);
	struct stat status;
	if (stat(path, &status) == -1)
	{
		//请求包的剩余数据读取完毕
		//numchars 记录读了多少字符
		while (numchars > 0 && strcmp(buff, "\n")) 
		{
			numchars = get_line(client, buff, sizeof(buff));
		}

	}
	else 
	{

	}


	return 0;
}
int main() {
	unsigned short port = 8000;//如果是0 动态端口
	//80端口被占用了
	int server_sock = startup(&port);
	printf("httpd服务已经启动，正在监听%d 端口...", port);
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1)
	{
		//等待客户端访问
		//阻塞式等待用户通过浏览器发起访问
		int client_sock = accept(server_sock, 
			(struct sockaddr*) & client_addr, 
			&client_addr_len);//一对一 新的套接字 客户端套接字
		if (client_sock == -1)
		{
			error_die("accept");
		}
		//创建一个新的线程  
		// 进程（可以包含多个线程）
		DWORD threadId = 0;//线程标志
		CreateThread(0, 0,
			accept_request,
			(void*)client_sock,//accept_request  参数
			0, &threadId);
		
	}
	closesocket(server_sock);//关闭主套接字
}
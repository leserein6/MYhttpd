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
	perror(str);//打印错误原因
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
			(const char*)&opt, sizeof(opt));
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
		if (::bind(server_socket, (struct sockaddr*)&server_addr,
			sizeof(server_addr)) < 0)
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
			//*port = server_addr.sin_port;修改1 处理动态端口
			*port = ntohs(server_addr.sin_port);
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
	while (i < size - 1 && c != '\n') {
		int n = recv(sock, &c, 1, 0);
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
	// 定义404页面HTML内容
	const char* html = "<html>"
		"<title>NOT FOUND</title>"
		"<body>"
		"<h2>The resource is unavailable.</h2>"
		"<img src=\"NOTFOUND.jpg\"/>"
		"</body>"
		"</html>";

	// 计算内容长度
	int content_length = strlen(html);

	// 构造完整的HTTP响应头（单次格式化保证原子性）
	char header[1024];
	sprintf_s(header, sizeof(header),
		"HTTP/1.0 404 NOT FOUND\r\n"
		"Server: LesereinHttpd/0.1\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n\r\n",  // 注意两个\r\n结束头
		content_length);

	// 发送响应（添加错误检查）
	if (send(client, header, strlen(header), 0) == SOCKET_ERROR ||
		send(client, html, content_length, 0) == SOCKET_ERROR) {
		closesocket(client);
		return;
	}
}
/*
void not_found(int client)
{

	//发送404响应
	char buff[1024];
	strcpy_s(buff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Server：LesereinHttpd/0.1\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "\r\n");
	send(client, buff, strlen(buff), 0);
	sprintf_s(buff, "<html>                      \
		<title>NOT FOUND</title>              \
		<body>                                \
		<h2>the resource is unavailable.</h2> \
		<img src = \"NOTFOUND.jpg\"/>         \ \
		</body>                               \ \
		</html>");
	send(client, buff, strlen(buff), 0);
	//发送404网页内容
	/*
	*<html>
	*	<title>NOT FOUND</title>
	*	<body>
	*		<h2>the resource is unavailable.</h2>
	*		<img src-"NOTFOUND.pnj"/>
	*	</body>
	* </html>
	*/

	// 需要添加图片处理函数
/*
void serve_image(int client, const char* path)
{
	FILE* fp = NULL;
	fopen_s(&fp, path, "rb");
	if (fp) {
		char header[1024];
		sprintf_s(header, sizeof(header),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: image/png\r\n"
			"Content-Length: %lld\r\n\r\n",
			_filelengthi64(_fileno(fp)));

		send(client, header, strlen(header), 0);

		// 发送图片数据
		char buffer[4096];
		size_t bytesRead;
		while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			send(client, buffer, bytesRead, 0);
		}
		fclose(fp);
	}
	else {
		not_found(client);  // 如果连404图片都不存在，继续返回404
	}
}
*/
/*
void not_found(int client)
{
	// 404响应内容
	const char* html = "<html>"
		"<title>NOT FOUND</title>"
		"<body>"
		"<h2>The resource is unavailable.</h2>"
		"<img src=\"NOTFOUND.jpg\"/>"
		"</body>"
		"</html>";

	char buff[1024];

	// 构造标准HTTP响应头
	sprintf_s(buff, sizeof(buff),
		"HTTP/1.0 404 NOT FOUND\r\n"
		"Server: LesereinHttpd/0.1\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %zu\r\n"
		"Connection: close\r\n\r\n",  // 注意这里有两个\r\n表示头结束
		strlen(html));

	send(client, buff, strlen(buff), 0);
	send(client, html, strlen(html), 0);
   
	
	// 注意：需要单独实现图片请求处理
	// 当浏览器请求NOTFOUND.png时需要返回真实的图片数据
}
//支持跨平台

void serve_image(int client, const char* path)
{
	FILE* fp = NULL;
	fopen_s(&fp, path, "rb"); // 必须用二进制模式
	if (fp) {
		// 获取文件大小（跨平台方法）
		long long file_size;
#if defined(_WIN32)
		_fseeki64(fp, 0, SEEK_END);
		file_size = _ftelli64(fp);
		_fseeki64(fp, 0, SEEK_SET);
#else
		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
#endif

		// 构造HTTP头
		char header[1024];
		sprintf_s(header, sizeof(header),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: image/png\r\n"
			"Content-Length: %lld\r\n\r\n",
			file_size);

		send(client, header, strlen(header), 0);

		// 发送图片数据
		char buffer[4096];
		size_t bytesRead;
		while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			send(client, buffer, bytesRead, 0);
		}
		fclose(fp);
	}
	else {
		not_found(client); // 连404图片都不存在时继续返回404
	}
}
*/
void headers(int client)
{
	//发送响应包的头信息
	char buff[1024];
	strcpy_s(buff, "HTTP/1.0 200 OK\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Server：LesereinHttpd/0.1\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Content-type:text/html\n");
	send(client, buff, strlen(buff), 0);

	/*char buf[1024];
	sprintf(buf,"Content-Type: %s\r\n",type);*/
	strcpy_s(buff, "\r\n");
	send(client, buff, strlen(buff), 0);

}
void cat(int client, FILE* resource)
{
	//一次读一个字节 然后把该字节发给浏览器
	char buff[4096];
	//12345
	int count = 0;
	while (1) 
	{
		int ret = fread(buff, sizeof(char), sizeof(buff), resource);
		if (ret <= 0)break;
		send(client, buff, ret, 0);
		count += ret;
	}
	printf("一共发送[%d]字节给浏览器\n", count);
}
//发送资源给客户端
void server_file(int client, const char* fileName)
{
	int numchars = 1;
	char buff[1024];
	//把请求数据包的剩余行，读完
	while (numchars > 0 && strcmp(buff, "\n"))
	{
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}
//	FILE* resource = fopen(fileName, "r"); 引发警告
	FILE* resource=NULL;
	//fopen_s(&resource, fileName, "r");//r 是 rt 文本文件
	//图片打不开的bug↑ 
	//fopen_s(&resource, fileName, "rb");
	/*if (strcmp(fileName, "htdocs/index.html") == 0)
		{
			fopen_s(&resource, fileName, "r");
		}
	else {
			fopen_s(&resource, fileName, "rb");
		}*/
	//统一使用二进制模式打开
	fopen_s(&resource, fileName, "rb");
	if (resource == NULL)//待访问文件不存在
		{
			not_found(client);
		}
		//正式发送资源给浏览器
		headers(client);
		//发送请求的资源信息
		cat(client,resource);
		printf("资源发送完毕!\n");
		if(resource)fclose(resource);//防止关闭空文件
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
	/*const char* ext = strrchr(path, '.');
	if (ext)
	{
		if (strcmp(ext, ".png") == 0)
		{
			serve_image(client, path);
			closesocket(client);
			return 0;
		}
	}*/
	struct stat status;
	if (stat(path, &status) == -1)
	{
		//请求包的剩余数据读取完毕
		//numchars 记录读了多少字符
		while (numchars > 0 && strcmp(buff, "\n")) 
		{
			numchars = get_line(client, buff, sizeof(buff));
		}
		not_found(client);//不存在
	}
	else 
	{
		if ((status.st_mode & S_IFMT) == S_IFDIR)//如果是目录
		{
			strcat_s(path, "/index.html");
		}
		/*if (ext && strcmp(ext, ".png") == 0)
		{
			serve_image(client, path);
		}*/
	/*	else {
			server_file(client, path);
		}*/
		server_file(client, path);
	}
	closesocket(client);
	return 0;
}
int main() {
	unsigned short port = 0;//如果是0 动态端口
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
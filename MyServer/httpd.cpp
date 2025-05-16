#include<bits/stdc++.h>
#include<ws2tcpip.h>
//����ͨ����Ҫ������ͷ�ļ�����Ҫ���صĿ��ļ�
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
	perror(str);//��ӡ����ԭ��
	exit(1);//��������
}

//ʵ������ĳ�ʼ��
//����ֵ���׽���
//�˿�
//������port ��ʾ�˿� ��һ��unsigned short����
//���*port��ֵ��0����ô���Զ�����һ�����õĶ˿�
int startup(unsigned short* port) 
{
	//1.����ͨ�ŵĳ�ʼ��
	WSADATA data;
	int ret = WSAStartup(MAKEWORD(1,1),//1.1�汾��Э��
		&data);
	if (ret) {//ret!=0
		error_die("WSAStartup");
	}
	//2.�����׽���
	int server_socket=socket(PF_INET,//�׽��ֵ�����
		SOCK_STREAM,
		IPPROTO_TCP);
		if (server_socket == -1)
		{
			error_die("�׽���");
		}
		//3.���ö˿ڿɸ���
		int opt = 1;
		setsockopt(server_socket,
			SOL_SOCKET, SO_REUSEADDR,
			(const char*)&opt, sizeof(opt));
		if (ret == -1) {
			error_die("setsockopt");
		}
		//���÷������˵������ַ
		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;//�����ַ����
		server_addr.sin_port = htons(*port);
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//0
		//���׽���
		//bind һ��ʼ���� ��Ϊ������ͻ�� ��ȷ��ָ��ΪWindows socket��bind�����ͺ�
		if (::bind(server_socket, (struct sockaddr*)&server_addr,
			sizeof(server_addr)) < 0)
		{
			error_die("bind");
		}
		// ��̬����˿�
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
		//������������
		if (listen(server_socket, 5) < 0)
		{
			error_die("listen");
		}
		return server_socket;
}
//�����û�������̺߳���
//�����������WEB����
//������׼���ú󣬾Ϳ��Խ�������ǰ�����������ҳ����������
/*�����������վ����������
* 1.�����������ҳ�������� ����GET�����
* 2.������������Ӧ�������ͱ��������ҳ
* 3.����������յ�����ҳ���ݣ��ٴη���GET������������ȡ��ҳ�а�����ͼƬ��JS��CSS���ļ�
* 4.����������Ӧ�������ͱ��������Դ
* 5.�û�����ҳ����д���ݣ�����ύ������POST����
* 6.����POST�������Ӧ��
*/
//��Ϊ�����ж���û�ͬʱ��������Ϊ�˸���Ĵ�����ҳ����ʹ�ö��̼߳���
//��ָ���Ŀͻ����׽��� ��ȡһ������ �����浽����buff��
//����ʵ�ʶ�ȡ�����ֽ���
int get_line(int sock, char* buff, int size)
{
	char c = 0;
	int i = 0;
	while (i < size - 1 && c != '\n') {
		int n = recv(sock, &c, 1, 0);
		if (n > 0) {
			if (c == '\r')
			{
				n = recv(sock, &c, 1, MSG_PEEK);//�����һ���ַ��ǲ���\n
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
	//��ָ�����׽��֣�����һ����ʾ ��û��ʵ�ֵĴ���ҳ��

}

void not_found(int client)
{

	//����404��Ӧ
	char buff[1024];
	strcpy_s(buff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Server��LesereinHttpd/0.1\r\n");
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
	//����404��ҳ����
	/*
	*<html>
	*	<title>NOT FOUND</title>
	*	<body>
	*		<h2>the resource is unavailable.</h2>
	*		<img src-"NOTFOUND.pnj"/>
	*	</body>
	* </html>
	*/
}
	// ��Ҫ���ͼƬ������
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

		// ����ͼƬ����
		char buffer[4096];
		size_t bytesRead;
		while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			send(client, buffer, bytesRead, 0);
		}
		fclose(fp);
	}
	else {
		not_found(client);  // �����404ͼƬ�������ڣ���������404
	}
}
*/
/*
void not_found(int client)
{
	// 404��Ӧ����
	const char* html = "<html>"
		"<title>NOT FOUND</title>"
		"<body>"
		"<h2>The resource is unavailable.</h2>"
		"<img src=\"NOTFOUND.jpg\"/>"
		"</body>"
		"</html>";

	char buff[1024];

	// �����׼HTTP��Ӧͷ
	sprintf_s(buff, sizeof(buff),
		"HTTP/1.0 404 NOT FOUND\r\n"
		"Server: LesereinHttpd/0.1\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %zu\r\n"
		"Connection: close\r\n\r\n",  // ע������������\r\n��ʾͷ����
		strlen(html));

	send(client, buff, strlen(buff), 0);
	send(client, html, strlen(html), 0);
   
	
	// ע�⣺��Ҫ����ʵ��ͼƬ������
	// �����������NOTFOUND.pngʱ��Ҫ������ʵ��ͼƬ����
}
//֧�ֿ�ƽ̨

void serve_image(int client, const char* path)
{
	FILE* fp = NULL;
	fopen_s(&fp, path, "rb"); // �����ö�����ģʽ
	if (fp) {
		// ��ȡ�ļ���С����ƽ̨������
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

		// ����HTTPͷ
		char header[1024];
		sprintf_s(header, sizeof(header),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: image/png\r\n"
			"Content-Length: %lld\r\n\r\n",
			file_size);

		send(client, header, strlen(header), 0);

		// ����ͼƬ����
		char buffer[4096];
		size_t bytesRead;
		while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			send(client, buffer, bytesRead, 0);
		}
		fclose(fp);
	}
	else {
		not_found(client); // ��404ͼƬ��������ʱ��������404
	}
}
*/
void headers(int client)
{
	//������Ӧ����ͷ��Ϣ
	char buff[1024];
	strcpy_s(buff, "HTTP/1.0 200 OK\r\n");
	send(client, buff, strlen(buff), 0);

	strcpy_s(buff, "Server��LesereinHttpd/0.1\r\n");
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
	//һ�ζ�һ���ֽ� Ȼ��Ѹ��ֽڷ��������
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
	printf("һ������[%d]�ֽڸ������\n", count);
}
//������Դ���ͻ���
void server_file(int client, const char* fileName)
{
	int numchars = 1;
	char buff[1024];
	//���������ݰ���ʣ���У�����
	while (numchars > 0 && strcmp(buff, "\n"))
	{
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}
//	FILE* resource = fopen(fileName, "r"); ��������
	FILE* resource=NULL;
	//fopen_s(&resource, fileName, "r");//r �� rt �ı��ļ�
	//ͼƬ�򲻿���bug�� 
	//fopen_s(&resource, fileName, "rb");
	/*if (strcmp(fileName, "htdocs/index.html") == 0)
		{
			fopen_s(&resource, fileName, "r");
		}
	else {
			fopen_s(&resource, fileName, "rb");
		}*/
	//ͳһʹ�ö�����ģʽ��
	fopen_s(&resource, fileName, "rb");
	if (resource == NULL)//�������ļ�������
		{
			not_found(client);
		}
		//��ʽ������Դ�������
		headers(client);
		//�����������Դ��Ϣ
		cat(client,resource);
		printf("��Դ�������!\n");
		if(resource)fclose(resource);//��ֹ�رտ��ļ�

}
DWORD WINAPI accept_request(LPVOID arg)
{
	char buff[1024];//1K
	int client = (SOCKET)arg;//�ͻ����׽���
	//��һ������
	int numchars = get_line(client, buff, sizeof(buff));
	//printf("[%s - %d]%s", __func__, __LINE__, buff);
	//printf("������ %s\n", buff);
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
	
	// �������ķ����� ���������Ƿ�֧��
	if (strcmp(method, "GET") && strcmp(method, "POST"))
	{
		//�����������һ��������ʾҳ��
		unimplement(client);
		return 0;
	}
	//������Դ�ļ���·��
	//www.rock.com/abc/test.html
	//GET /abc/test.html HTTP/1.1\n
	char url[255];//����������Դ������·��
	i = 0;
	while (isspace(buff[j]) && j < sizeof(buff))
	{
		j++;//�����ո� �Լ�  ��б��
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
		//�������ʣ�����ݶ�ȡ���
		//numchars ��¼���˶����ַ�
		while (numchars > 0 && strcmp(buff, "\n")) 
		{
			numchars = get_line(client, buff, sizeof(buff));
		}
		not_found(client);//������
	}
	else 
	{
		if ((status.st_mode & S_IFMT) == S_IFDIR)//�����Ŀ¼
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
	unsigned short port = 8000;//�����0 ��̬�˿�
	//80�˿ڱ�ռ����
	int server_sock = startup(&port);
	printf("httpd�����Ѿ����������ڼ���%d �˿�...", port);
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1)
	{
		//�ȴ��ͻ��˷���
		//����ʽ�ȴ��û�ͨ��������������
		int client_sock = accept(server_sock, 
			(struct sockaddr*) & client_addr, 
			&client_addr_len);//һ��һ �µ��׽��� �ͻ����׽���
		if (client_sock == -1)
		{
			error_die("accept");
		}
		//����һ���µ��߳�  
		// ���̣����԰�������̣߳�
		DWORD threadId = 0;//�̱߳�־
		CreateThread(0, 0,
			accept_request,
			(void*)client_sock,//accept_request  ����
			0, &threadId);
		
	}
	closesocket(server_sock);//�ر����׽���
}
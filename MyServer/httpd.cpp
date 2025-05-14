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
	perror(str);//��ӡ����Ԫ��
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
		(const char*)& opt, sizeof(opt));
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
	if (::bind(server_socket,(struct sockaddr*)&server_addr,
		sizeof(server_addr)) <0 )
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
	while (i<size-1 && c!='\n') {
		int n = recv(sock, &c, 1,0);
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
	struct stat status;
	if (stat(path, &status) == -1)
	{
		//�������ʣ�����ݶ�ȡ���
		//numchars ��¼���˶����ַ�
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
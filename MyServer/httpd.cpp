#include<bits/stdc++.h>
#include<ws2tcpip.h>
//����ͨ����Ҫ������ͷ�ļ�����Ҫ���صĿ��ļ�
#include<WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
using namespace std;
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
int main() {
	unsigned short  port = 0;//�����0 ��̬�˿�
	int server_sock = startup(&port);
	printf("httpd�����Ѿ����������ڼ���%d �˿�...", port);

}
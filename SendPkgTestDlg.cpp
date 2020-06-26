﻿
// SendPkgTestDlg.cpp: 实现文件
//

#include "pch.h"
#include <string.h>
#include "framework.h"
#include "SendPkgTest.h"
#include "SendPkgTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSendPkgTestDlg 对话框



CSendPkgTestDlg::CSendPkgTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SENDPKGTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSendPkgTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSendPkgTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSendPkgTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSendPkgTestDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSendPkgTestDlg 消息处理程序

BOOL CSendPkgTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSendPkgTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSendPkgTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


SOCKET           sClient;
void CSendPkgTestDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//连接
	static int ifinit = false;
	if (ifinit == false)
	{
		ifinit = true;
		WSADATA m_wsadata;
		if (WSAStartup(MAKEWORD(2, 2),&m_wsadata))   //初始化socket，WSACleanup()释放变量
		{
			AfxMessageBox(_T("Socket初始化失败!"));
		}
	}
	//(1)创建socket
	sClient = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		int nCodeError = ::WSAGetLastError();
		AfxMessageBox(_T("Socket失败！"));
		return;
	}
	//(2)连接服务器
	SOCKADDR_IN           server_in;
	memset(&server_in, 0, sizeof(SOCKADDR_IN));
	server_in.sin_family = AF_INET;
	server_in.sin_port = htons(80);
	server_in.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");

	if (connect(sClient, (struct sockaddr*) & server_in, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) 
	{
		AfxMessageBox(_T("connect()失败！"));
		closesocket(sClient);
		return;
	}
	//(3)一般来讲可以设置个收发数据超时时间，一面send(),recv()函数调用时被卡住
	int iSendRecvTimeOut = 5000;
	if (setsockopt(sClient, SOL_SOCKET, SO_SNDTIMEO, (const char*)&iSendRecvTimeOut, sizeof(int)) == SOCKET_ERROR) 
	{
		AfxMessageBox(_T("setsockopt(SO_SNDTIMEO)失败！"));
		closesocket(sClient);
		return;
	}
	if (setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&iSendRecvTimeOut, sizeof(int)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("setsockopt(SO_RCVTIMEO)失败！"));
		closesocket(sClient);
		return;
	}
	AfxMessageBox(_T("connect()成功.."));
}

//发包函数
int SendData(SOCKET sSocket, char *p_sendbuf, int ibuflen)
{
	int usend  = ibuflen;
	int uwrote = 0;
	int tmp_sret;
	while(uwrote < usend)
	{
		tmp_sret = send(sSocket, p_sendbuf + uwrote, usend - uwrote, 0);
		if((tmp_sret == SOCKET_ERROR)||(tmp_sret==0))
		{
			return SOCKET_ERROR;
		}
		uwrote += tmp_sret;
	}
	return uwrote;
}
//结构定义
#pragma pack(1)
typedef struct _COMM_PKG_HEADER
{
	unsigned short pkgLen;
	unsigned short msgCode;
	int            crc32;
}COMM_PKG_HEADER,*LPCOMM_PKG_HEADER;

typedef struct _STRUCT_REGISTER
{
	int         iType;
	char        username[56];
	char        password[40];
}STRUCT_REGISTER,*LPSTRUCT_REGISTER;

typedef struct _STRUCT_LOGIN
{
	char username[56];
	char password[40];
}STRUCT_LOGIN,*LPSTRUCT_LOGIN;
#pragma pack(1)
int g_iLenPkgHeader = sizeof(COMM_PKG_HEADER);
void CSendPkgTestDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	char* p_sendbuf = (char*) new char[g_iLenPkgHeader + sizeof(STRUCT_REGISTER)];
	//注册命令
	LPCOMM_PKG_HEADER               pinfohead;
	pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
	pinfohead->msgCode = 1;
	pinfohead->msgCode = htons(pinfohead->msgCode);
	pinfohead->crc32 = htonl(123);
	pinfohead->pkgLen = htons(g_iLenPkgHeader + sizeof(STRUCT_REGISTER));

	LPSTRUCT_REGISTER pstruc_sendstruc = (LPSTRUCT_REGISTER)(p_sendbuf + g_iLenPkgHeader);
	pstruc_sendstruc->iType = htonl(100);
	strcpy_s(pstruc_sendstruc->username, "1234");
	if(SendData(sClient,p_sendbuf,g_iLenPkgHeader+sizeof(STRUCT_REGISTER))==SOCKET_ERROR)
	{
		AfxMessageBox(_T("SendData()失败!"));
		delete[]p_sendbuf;
		return;
	}
	delete[]p_sendbuf;

	//登录命令
	p_sendbuf = (char*) new char[g_iLenPkgHeader + sizeof(STRUCT_LOGIN)];
	pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
	pinfohead->msgCode = 2;
	pinfohead->msgCode = htons(pinfohead->msgCode);
	pinfohead->crc32 = htonl(345);
	pinfohead->pkgLen = htons(g_iLenPkgHeader + sizeof(STRUCT_LOGIN));

	LPSTRUCT_LOGIN pstruc_sendstruc2 = (LPSTRUCT_LOGIN)(p_sendbuf + g_iLenPkgHeader);
	strcpy_s(pstruc_sendstruc2->username, "5678");
	if (SendData(sClient, p_sendbuf, g_iLenPkgHeader + sizeof(STRUCT_LOGIN)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("SendData()失败!"));
		delete[]p_sendbuf;
		return;
	}
	delete[]p_sendbuf;
	AfxMessageBox(_T("发送两个成功.."));
}

/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "Socket.h"

int Socket_init(){
	// ソケットに関する情報を格納する構造体
	WSADATA wsaData;

	// WinSock2を初期化
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		fprintf(stderr, "Socket: WSAStartup error\n");
		WSACleanup();
		return -1;
	}
	return 0;
}

void Socket_release(){
	// WinSock2のクリーンアップ
	WSACleanup();
}

// サーバーソケットの作成．待ち受けソケットを返す．
SOCKET Socket_server(u_short port){

	// 待ち受け用（リスン）ソケット
	SOCKET listen_s;

	// 接続先の端末に関する情報を格納する構造体
	SOCKADDR_IN saddr;

	bool yes = 1;

	// 待ち受け用（リスン）ソケットを作成（オープン）
	listen_s = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_s < 0) {
		fprintf(stderr, "Socket: error: cannot open socket.\n");
		WSACleanup();
		return -2;
	}
	else {
		// fprintf(stderr, "Socket: Socket is opened.\n");
	}

	// SOCKADDR_IN構造体を設定
	memset(&saddr, 0, sizeof(SOCKADDR_IN));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = INADDR_ANY;

	setsockopt(listen_s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	// ソケットに名前をつける
	if (bind(listen_s, (struct sockaddr *)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		fprintf(stderr, "Socket: bind error");
		closesocket(listen_s);
		WSACleanup();
		return -3;
	}
	else {
		// fprintf(stderr, "Socket: bind: Success\n");
	}
	// クライアントからの接続待ち状態にする
	if (listen(listen_s, 0) == SOCKET_ERROR) {
		fprintf(stderr, "Socket: listen error\n");
		closesocket(listen_s);
		WSACleanup();
		return -4;
	}
	else {
		// fprintf(stderr, "Socket: listen succeeded.\n");
	}
	return listen_s;
}

SOCKET Socket_client(char* server, u_short port){

	HOSTENT *lpHost;
	unsigned int addr;
	SOCKADDR_IN saddr;

	SOCKET s;

	// ホスト情報を獲得
	lpHost = gethostbyname(server);
	if (lpHost == NULL) {
		addr = inet_addr(server);
		lpHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	}
	if (lpHost == NULL) {
		fprintf(stderr, "Socket: cannot get information of host\n");
		WSACleanup();

		return -3;
	}

	// 接続用ソケットの作成（オープン）
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		fprintf(stderr, "Socket: cannot open socket.\n");
		WSACleanup();

		return -2;
	}

	// SOCKADDR_IN構造体の設定
	memset(&saddr, 0, sizeof(SOCKADDR_IN));
	saddr.sin_family = lpHost->h_addrtype;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = *((u_long *)lpHost->h_addr);
	// クライアントソケットをサーバソケットに接続
	if (connect(s, (SOCKADDR *)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		fprintf(stderr, "Socket: Connect: Failed\n");
		closesocket(s);
		WSACleanup();
		return -4;
	}
	else {
		// fprintf(stderr, "Socket: connect: OK\n");
	}

	return s;
}

void Socket_close(SOCKET s){
	shutdown(s, SD_BOTH);

	// 通信用ソケットを閉じる
	closesocket(s);

}

SOCKET Socket_accept(SOCKET listen_sock){

	SOCKADDR_IN from;
	// クライアントからの接続を待機する
	int fromlen = (int)sizeof(from);
	SOCKET s = accept(listen_sock, (SOCKADDR *)&from, &fromlen);

	if (s == INVALID_SOCKET) {

		//fprintf(stderr, "INVALID_SOCKET: Accept in Socket.cpp\n");
		//closesocket(listen_sock);
		//WSACleanup();
		//return s;

	}
	else {
		// fprintf(stderr, "Connected by %s\n", inet_ntoa(from.sin_addr));
	}
	return s;
}

void Socket_setNonBlockingMode(SOCKET sock){
	// ソケットを非ブロッキングモードにする
	unsigned long argp = 1;
	ioctlsocket(sock, FIONBIO, &argp);
}

void Socket_setBlockingMode(SOCKET sock){
	// ソケットをブロッキングモードにする
	unsigned long argp = 0;
	ioctlsocket(sock, FIONBIO, &argp);
}

/* readn - read exactly n bytes */
// nバイトのデータを過不足なく受信するための関数
int Socket_recvn(SOCKET sock, unsigned char *bp, int len)
{
	int cnt;
	int rc;
	int err;

	cnt = len;
	while (cnt > 0) {
		rc = recv(sock, (char *)bp, cnt, 0);
		
		//fprintf(stderr, "rc = %d\n", rc);

		/* @.bp */
		if (rc < 0) {		/* read error? */
			err = WSAGetLastError();
			//fprintf(stderr, "Socket_recvn: failed: %d\n", err);
			// if (err == WSAEINTR || err == WSAEWOULDBLOCK) {	/* interrupted? */
			if (err == WSAEINTR) {	/* interrupted? */
				continue;		/* restart the read */
			}
			return -1;		/* return error */
		}
		if (rc == 0) {		/* EOF? */
			return len - cnt;		/* return short count */
		}
		bp += rc;
		cnt -= rc;
	}
	return len;
}

int Socket_send(SOCKET sock, unsigned char* data, int len) {
	int ret = send(sock, (char*)data, len, 0);
	return ret;
}

int Socket_sendIntData(SOCKET sock, int data){
	int ret = send(sock, (char*)&data, sizeof(int), 0);
	return ret;
}

int Socket_recvIntData(SOCKET sock, int* data){
	int ret = Socket_recvn(sock, (unsigned char*)&data, sizeof(int));
	return ret;
}
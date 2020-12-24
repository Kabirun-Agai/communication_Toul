/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "Command.h"

// 色テーブル(Blue, Green, Red)
cv::Scalar color[NUM_OF_COLOR] = {
		cv::Scalar(255, 255, 255), // 0 white
		cv::Scalar(  0, 128, 128), // 1 olive
		cv::Scalar(  0, 255, 255), // 2 yellow
		cv::Scalar(255,   0, 255), // 3 fuchsia
		cv::Scalar(192, 192, 192), // 4 silver
		cv::Scalar(255, 255,   0), // 5 aqua
		cv::Scalar(  0, 255,   0), // 6 lime
		cv::Scalar(  0,   0, 255), // 7 red
		cv::Scalar(128, 128, 128), // 8 gray
		cv::Scalar(255,   0,   0), // 9 blue
		cv::Scalar(  0, 128,   0), // 10 green
		cv::Scalar(128,   0, 128), // 11 purple
		cv::Scalar(  0,   0,   0), // 12 black
		cv::Scalar(128,   0,   0), // 13 navy
		cv::Scalar(128, 128,   0), // 14 teal
		cv::Scalar(  0,   0, 128) // 15 maroon
		};

#if 0
// コマンド（１バイト）を送信するための関数
int Command_send(SOCKET s, unsigned char command){
	int nSnd;

	// fprintf(stderr, "Client: send command = 0x%02x\n", command);

	nSnd = send(s, (char*)&command, sizeof(char), 0);
	if (nSnd == SOCKET_ERROR){
		int errorNo = WSAGetLastError();
		fprintf(stderr, "Error No: %d in sendCommand. Command: 0x%02x\n", errorNo, command);
	}

	return nSnd;
}

// コマンド（１バイト）を受信するための関数
int Command_recv(SOCKET s, unsigned char* command){
	unsigned char cmd;
	
	int nRcv = Socket_recvn(s, (char*)&cmd, sizeof(unsigned char));
	
	if (nRcv == SOCKET_ERROR){
		int errorNo = WSAGetLastError();
		//fprintf(stderr, "Commnad_recv: Error No: %d\n", errorNo);
		*command = 0xff;
	}
	else {
		*command = cmd;
	}

	//fprintf(stderr, "Server: Received command = 0x%02x\n", *command);

	//fprintf(stderr, "nRcv = %d\n", nRcv);

	return nRcv;
}
#endif

#if 0
// ユーザー名（配列）を送信するための関数
int SendUsername(SOCKET sock, char* user_name){
	int nSnd;

	// fprintf(stderr, "Client: send command = 0x%02x\n", command);

	nSnd = send(sock, (char*)user_name, MAX_USERNAME_LEN, 0);
	if (nSnd == SOCKET_ERROR){
		int errorNo = WSAGetLastError();
	}

	return nSnd;
}

// ユーザー名（配列）を受信するための関数
int RecvUsername(SOCKET sock, char* user_name){
	int errorNo = 0;

	int nRcv = Socket_recvn(sock, (char*)user_name, MAX_USERNAME_LEN);

	if (nRcv == SOCKET_ERROR){
		errorNo = WSAGetLastError();
	}

	return nRcv;
}

#endif
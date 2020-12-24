/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "Command.h"
#include "Database.h"
#include "ChatData.h"
#include "MouseData.h"
#include "StampData.h"
#include "ImageData.h"
#include "PacketData.h"
#include <Ws2def.h>
#include <ws2ipdef.h>
#include <stdlib.h>
#include <signal.h>

#pragma comment(lib, "ws2_32.lib")

#if _DEBUG
#pragma comment(lib, "opencv_world430d.lib")
#pragma comment(lib, "../x64/Debug/LibSP.lib")
#else
#pragma comment(lib, "opencv_world430.lib")
#pragma comment(lib, "../x64/Release/LibSP.lib")
#endif

Database database; // [MAX_USERNUM];
HANDLE hHandle[MAX_USERNUM];

char* winname = "Integrated Server";

// 起動時オプションの表示
int usage(){
	printf("command port\n");
	return -1;
}

// 全ての登録メンバにデータを送信
// databaseを線形探索し，ユーザ登録されていればパケットデータを送信
int PacketData_send_all(PacketData* packet_data)
{
	int ret = -1;

	
	for (int idx = 0; idx < MAX_USERNUM; idx++) {
		// ユーザが登録されていれば
		if (database.entry[idx].sock != -1){
			
			// データベースに登録されたユーザ構造体のソケットにパケットデータを送信
			ret = PacketData_send(database.entry[idx].sock, packet_data);
			
			if (ret == SOCKET_ERROR) {
				fprintf(stderr, "Integrated Server: failed of PacketData_send_all (%d)\n", WSAGetLastError());
			}
		}
	}
	
	return ret;
}

// サーバ（サービス）スレッド
unsigned __stdcall service_thread(void* p){

	// 通信ソケットを受け取る
	SOCKET sock = *(SOCKET*)p;

	// 非ブロッキングモードにする
	Socket_setNonBlockingMode(sock);

	bool connected = true;

	// ログイン・ログアウトメッセージ用
	ChatData chat_data;

	// パケットデータ送受信用
	PacketData packet_data;
	PacketData_init(&packet_data);

	while (connected){

		int ret;
		
		// ユーザデータ用
		UserData user_data;

		// パケットデータをクリア
		PacketData_clear(&packet_data);

		// パケットデータを受信（非同期）
		ret = PacketData_recv(sock, &packet_data);
		
		//fprintf(stderr, "Integrated Server: ret = %d in server_socket\n", ret);

		if (sock == INVALID_SOCKET) {
			fprintf(stderr, "Integrated Server: error of PacketData_recv in server_thread\n");
		}
		else if (ret == 0) {
			// クライアントが（エラーなどで）切断したとき
			// ユーザデータベースから削除
			Database_unregist(&database, &user_data);
		}
		else if (ret > 0) {

			switch (packet_data.cmd) {
			case CMD_USERDATA_REGIST: // コマンドがユーザ登録のとき

				// 受信したパケットデータをユーザデータ構造体に変換
				PacketData2UserData(&packet_data, &user_data);

				// 自身のソケットを登録しておく（データベース上での探索のため）
				user_data.sock = sock;

				//UserData_print(&user_data);

				// ユーザデータベースに登録
				ret = Database_regist(&database, &user_data);
				
				// ログインメッセージを作成
				ChatData_make(&chat_data, 12, user_data.user_name, "Login");
				ChatData2PacketData(&chat_data, &packet_data);

				// 登録ユーザ全員に送信
				PacketData_send_all(&packet_data);

				break;
			case CMD_USERDATA_UNREGIST:	// コマンドがユーザ削除のとき
				
				// ユーザデータベースから削除
				Database_unregist(&database, &user_data);

				// ログアウトメッセージを作成
				ChatData_make(&chat_data, 12, user_data.user_name, "Logout"); 
				ChatData2PacketData(&chat_data, &packet_data);
				
				// 登録ユーザ全員に送信
				PacketData_send_all(&packet_data);

				connected = false;
				break;
			case DAT_CHATDATA:  // メッセージデータのとき
			case DAT_MOUSEDATA: // マウスポインタデータのとき
			case DAT_STAMPDATA: // スタンプデータのとき
			case DAT_IMAGEDATA: // 画像データのとき
				// 登録ユーザ全員に送信
				PacketData_send_all(&packet_data);
				break;
			default:
				fprintf(stderr, "IntegretedServer: unknown command (0x%02x).\n", packet_data.cmd);
				break;
			}
		}
	}

	PacketData_release(&packet_data);

	// 通信用ソケットを閉じる
	Socket_close(sock);

	_endthreadex(0);
	return 0;
}

// アクセプトスレッド
unsigned __stdcall accept_thread(void* p) {

	// 待ち受けソケットを受け取る
	SOCKET listen_sock = *(SOCKET*)p;

	SOCKET sock;

	Database_init(&database);

	// ソケットを非ブロッキングモードにする
	// Socket_setNonBlockingMode(listen_sock);

	fprintf(stdout, "=========================================================================\n");
	fprintf(stdout, " Integrated Server                                        ver.2020-08-18 \n");
	fprintf(stdout, " Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.\n");
	fprintf(stdout, "=========================================================================\n");
	
	// 永久ループ
	while (database.running) {

		fprintf(stdout, "Integrated Server: waiting for connecting..\n");
		// 接続待ち(ブロック)
		sock = Socket_accept(listen_sock);
		
		if (sock == INVALID_SOCKET) {
			fprintf(stderr, "IntegratedServr: Error: Socket_accept\n");
			break;
		}
		else {
			// ユーザデータベースに空きがあるか検索
			int idx = Database_search_empty(&database);
			if (idx >= 0) {
				fprintf(stderr, "Integrated Server: accepted new user. idx = %d\n", idx);
				hHandle[idx] = (HANDLE)_beginthreadex(NULL, 0, service_thread, (void*)&sock, 0, NULL);
			}
			else {
				Socket_close(sock);
				fprintf(stderr, "Integrated Server: cannot register User bacause MAX_USERNUM = %d exceeded\n", MAX_USERNUM);
			}
		}
	}

	// データベース用リソースを解放
	Database_release(&database);

	WaitForMultipleObjects(MAX_USERNUM, hHandle, TRUE, INFINITE);

	_endthreadex(0);

	return 0;

}

void CallbackSingnalControl(int sig) {
	if (sig == SIGINT) {
		signal(SIGINT, CallbackSingnalControl);
		fprintf(stderr, "Integrated Server: catch signal %d\n", sig);
	}
	exit(-1);
}

int main(int argc, char* argv[]){

	signal(SIGINT, CallbackSingnalControl);

	// 引数がプログラム本体を含めて2個（実質1個）かどうかチェック
	if (argc != 2){
		usage();
		return -1;
	}
	// 第一引数をポート番号として取得
	int port = atoi(argv[1]);

	/***** サーバー用初期設定 *****/
	// ソケット用リソースの初期化（サーバ，クライアント共用）
	Socket_init();

	// サーバーソケット（待ち受け用）の作成
	SOCKET listen_sock = Socket_server(port);

	/*** ソケットオプションの設定 (今回は調査しなくてよい) ***/
	int option = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&option, sizeof(option));
	option = 3;
	setsockopt(listen_sock, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&option, sizeof(option));
	option = 2;
	setsockopt(listen_sock, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&option, sizeof(option));
	option = 5;
	setsockopt(listen_sock, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&option, sizeof(option));
	
	// データベースにリスンソケットを設定
	database.listen_sock = listen_sock;
	
	HANDLE keyHandle = (HANDLE)_beginthreadex(NULL, 0, Database_keyinput_thread, (void*)&database, 0, NULL);
	
	HANDLE acceptHandle = (HANDLE)_beginthreadex(NULL, 0, accept_thread, (void*)&listen_sock, 0, NULL);

	WaitForSingleObject(keyHandle, INFINITE);
	WaitForSingleObject(acceptHandle, INFINITE);
	
	// ソケット用リソースを解放（サーバ，クライアント共用）
	Socket_release();

	return 0;
}
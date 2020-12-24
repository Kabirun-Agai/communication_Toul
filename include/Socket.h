/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef SOCKET_H
#define SOCKET_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

// 説明：ソケット通信用システムを初期化する．サーバ―・クライアント共用．
// 引数：なし
// 戻り値：エラーのとき -1，成功のとき 0
int Socket_init();

// 説明：ソケット通信の利用を終了し，プログラムを終了する際に呼び出す．サーバー・クライアント共用．
// 引数：なし
// 戻り値：なし
void Socket_release();

// 説明：サーバー用待ち受けソケットを作成する．
// 引数：port 待ち受け用ポート番号（整数）
// 戻り値：サーバー側の待ち受け用ソケット
SOCKET Socket_server(u_short port);

// 説明：サーバー用待ち受けソケットに対する接続要求を待ち，ブロックする．
// 引数：losten_sock 待ち受け用ソケット
// 戻り値：サーバー側の通信用ソケット
SOCKET Socket_accept(SOCKET listen_sock);

// 説明：クライアント用通信ソケットを作成する．
// 引数：server ホスト名あるいはIPアドレス（文字列)
// 引数：port サーバーのポート番号(整数)
// 戻り値：クライアント側の通信用ソケット
SOCKET Socket_client(char* server, u_short port);

// 説明：ソケットを閉じる（サーバー・クライアント共用，待ち受け・通信共用）
// 引数：sock 閉じるソケット
// 戻り値：なし
void Socket_close(SOCKET sock);

// 説明：ソケットから指定されたバイト数のデータを正確に読み出す
// 引数：sock 通信用ソケット
// 引数：bp 読み出す先のchar型配列へのポインタ
// 引数：len 読み出すバイト数
// 戻り値：正常終了時　読み出したバイト数（引数lenと等しい）
// 戻り値：ソケットが閉じられたとき　0
// 戻り値：読み出せなかったとき -1
int Socket_recvn(SOCKET sock, unsigned char *bp, int len);

// 説明：ソケットに指定されたバイト数のデータを送信する
// 引数：sock 通信用ソケット
// 引数：bp 送信するデータを持つchar型配列へのポインタ
// 引数：len 送信するバイト数
// 戻り値：正常終了時　送信したバイト数
// 戻り値：送信できなかったとき -1
int Socket_send(SOCKET sock, unsigned char *bp, int len);

// 説明：ソケットからint型データを一つ送信する
// 引数：sock 通信用ソケット
// 引数：data 送信するint型データ
// 戻り値：送信したデータのバイト数 sizeof(int)
int Socket_sendIntData(SOCKET sock, int data);

int Socket_recvIntData(SOCKET sock, int* data);

// 説明：ソケットを非ブロッキングモードにする
// 引数：sock 非ブロッキングモードにするソケット
// 戻り値：なし
void Socket_setNonBlockingMode(SOCKET sock);

// 説明：ソケットをブロッキングモードにする
// 引数：sock ブロッキングモードにするソケット
// 戻り値：なし
void Socket_setBlockingMode(SOCKET sock);

#endif /* SOCEKT_H */
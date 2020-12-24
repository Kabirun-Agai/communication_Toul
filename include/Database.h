/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef DATABASE_H
#define DATABASE_H

#include <Winsock2.h>
#include <Windows.h>
#include <conio.h>
#include <wincon.h>
#include <process.h>
#include <stdio.h>

#include "Socket.h"
#include "UserData.h"

#define MAX_USERNUM 10 // 接続可能最大ユーザー数
#define MAX_NUM_IN_ROW 10 // Database_print関数における1行あたりの最大表示数

// ユーザ管理データベース構造体
typedef struct Database {
	HANDLE hMutex; // 相互排除用
	SOCKET listen_sock; 
	bool registered; // 1名以上登録者がいればtrue
	bool running; // 実行中であるかどうかを示すフラグ
	UserData entry[MAX_USERNUM]; // 格納されるユーザデータの配列
} Database;

// ユーザデータベース内のエントリ状況をテキスト表示
int Database_print(Database* database);

// ユーザデータベース内で空いているエントリのインデックスを検索
// 空きエントリがあるとき：インデックス(0 <-> (MAX_USER - 1))を返す
// 空きエントリがないとき：-1を返す
int Database_search_empty(Database* database);

// ユーザー管理データベースを初期化（全てのエントリを登録なしにする）
void Database_init(Database* database);

// ユーザ管理データベースのリソースの解放
void Database_release(Database* database);

// ユーザーデータをユーザ管理データベースに登録
int Database_regist(Database* database, UserData* user_data);

// ユーザーデータをユーザ管理データベースから削除
int Database_unregist(Database* database, UserData* user_data);

// ユーザ管理データベースによるキー入力待ち受けスレッド用関数
unsigned __stdcall Database_keyinput_thread(void* p);

#endif /* DATABASE_H */
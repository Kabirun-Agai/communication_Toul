/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "Database.h"

// メンバー配列のエントリ状況を表示
int Database_print(Database* database){

	WaitForSingleObject(database->hMutex, INFINITE);
	fprintf(stdout, "--------------------------------\n");
	for(int row = 0 ; row < MAX_USERNUM / MAX_NUM_IN_ROW ; row++){
		fprintf(stdout, "No:   ");
		for (int i = 0; i < MAX_NUM_IN_ROW; i++){
			fprintf(stdout, "%10d ", MAX_NUM_IN_ROW * row + i);
		}
		fprintf(stdout, "\n");

		fprintf(stdout, "Name: ");
		for (int i = 0; i < MAX_NUM_IN_ROW; i++){
			fprintf(stdout, "%10s ", database->entry[MAX_NUM_IN_ROW * row + i].user_name);
		}
		fprintf(stdout, "\n");
	}
	ReleaseMutex(database->hMutex);
	return 0;
}

// メンバー配列内の未登録エントリのインデックスを検索
// インデックス(0 <-> (MAX_USER - 1))を返す
int Database_search_empty(Database* database){
	int idx = 0;
	WaitForSingleObject(database->hMutex, INFINITE);
	for (idx = 0; idx < MAX_USERNUM; idx++) {
		if (database->entry[idx].sock == -1) {
			break;
		}
	}
	ReleaseMutex(database->hMutex);

	// 空きエントリが見つからなければ（登録不可状態）-1を返す．
	return idx == MAX_USERNUM ? -1 : idx;
}

// ユーザーデータベースを初期化（全てのエントリを登録なしにする）
void Database_init(Database* database){
	database->hMutex = CreateMutex(NULL, FALSE, NULL);
	database->registered = false;
	database->running = true;
	for (int idx = 0; idx < MAX_USERNUM; idx++) {
		// ソケットを-1（登録なし）に設定
		database->entry[idx].sock = -1;
		// 名前を'\0'に設定
		memset(database->entry[idx].user_name, '\0', MAX_USERNAME_LEN);
	}
}

void Database_release(Database* database){
	CloseHandle(database->hMutex);
}

// ユーザーデータをデータベースに登録
int Database_regist(Database* database, UserData* user_data)
{
	int idx = -1;

	WaitForSingleObject(database->hMutex, INFINITE);
	// 空きのインデックスを探す
	idx = Database_search_empty(database);

	// 空きがあったとき
	if (0 <= idx && idx < MAX_USERNUM){
		database->registered = true;
		database->entry[idx].sock = user_data->sock;
		sprintf_s(database->entry[idx].user_name, MAX_USERNAME_LEN, "%s", user_data->user_name);
	}
	ReleaseMutex(database->hMutex);

	if (idx != -1){
		fprintf(stderr, "Database: userdata registered. idx (%d), user_name(%s)\n", idx, user_data->user_name);
	}
	else{
		fprintf(stderr, "Database: cannot regist %s\n", user_data->user_name);
	}

	return idx;
}

// ユーザーデータをデータベースから削除
int Database_unregist(Database* database, UserData* data)
{
	int ret = -1;
	WaitForSingleObject(database->hMutex, INFINITE);
	for(int idx = 0 ; idx < MAX_USERNUM ; idx++){
		if (database->entry[idx].sock == data->sock){
			// ソケットを-1にする
			database->entry[idx].sock = -1;
			memset(database->entry[idx].user_name, '\0', MAX_USERNAME_LEN);
			ret = idx;
			break;
		}
	}

	// 登録ユーザが一人もいなければ，registered  = falseにセット
	database->registered = false;
	for(int idx = 0 ; idx < MAX_USERNUM ; idx++){
		if (database->entry[idx].sock != -1){
			database->registered = true;
			break;
		}
	}
	ReleaseMutex(database->hMutex);
	// 削除対象メンバが存在していなければ-1, 存在していればそのインデックスを返す
	return ret;
}

// キー入力待ち受けスレッド用関数
unsigned __stdcall Database_keyinput_thread(void* p){
	Database* database = (Database*)p;

	for(;;){

		int c = _getch();
		//fprintf(stderr, "c = %c\n", c);
		if ((char)c == 'p'){
			Database_print(database);
		}
		else if ((char)c == 'q'){
			WaitForSingleObject(database->hMutex, INFINITE);
			if (database->registered == false){
				database->running = false;
				// 待ち受け用ソケットを閉じる
				Socket_close(database->listen_sock);
				fprintf(stderr, "IntegratedServer: Terminated.\n");
				break;
			}else{
				fprintf(stderr, "IntegratedServer: Cannot exit the server because there are some registered users.\n");
			}
			ReleaseMutex(database->hMutex);
		}
	}

	_endthreadex(0);

	return 0;
}

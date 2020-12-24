/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef USERDATA_H
#define USERDATA_H

#include "Socket.h"
#include "PacketData.h"
#include "Command.h"

#define MAX_USERNAME_LEN 64 // bytes

// ユーザーデータ
typedef struct UserData {
	SOCKET sock;            // サーバーあるいはクライアントと通信するためのソケット
	char user_name[MAX_USERNAME_LEN]; // ユーザー名
} UserData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/

/* ユーザ名からUserDataを生成する */
// idx = -1, sock = -1にセットされる
void UserData_make(char* user_name, UserData* user_data); 

// cmd = CMD_USERDATA_REGIST or CMD_USERDATA_UNREGIST
void UserData2PacketData(unsigned char cmd, UserData* src, PacketData* dst);
void PacketData2UserData(PacketData* src, UserData* dst);

void UserData_clear(UserData* user_data);
void UserData_copy(UserData* src, UserData* dst);

// ユーザーデータのテキスト情報表示
void UserData_print(UserData* data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

// UserData構造体から配列へのシリアライズ関数
void UserData_serialize(UserData* user_data, PacketData* packet_data);

// 配列からUser構造体へのデシリアライズ関数
void UserData_deserialize(PacketData* packet_data, UserData* user_data);




#endif /* USERDATA_H */
/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef CHATDATA_H
#define CHATDATA_H

#include <process.h>
#include "Socket.h"
#include "Command.h"
#include "UserData.h"
#include "PacketData.h"

#define MAX_MESSAGE_LEN 128 // bytes

/* ChatData構造体 */
typedef struct ChatData {
	int color_idx;                    // 色インデックス(Command.h参照)
	char user_name[MAX_USERNAME_LEN]; // ユーザ名
	char message[MAX_MESSAGE_LEN];    // メッセージ
} ChatData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/

// 色IDX, ユーザ名, メッセージからChatDataを構築する
void ChatData_make(ChatData* data, int color_idx, char* user_name, char* message);

// PacketDataへの変換関数
void ChatData2PacketData(ChatData* src, PacketData* dst);

// PacketDataからの変換関数
void PacketData2ChatData(PacketData* src, ChatData* dst);

void ChatData_clear(ChatData* chat_data);
void ChatData_copy(ChatData* src, ChatData* dst);

// ChatDataの情報を画面に表示
void ChatData_print(ChatData* chat_data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

// ChatDataをシリアライズ
void ChatData_serialize(ChatData* chat_data, PacketData* packet_data);

// ChatDataをデシリアライズ
void ChatData_deserialize(PacketData* packet_data, ChatData* chat_data);

#endif /* CHATDATA_H */
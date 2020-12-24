/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "ChatData.h"

// メッセージの生成
void ChatData_make(ChatData* data, int color_idx, char* user_name, char* message){

	data->color_idx = color_idx;

	memset(data->user_name, '\0', MAX_USERNAME_LEN);
	memcpy(data->user_name, user_name, strlen(user_name));

	memset(data->message, '\0', MAX_MESSAGE_LEN);
	memcpy(data->message, message, strlen(message));
}

void ChatData2PacketData(ChatData* src, PacketData* dst) {

	dst->cmd = DAT_CHATDATA;
	dst->size = sizeof(int) + MAX_USERNAME_LEN + MAX_MESSAGE_LEN;
	ChatData_serialize(src, dst);
}

void PacketData2ChatData(PacketData* src, ChatData* dst) {
	ChatData_deserialize(src, dst);
}

void ChatData_clear(ChatData* chat_data) {
	chat_data->color_idx = 12; // black. check in Command.h
	memset(chat_data->user_name, '\0', MAX_USERNAME_LEN);
	memset(chat_data->message, '\0', MAX_MESSAGE_LEN);
}

void ChatData_copy(ChatData* src, ChatData* dst) {
	dst->color_idx = src->color_idx;
	memcpy(dst->user_name, src->user_name, MAX_USERNAME_LEN);
	memcpy(dst->message, src->message, MAX_MESSAGE_LEN);
}

void ChatData_print(ChatData* chat_data) {
	fprintf(stderr, "ChatData_print: user_name = %s, message = %s, color_idx = %d\n",
		chat_data->user_name, chat_data->message, chat_data->color_idx);
}

// ChatData構造体のシリアライズ関数
void ChatData_serialize(ChatData* chat_data, PacketData* packet_data){
	
	memset(packet_data->binary, '\0', DATA_W * DATA_H * DATA_C);

	// メッセージータのシリアライズ
	memcpy(&packet_data->binary[0], &chat_data->color_idx, sizeof(int));
	memcpy(&packet_data->binary[sizeof(int)], chat_data->user_name, MAX_USERNAME_LEN);
	memcpy(&packet_data->binary[sizeof(int) + MAX_USERNAME_LEN], chat_data->message, MAX_MESSAGE_LEN);
}

// ChatData構造体のデシリアライズ関数
void ChatData_deserialize(PacketData* packet_data, ChatData* chat_data){

	// メッセージータのデシリアライズ
	memcpy(&chat_data->color_idx, &packet_data->binary[0], sizeof(int));
	memcpy(chat_data->user_name, &packet_data->binary[sizeof(int)], MAX_USERNAME_LEN);
	memcpy(chat_data->message, &packet_data->binary[sizeof(int) + MAX_USERNAME_LEN], MAX_MESSAGE_LEN);
	
}
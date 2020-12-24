/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "UserData.h"

void UserData_make(char* user_name, UserData* data){
	data->sock = -1;
	memset(data->user_name, '\0', MAX_USERNAME_LEN);
	if (user_name != NULL) {
		memcpy(data->user_name, user_name, strlen(user_name));
	}
}

void UserData2PacketData(unsigned char cmd, UserData* src, PacketData* dst) {
	dst->cmd = cmd; // CMD_USERDATA_REGIST or CMD_USERDATA_UNREGIST
	if (cmd == CMD_USERDATA_REGIST) {
		dst->size = sizeof(SOCKET) + (int)strlen(src->user_name);
		UserData_serialize(src, dst);
	}
	else {
		dst->size = 0;
	}
}

void PacketData2UserData(PacketData* src, UserData* dst) {
	UserData_deserialize(src, dst);
}

void UserData_clear(UserData* user_data) {
	user_data->sock = NULL;
	memset(user_data->user_name, '\0', MAX_USERNAME_LEN);
}

void UserData_copy(UserData* src, UserData* dst) {
	dst->sock = src->sock;
	memcpy(dst->user_name, src->user_name, MAX_USERNAME_LEN);
}

void UserData_serialize(UserData* user_data, PacketData* packet_data){
	memset(packet_data->binary, '\0', DATA_W * DATA_H * DATA_C);
	// ユーザデータのシリアライズ
	memcpy(&packet_data->binary[0], &user_data->sock, sizeof(SOCKET));
	memcpy(&packet_data->binary[sizeof(SOCKET)], &user_data->user_name, strlen(user_data->user_name));
}

void UserData_deserialize(PacketData* packet_data, UserData* user_data){
	// ユーザデータのデシリアライズ
	memcpy(&user_data->sock, &packet_data->binary[0], sizeof(SOCKET));
	memset(user_data->user_name, '\0', MAX_USERNAME_LEN);
	memcpy(user_data->user_name, &packet_data->binary[sizeof(SOCKET)], packet_data->size - sizeof(SOCKET));
}

void UserData_print(UserData* user_data){
	fprintf(stderr, "UserData_print: sock(%lld), user_name(%s)\n", 
		user_data->sock, user_data->user_name);
}


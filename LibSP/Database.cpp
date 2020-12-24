/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "Database.h"

// �����o�[�z��̃G���g���󋵂�\��
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

// �����o�[�z����̖��o�^�G���g���̃C���f�b�N�X������
// �C���f�b�N�X(0 <-> (MAX_USER - 1))��Ԃ�
int Database_search_empty(Database* database){
	int idx = 0;
	WaitForSingleObject(database->hMutex, INFINITE);
	for (idx = 0; idx < MAX_USERNUM; idx++) {
		if (database->entry[idx].sock == -1) {
			break;
		}
	}
	ReleaseMutex(database->hMutex);

	// �󂫃G���g����������Ȃ���΁i�o�^�s��ԁj-1��Ԃ��D
	return idx == MAX_USERNUM ? -1 : idx;
}

// ���[�U�[�f�[�^�x�[�X���������i�S�ẴG���g����o�^�Ȃ��ɂ���j
void Database_init(Database* database){
	database->hMutex = CreateMutex(NULL, FALSE, NULL);
	database->registered = false;
	database->running = true;
	for (int idx = 0; idx < MAX_USERNUM; idx++) {
		// �\�P�b�g��-1�i�o�^�Ȃ��j�ɐݒ�
		database->entry[idx].sock = -1;
		// ���O��'\0'�ɐݒ�
		memset(database->entry[idx].user_name, '\0', MAX_USERNAME_LEN);
	}
}

void Database_release(Database* database){
	CloseHandle(database->hMutex);
}

// ���[�U�[�f�[�^���f�[�^�x�[�X�ɓo�^
int Database_regist(Database* database, UserData* user_data)
{
	int idx = -1;

	WaitForSingleObject(database->hMutex, INFINITE);
	// �󂫂̃C���f�b�N�X��T��
	idx = Database_search_empty(database);

	// �󂫂��������Ƃ�
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

// ���[�U�[�f�[�^���f�[�^�x�[�X����폜
int Database_unregist(Database* database, UserData* data)
{
	int ret = -1;
	WaitForSingleObject(database->hMutex, INFINITE);
	for(int idx = 0 ; idx < MAX_USERNUM ; idx++){
		if (database->entry[idx].sock == data->sock){
			// �\�P�b�g��-1�ɂ���
			database->entry[idx].sock = -1;
			memset(database->entry[idx].user_name, '\0', MAX_USERNAME_LEN);
			ret = idx;
			break;
		}
	}

	// �o�^���[�U����l�����Ȃ���΁Cregistered  = false�ɃZ�b�g
	database->registered = false;
	for(int idx = 0 ; idx < MAX_USERNUM ; idx++){
		if (database->entry[idx].sock != -1){
			database->registered = true;
			break;
		}
	}
	ReleaseMutex(database->hMutex);
	// �폜�Ώۃ����o�����݂��Ă��Ȃ����-1, ���݂��Ă���΂��̃C���f�b�N�X��Ԃ�
	return ret;
}

// �L�[���͑҂��󂯃X���b�h�p�֐�
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
				// �҂��󂯗p�\�P�b�g�����
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

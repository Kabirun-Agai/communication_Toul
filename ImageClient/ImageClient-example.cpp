//#define _CRT_SECURE_NO_WARNINGS
#include "Command.h"
#include "UserData.h"
#include "ImageData.h"

#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Comdlg32.lib")

#if _DEBUG
#pragma comment(lib, "opencv_world430d.lib")
#pragma comment(lib, "../x64/Debug/LibSP.lib")
#else
#pragma comment(lib, "opencv_world430.lib")
#pragma comment(lib, "../x64/Release/LibSP.lib")
#endif

// �z���C�g�{�[�h�C�X�^���v�C�摜�`��p�̉摜�T�C�Y
#define DRAW_AREA_W 640
#define DRAW_AREA_H 480

// �`���b�g�\���p�̉摜�T�C�Y
#define CHAT_AREA_W 640
#define CHAT_AREA_H 240

// �S�̗p�̉摜�T�C�Y
#define CANVAS_W (DRAW_AREA_W)
#define CANVAS_H (DRAW_AREA_H + CHAT_AREA_H)

#define FONT_SIZE 16 // points

#define MAX_WINNAME_LEN 80

cv::Mat canvas;
cv::Mat draw_area;
cv::Mat chat_area;

// false�̂Ƃ��X���b�h���I��
bool connected = true;

// ���[�U�f�[�^(�o�^�C�폜�p)
UserData user_data;

void update_draw_area(cv::Mat& canvas, cv::Mat draw_area) {
	cv::Rect drawrect(0, 0, DRAW_AREA_W, DRAW_AREA_H);
	cv::Mat draw_area_on_canvas = canvas(drawrect);
	draw_area.copyTo(draw_area_on_canvas);
}

void update_chat_area(cv::Mat& canvas, cv::Mat chat_area) {
	cv::Rect chatrect(0, DRAW_AREA_H, CHAT_AREA_W, CHAT_AREA_H);
	cv::Mat chat_area_on_canvas = canvas(chatrect);
	chat_area.copyTo(chat_area_on_canvas);
}

// �N�����I�v�V�����̕\��
int usage(char* program_name) {
	printf("%s server_ip server_port user_name\n", program_name);
	return -1;
}
/****************************************************************/

char winname[MAX_WINNAME_LEN]; // = "Image Client"; // �E�B���h�E��

HWND handle;

/***** �摜�A�b�v���[�h�N���C�A���g *****/
bool update_image_flag = false;

unsigned __stdcall open_file_dialog(void* p) {
	char* return_path = (char*)p;
	char path[MAX_FILE_PATH], name[MAX_FILE_PATH];

	// �t�@�C���_�C�A���O�\���p�̃E�B���h�E�̃n���h�����擾
	HWND hwnd = (HWND)cvGetWindowHandle(winname);

	//������Ƀk�������������Ă���
	memset(path, '\0', sizeof(path));
	memset(name, '\0', sizeof(name));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;     //�I�����ꂽ�t�@�C�������󂯎��(�t���p�X)
	ofn.nMaxFile = MAX_FILE_PATH;
	ofn.lpstrFileTitle = name; //�I�����ꂽ�t�@�C����
	ofn.nMaxFileTitle = MAX_FILE_PATH;

	//fprintf(stderr, "Pass 01\n");

	ofn.lpstrFilter = "jpg(*.jpg)\0*.jpg\0all(*.*)\0*.*\0\0"; //�t�B���^
	ofn.lpstrTitle = "�摜���J��"; //�_�C�A���O�{�b�N�X�̃^�C�g��

	ofn.Flags = OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == FALSE) {
		return FALSE;
	}

	fprintf(stderr, "�I�����ꂽ�t�@�C���̃t���p�X:%s\n", path);
	fprintf(stderr, "�I�����ꂽ�t�@�C����: %s\n", name);

	memcpy(return_path, path, MAX_FILE_PATH);
	update_image_flag = true;

	_endthreadex(0);

	return 0;
}

// �N���C�A���g�X���b�h
unsigned __stdcall client_thread(void* p) {
	int key;

	// �ʐM�\�P�b�g���󂯎��
	SOCKET sock = *(SOCKET*)p;

	// �\�P�b�g���u���b�L���O���[�h�ɂ���
	Socket_setNonBlockingMode(sock);

	char path[MAX_FILE_PATH];

	PacketData packet_data;
	PacketData_init(&packet_data);

	ImageData image_data;
	ImageData_init(&image_data);

	ImageData upload_image;
	ImageData_init(&upload_image);

	fprintf(stderr, "IntegratedClient: client_thread: pass 00\n");

	/***** �h���[�G���A�p�摜�̏��� *****/
	// ���������蓖��
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// ���œh��Ԃ�
	draw_area = cv::Scalar(255, 255, 255);

	/***** �`���b�g�G���A�p�摜�̏��� *****/

	// ���������蓖��
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// ���F�œh��Ԃ�
	chat_area = cv::Scalar(255, 127, 127);

	fprintf(stderr, "IntegratedClient: client_thread: pass 01\n");

	/***** �S�̕\���p�摜�i�L�����o�X�j�̏��� *****/
	// ���������蓖��
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// ���F�œh��Ԃ�
	canvas = cv::Scalar(255, 255, 255);

	// �E�B���h�E���J��
	cv::namedWindow(winname);
	// ���T�C�Y����
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);

	update_draw_area(canvas, draw_area);
	update_chat_area(canvas, chat_area);

	// �摜�̕\���\��
	cv::imshow(winname, canvas);
	// �摜�̕`��&�L�[���͑҂�(����0�̎��u���b�N�C1�ȏ�̂Ƃ��ҋ@����(ms))
	key = cv::waitKey(1);

	while (connected) {

		int ret;

		// PacketData����M
		ret = PacketData_recv(sock, &packet_data);

		if (ret == 0) {
			// �\�P�b�g������ꂽ�Ƃ�
		}
		else if (ret < 0) {
			// �f�[�^����M�̂Ƃ�
		}
		else {
			switch (packet_data.cmd) {
			
			case DAT_IMAGEDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_IMAGEDATA\n");
				PacketData2ImageData(&packet_data, &image_data);

				fprintf(stderr, "Integrated Client: ImageData_recv: size = %d\n", ret);
				image_data.image.copyTo(draw_area);
				// draw_area�̃A�b�v�f�[�g
				update_draw_area(canvas, draw_area);
				break;
			default:
				fprintf(stderr, "Image Client: client_thread: Received Command: 0x%02x\n", packet_data.cmd);
				break;
			}
		}

		if (update_image_flag == true) {
			// �_�C�A���O�Ŏw�肵���摜�t�@�C����ǂݏo����image�ɃZ�b�g����
			cv::Mat image = cv::imread(path);
			if (!image.empty()) {

				// �ǂݏo�����摜��(IMAGE_W, IMAGE_H)�Ƀ��T�C�Y����
				cv::resize(image, upload_image.image, upload_image.image.size(), cv::INTER_CUBIC);

				ImageData_make(&image_data, upload_image.image);
				ImageData2PacketData(&image_data, &packet_data);
				PacketData_print(&packet_data);

				ret = PacketData_send(sock, &packet_data);
			}
			update_image_flag = false;
		}

		// �摜�̕\���\��
		cv::imshow(winname, canvas);
		// �摜�̕\��&�L�[����
		key = cv::waitKey(1);

		if (key == 0x1b || key == 'q' || key == 'Q') {
			/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o����폜 ***/
			// UserData�𐶐�
			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);

			ret = PacketData_send(sock, &packet_data);

			connected = false;
		}
		else if (key == 'o') {

			// �t�@�C���_�C�A���O���J���C���M�������摜��I������
			HANDLE h2 = (HANDLE)_beginthreadex(NULL, 0, open_file_dialog, path, 0, NULL);
			CloseHandle(h2);
		}
		else {

		}
	}

	ImageData_release(&image_data);
	ImageData_release(&upload_image);

	PacketData_release(&packet_data);

	// �L�����o�X�p�����������
	canvas.release();
	draw_area.release();
	chat_area.release();

	_endthreadex(0);

	return 0;
}

int main(int argc, char* argv[]){
	int ret;

	char* server;
	int port;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	PacketData packet_data;
	PacketData_init(&packet_data);

	char user_name[MAX_USERNAME_LEN];

	/* �������� */
	// IP�A�h���X(�K�{)�C�|�[�g�ԍ�(�K�{)�C���[�U��(�K�{), �����F�i�f�t�H���g12�j
	// �������v���O�����{�̂��܂߂�5�i�����݂̂�4�j���ǂ����`�F�b�N
	if (argc == 4) {
		// ���������T�[�o�����邢��IP�A�h���X�Ƃ��Ď擾
		server = argv[1];
		// ���������|�[�g�ԍ��Ƃ��Ď擾
		port = atoi(argv[2]);

		sprintf_s(user_name, MAX_USERNAME_LEN, "%s", argv[3]);
		fprintf(stderr, "Username: %s, size: %zd\n", user_name, strlen(user_name));
	}
	else {
		usage(argv[0]);
		return -1;
	}

	sprintf_s(winname, MAX_WINNAME_LEN, "ImageClient - %s", user_name);

	// ���[�U�f�[�^(�o�^�p)���쐬
	UserData_make(user_name, &user_data);
	// ���[�U�f�[�^����\��
	UserData_print(&user_data);
	// �p�P�b�g�f�[�^�ɕϊ�
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// �p�P�b�g�f�[�^����\��
	PacketData_print(&packet_data);
	// �p�P�b�g�f�[�^���(�o�C�i�����)��\��
	//PacketData_print_hex(&packet_data);

	/***** �N���C�A���g�p�����ݒ� *****/
	// �\�P�b�g�p���\�[�X�̏������i�T�[�o�C�N���C�A���g���p�j
	Socket_init();
	// �N���C�A���g�\�P�b�g�i�ڑ��p�j�̍쐬
	SOCKET sock = Socket_client(server, port);

	/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o�ɓo�^ ***/
	if (sock >= 0) {
		// �p�P�b�g�f�[�^�𑗐M
		ret = PacketData_send(sock, &packet_data);
	}
	else {
		fprintf(stderr, "Integrated Client: Cannot connect to server\n");
		return -1;
	}
	
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// �X���b�h���I������܂őҋ@
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	PacketData_release(&packet_data);

	// �ʐM�p�\�P�b�g�����
	Socket_close(sock);

	// �\�P�b�g�p���\�[�X������i�T�[�o�C�N���C�A���g���p�j
	Socket_release();

	return 0;
}
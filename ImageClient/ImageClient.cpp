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

// ホワイトボード，スタンプ，画像描画用の画像サイズ
#define DRAW_AREA_W 640
#define DRAW_AREA_H 480

// チャット表示用の画像サイズ
#define CHAT_AREA_W 640
#define CHAT_AREA_H 240

// 全体用の画像サイズ
#define CANVAS_W (DRAW_AREA_W)
#define CANVAS_H (DRAW_AREA_H + CHAT_AREA_H)

#define FONT_SIZE 16 // points

#define MAX_WINNAME_LEN 80

cv::Mat canvas;
cv::Mat draw_area;
cv::Mat chat_area;

// falseのときスレッドを終了
bool connected = true;

// ユーザデータ(登録，削除用)
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

// 起動時オプションの表示
int usage(char* program_name) {
	printf("%s server_ip server_port user_name\n", program_name);
	return -1;
}
/****************************************************************/

char winname[MAX_WINNAME_LEN]; // = "Image Client"; // ウィンドウ名

HWND handle;

/***** 画像アップロードクライアント *****/
bool update_image_flag = false;

unsigned __stdcall open_file_dialog(void* p) {
	char* return_path = (char*)p;
	char path[MAX_FILE_PATH], name[MAX_FILE_PATH];

	// ファイルダイアログ表示用のウィンドウのハンドルを取得
	HWND hwnd = (HWND)cvGetWindowHandle(winname);

	//文字列にヌル文字を代入しておく
	memset(path, '\0', sizeof(path));
	memset(name, '\0', sizeof(name));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;     //選択されたファイル名を受け取る(フルパス)
	ofn.nMaxFile = MAX_FILE_PATH;
	ofn.lpstrFileTitle = name; //選択されたファイル名
	ofn.nMaxFileTitle = MAX_FILE_PATH;

	//fprintf(stderr, "Pass 01\n");

	ofn.lpstrFilter = "jpg(*.jpg)\0*.jpg\0all(*.*)\0*.*\0\0"; //フィルタ
	ofn.lpstrTitle = "画像を開く"; //ダイアログボックスのタイトル

	ofn.Flags = OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == FALSE) {
		return FALSE;
	}

	fprintf(stderr, "選択されたファイルのフルパス:%s\n", path);
	fprintf(stderr, "選択されたファイル名: %s\n", name);

	memcpy(return_path, path, MAX_FILE_PATH);
	update_image_flag = true;

	_endthreadex(0);

	return 0;
}

// クライアントスレッド
unsigned __stdcall client_thread(void* p) {
	int key;

	// 通信ソケットを受け取る
	SOCKET sock = *(SOCKET*)p;

	// ソケットを非ブロッキングモードにする
	Socket_setNonBlockingMode(sock);

	char path[MAX_FILE_PATH];

	PacketData packet_data;
	PacketData_init(&packet_data);

	ImageData image_data;
	ImageData_init(&image_data);

	ImageData upload_image;
	ImageData_init(&upload_image);

	fprintf(stderr, "IntegratedClient: client_thread: pass 00\n");

	/***** ドローエリア用画像の準備 *****/
	// メモリ割り当て
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// 白で塗りつぶす
	draw_area = cv::Scalar(255, 255, 255);

	/***** チャットエリア用画像の準備 *****/

	// メモリ割り当て
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// 薄青色で塗りつぶす
	chat_area = cv::Scalar(255, 127, 127);

	fprintf(stderr, "IntegratedClient: client_thread: pass 01\n");

	/***** 全体表示用画像（キャンバス）の準備 *****/
	// メモリ割り当て
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// 薄青色で塗りつぶす
	canvas = cv::Scalar(255, 255, 255);

	// ウィンドウを開く
	cv::namedWindow(winname);
	// リサイズする
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);

	update_draw_area(canvas, draw_area);
	update_chat_area(canvas, chat_area);

	// 画像の表示予約
	cv::imshow(winname, canvas);
	// 画像の描画&キー入力待ち(引数0の時ブロック，1以上のとき待機時間(ms))
	key = cv::waitKey(1);

	while (connected) {

		int ret;

		// PacketDataを受信
		ret = PacketData_recv(sock, &packet_data);

		if (ret == 0) {
			// ソケットが閉じられたとき
		}
		else if (ret < 0) {
			// データ未受信のとき
		}
		else {
			switch (packet_data.cmd) {
			
			case DAT_IMAGEDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_IMAGEDATA\n");
				// PacketDataから画像データ(ImageData)に変換
				/*** ここから ***/
				PacketData2ImageData(&packet_data, &image_data);



				/*** ここまで***/
				fprintf(stderr, "Integrated Client: ImageData_recv: size = %d\n", ret);
				image_data.image.copyTo(draw_area);

				// draw_areaのアップデート
				update_draw_area(canvas, draw_area);
				break;
			default:
				fprintf(stderr, "Image Client: client_thread: Received Command: 0x%02x\n", packet_data.cmd);
				break;
			}
		}

		if (update_image_flag == true) {
			// ダイアログで指定した画像ファイルを読み出してimageにセットする
			cv::Mat image = cv::imread(path);
			if (!image.empty()) {

				// 読み出した画像を(IMAGE_W, IMAGE_H)にリサイズしてupload_image.imageにセット
				cv::resize(image, upload_image.image, upload_image.image.size(), cv::INTER_CUBIC);
				
				// 画像データ(ImageData)を送信
				/*** ここから ***/
				ImageData2PacketData(&upload_image, &packet_data);
				ret = PacketData_send(sock, &packet_data);

				ImageData_print(&upload_image);
				PacketData_print(&packet_data);

				/*** ここまで***/

			}
			update_image_flag = false;
		}

		// 画像の表示予約
		cv::imshow(winname, canvas);
		// 画像の表示&キー入力
		key = cv::waitKey(1);

		if (key == 0x1b || key == 'q' || key == 'Q') {
			/*** ユーザデータをデータベースサーバから削除 ***/
			// UserDataを生成
			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);

			ret = PacketData_send(sock, &packet_data);

			connected = false;
		}
		else if (key == 'o') {

			// ファイルダイアログを開き，送信したい画像を選択する
			HANDLE h2 = (HANDLE)_beginthreadex(NULL, 0, open_file_dialog, path, 0, NULL);
			CloseHandle(h2);
		}
		else {

		}
	}

	ImageData_release(&image_data);
	ImageData_release(&upload_image);

	PacketData_release(&packet_data);

	// キャンバス用メモリを解放
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

	/* 引数処理 */
	// IPアドレス(必須)，ポート番号(必須)，ユーザ名(必須), 文字色（デフォルト12）
	// 引数がプログラム本体を含めて5個（引数のみで4個）かどうかチェック
	if (argc == 4) {
		// 第一引数をサーバ名あるいはIPアドレスとして取得
		server = argv[1];
		// 第二引数をポート番号として取得
		port = atoi(argv[2]);

		sprintf_s(user_name, MAX_USERNAME_LEN, "%s", argv[3]);
		fprintf(stderr, "Username: %s, size: %zd\n", user_name, strlen(user_name));
	}
	else {
		usage(argv[0]);
		return -1;
	}

	sprintf_s(winname, MAX_WINNAME_LEN, "ImageClient - %s", user_name);

	// ユーザデータ(登録用)を作成
	UserData_make(user_name, &user_data);
	// ユーザデータ情報を表示
	UserData_print(&user_data);
	// パケットデータに変換
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// パケットデータ情報を表示
	PacketData_print(&packet_data);
	// パケットデータ情報(バイナリ情報)を表示
	//PacketData_print_hex(&packet_data);

	/***** クライアント用初期設定 *****/
	// ソケット用リソースの初期化（サーバ，クライアント共用）
	Socket_init();
	// クライアントソケット（接続用）の作成
	SOCKET sock = Socket_client(server, port);

	/*** ユーザデータをデータベースサーバに登録 ***/
	if (sock >= 0) {
		// パケットデータを送信
		ret = PacketData_send(sock, &packet_data);
	}
	else {
		fprintf(stderr, "Integrated Client: Cannot connect to server\n");
		return -1;
	}
	
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, client_thread, (void*)&sock, 0, NULL);

	// スレッドが終了するまで待機
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	PacketData_release(&packet_data);

	// 通信用ソケットを閉じる
	Socket_close(sock);

	// ソケット用リソースを解放（サーバ，クライアント共用）
	Socket_release();

	return 0;
}
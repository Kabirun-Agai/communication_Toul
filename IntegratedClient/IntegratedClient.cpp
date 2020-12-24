#include "Command.h"
#include "UserData.h"
#include "ChatData.h"
#include "MouseData.h"
#include "StampData.h"
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

bool lbutton_down = false;
int color_idx = 12; // black in Command.h

int stamp_id = 0;

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

char winname[MAX_WINNAME_LEN]; // = "IntegratedClient"; // ウィンドウ名

/***** 送信用マウスクライアントデータ ************/
MouseData send_mouse_data;

/***** スタンプ機能とホワイトボード機能の区別のために使用 *****/
typedef enum {
	MODE_STAMP = 0x00,
	MODE_MOUSE = 0x01,
} Mode;

// デフォルトはスタンプモード
Mode mode = MODE_STAMP;

/***** スタンプデータ for 送信用 */
StampData send_stamp_data;
// スタンプ画像格納用
cv::Mat stamp[NUM_OF_STAMP];
/*********************************/

/***** 画像共有クライアント用 *****/
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

void on_mouse(int event, int x, int y, int flag, void* param) {
	std::string event_description;
	int ret;

	SOCKET sock = *(SOCKET*)param;

	PacketData packet_data;
	PacketData_init(&packet_data);

	if (0 <= x && x < DRAW_AREA_W && 0 <= y && y < DRAW_AREA_H) {
		/*** ここから ***/
		/*** スタンプの幅，高さ，中心位置のx座標, 中心位置のy座標を計算 ***/
		int sw = stamp[send_stamp_data.stamp_id].cols;
		int sh = stamp[send_stamp_data.stamp_id].rows;
		int sx = x - sw / 2;
		int sy = y - sh / 2;

		// スタンプの矩形領域が画像の座標範囲に入っていれば
		if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

			// マウスイベントの種類による処理の分岐
			switch (event) {
				// マウスポインタが動いたとき
			case cv::EVENT_MOUSEMOVE:

				switch (mode)
				{
				case MODE_STAMP:
						event_description += "MOUSE_MOVE";
						break;

				case MODE_MOUSE:
					if (lbutton_down) {

						/*** ここから ***/
						MouseData_make(&send_mouse_data, x, y, true, true, color_idx);

						MouseData2PacketData(&send_mouse_data, &packet_data);
						ret = PacketData_send(sock, &packet_data);

						MouseData_print(&send_mouse_data);
						PacketData_print(&packet_data);
						break;

						/*** ここまで ***/
					}
				default:
					break;
				}
				break;

				// 左ボタンがクリックされたとき
			case cv::EVENT_LBUTTONDOWN:
				event_description += "LBUTTON_DOWN";
				switch (mode){

				case MODE_STAMP:
					std::cerr << sx << ", " << sy << ", " << sw << ", " << sh << std::endl;

					StampData_make(&send_stamp_data, x, y, send_stamp_data.stamp_id);
					StampData2PacketData(&send_stamp_data, &packet_data);

					ret = PacketData_send(sock, &packet_data);


					// 送信したデータをテキスト表示
					StampData_print(&send_stamp_data);
					break;

				case MODE_MOUSE:
					lbutton_down = true;

					MouseData_make(&send_mouse_data, x, y, true, false, color_idx);

					MouseData2PacketData(&send_mouse_data, &packet_data);
					ret = PacketData_send(sock, &packet_data);

					MouseData_print(&send_mouse_data);
					PacketData_print(&packet_data);

					break;

				default:
					break;
				}

				break;
				// 左ボタンが離されたとき
			case cv::EVENT_LBUTTONUP:
				event_description += "LBUTTON_UP";
				lbutton_down = false;
				switch (mode) {

				case MODE_STAMP:
					break;

				case MODE_MOUSE:
					MouseData_make(&send_mouse_data, x, y, false, false, color_idx);

					MouseData2PacketData(&send_mouse_data, &packet_data);
					ret = PacketData_send(sock, &packet_data);

					MouseData_print(&send_mouse_data);
					PacketData_print(&packet_data);

					break;

				default:
					break;
				}
				break;
			}
		}

		/*** ここまで ***/
	}
	//std::cerr << "mouse event: " << event_description << std::endl;

	PacketData_release(&packet_data);

	return;
}

// クライアントスレッド
unsigned __stdcall client_thread(void* p) {
	int key;

	// 通信ソケットを受け取る
	SOCKET sock = *(SOCKET*)p;

	// ソケットを非ブロッキングモードにする
	Socket_setNonBlockingMode(sock);

	char path[MAX_FILE_PATH];

	PacketData recv_packet_data;
	PacketData_init(&recv_packet_data);

	ImageData image_data;
	ImageData_init(&image_data);

	ImageData upload_image;
	ImageData_init(&upload_image);

	MouseData recv_mouse_data;
	MouseData_make(&recv_mouse_data, 0, 0, false, false, 0);

	/***** スタンプデータ for 受信用 */
	StampData recv_stamp_data;

	MouseData pre_mouse_data;
	MouseData_make(&pre_mouse_data, 0, 0, false, false, 0);

	ChatData chat_data;

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

	cvPutTextJP pt("MS ゴシック", FONT_SIZE);
	cv::Rect src_rect(0, 0, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);
	cv::Rect dst_rect(0, FONT_SIZE, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);

	// ダブルバッファをメッセージ表示用画像領域のクローンとして，別途確保
	// ダブル（裏面）用バッファ
	cv::Mat doubleBuffer;
	doubleBuffer = chat_area.clone();
	doubleBuffer = cv::Scalar(255, 127, 127);

	// マウスイベントが発生したときに呼び出す関数（コールバック関数）を登録
	cv::setMouseCallback(winname, on_mouse, &sock);

	while (connected) {

		int ret;

		// PacketDataを受信
		PacketData_clear(&recv_packet_data);
		ret = PacketData_recv(sock, &recv_packet_data);
		
		if (ret == 0) {
			// ソケットが閉じられたとき
		}
		else if (ret < 0) {
			// データ未受信のとき
		}
		else {
			cv::Mat src_roi, dst_roi;
			int sw, sh, sx, sy;
			unsigned char cmd = recv_packet_data.cmd;

			/*** ここから ***/
			switch (cmd) {
			case DAT_STAMPDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_STAMPDATA\n");

				// 受信したpacket_dataからStampDataを生成
				PacketData2StampData(&recv_packet_data, &recv_stamp_data);

				StampData_print(&recv_stamp_data);

				// 受信データからスタンプIDを取得
				/* スタンプの幅sw，高さsh，中心位置のsx座標, 中心位置のsy座標を計算 */
				sw = stamp[recv_stamp_data.stamp_id].cols;
				sh = stamp[recv_stamp_data.stamp_id].rows;
				sx = recv_stamp_data.x - sw / 2;
				sy = recv_stamp_data.y - sh / 2;

				// スタンプの矩形領域が画像の座標範囲に入っていれば
				if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

					// ROI(x, y, w, h)を作る
					cv::Rect roi_rect(sx, sy, sw, sh);
					// draw_area上にROIを設定
					cv::Mat src_roi = draw_area(roi_rect);
					// スタンプをキャンバスにコピー
					stamp[recv_stamp_data.stamp_id].copyTo(src_roi);

				}
				// draw_areaのアップデート
				update_draw_area(canvas, draw_area);
				break;

			case DAT_CHATDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_CHATDATA\n");

				PacketData2ChatData(&recv_packet_data, &chat_data);
				ChatData_print(&chat_data);

				// 表示メッセージの作成
				char message_text[MAX_USERNAME_LEN + MAX_MESSAGE_LEN];
				sprintf_s(message_text, MAX_USERNAME_LEN + MAX_MESSAGE_LEN, "%s: %s", chat_data.user_name, chat_data.message);

				//ChatData_maketext(&chat_data, message_text);

				/* 画像の一番上に一行追加 */
				src_roi = chat_area(src_rect);
				dst_roi = doubleBuffer(dst_rect);
				src_roi.copyTo(dst_roi);

				doubleBuffer.copyTo(chat_area);

				pt.setLocate(cv::Point(0, 0));
				pt.putText(chat_area, (TCHAR*)message_text, cv::Scalar(0, 0, 0));

				// chat_areaのアップデート
				update_chat_area(canvas, chat_area);

				break;

			case DAT_IMAGEDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_IMAGEDATA\n");
				// PacketDataから画像データ(ImageData)に変換
			
				PacketData2ImageData(&recv_packet_data, &image_data);

				fprintf(stderr, "Integrated Client: ImageData_recv: size = %d\n", ret);
				image_data.image.copyTo(draw_area);

				// draw_areaのアップデート
				update_draw_area(canvas, draw_area);
				break;

			case DAT_MOUSEDATA:
				fprintf(stderr, "Whiteboard Client: Recv: DAT_MOUSEDATA: ");
				// 受信データに基づいてdraw_areaに描画
				/*** ここから ***/
				PacketData2MouseData(&recv_packet_data, &recv_mouse_data);

				if (recv_mouse_data.lbutton_down && !recv_mouse_data.dragged) {
					cv::circle(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), 10, color[recv_mouse_data.color_idx],-1);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);
				}
				else if (recv_mouse_data.lbutton_down && recv_mouse_data.dragged) {
					cv::line(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), cv::Point(pre_mouse_data.x, pre_mouse_data.y), color[recv_mouse_data.color_idx],20);
					fprintf(stderr, "(%d. %d) - (%d. %d)\n", recv_mouse_data.x, recv_mouse_data.y, pre_mouse_data.x, pre_mouse_data.y);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);

				}

				/*** ここまで ***/

				// draw_areaのアップデート
				update_draw_area(canvas, draw_area);
				break;

			default:
				fprintf(stderr, "Integrated Client: client_thread: Received Command: 0x%02x\n", recv_packet_data.cmd);
				break;
			}

			/*** ここまで ***/
		}
		
		if (update_image_flag == true) {
			// ダイアログで指定した画像ファイルを読み出してimageにセットする
			
			cv::Mat image = cv::imread(path);
			if (!image.empty()) {
				PacketData image_packet_data;
				PacketData_init(&image_packet_data);
				// 読み出した画像を(IMAGE_W, IMAGE_H)にリサイズする
				cv::resize(image, upload_image.image, upload_image.image.size(), cv::INTER_CUBIC);
				
				ImageData_make(&image_data, upload_image.image);
				ImageData2PacketData(&image_data, &image_packet_data);
				PacketData_print(&image_packet_data);
				
				ret = PacketData_send(sock, &image_packet_data);
				
				PacketData_release(&image_packet_data);
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
			PacketData user_packet_data;
			PacketData_init(&user_packet_data);
			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &user_packet_data);
			
			ret = PacketData_send(sock, &user_packet_data);
			
			PacketData_release(&user_packet_data);
			connected = false;
		}
		else if (key == 'o') {

			// ファイルダイアログを開き，送信したい画像を選択する
			HANDLE h_open = (HANDLE)_beginthreadex(NULL, 0, open_file_dialog, path, 0, NULL);
			CloseHandle(h_open);
		}
		else if (key == 'm') {
			/*** ここから(StampClient機能とWhiteboardClient機能を同時に実現する場合に必要) ***/
			if (mode == MODE_MOUSE) {
				mode = MODE_STAMP;
			}else if(mode == MODE_STAMP) {
				mode = MODE_MOUSE;
			}

			/*** ここまで(StampClient機能とWhiteboardClient機能を同時に実現する場合に必要) ***/
		}
		else {
			/*** '0' ~ 'f'までのキーが押されたとき，keynumに0 ~ 15(十進数)を代入する．それ以外のキー入力の場合，keynumに-1を代入する ***/
			int keynum;
			if (0x30 <= key && key <= (0x30 + 9)) { // 0 - 9
				keynum = key - 0x30;
			}
			else if (0x61 <= key && key <= (0x61 + 5)) { // 'a'(10) - 'f'(15)
				keynum = key - 0x61 + 10;
			}
			else {
				keynum = -1;
			}
			
			

			/*** ここから(StampClient機能とWhiteboardClient機能を同時に実現する場合に必要) ***/
			/*** スタンプモードのとき，'0' ~ NUM_OF_STAMPのキー入力でstamp_idを設定 ***/
			/*** ここから ***/
			if (mode == MODE_STAMP) {
				if (0 <= keynum && keynum < NUM_OF_STAMP) {
					send_stamp_data.stamp_id = keynum;
					fprintf(stderr, "Integrated Client: stamp_data.stamp_id = %d\n", send_stamp_data.stamp_id);
				}

			}	
			/*** ここまで***/

			/*** ホワイトボードモードのとき，'0' ~ 'f'のキー入力でcolor_idxに色のインデックス(Command.hで定義)を設定 ***/
			/*** ここから ***/
			else if (mode == MODE_MOUSE) {
				if (0 <= keynum && keynum < NUM_OF_COLOR) {
					send_mouse_data.color_idx = keynum;
					fprintf(stderr, "Whiteboard Client: send_mouse_data.color_idx = %d\n", send_mouse_data.color_idx);
				}
			}


			/*** ここまで ***/
			/*** ここまで(StampClient機能とWhiteboardClient機能を同時に実現する場合に必要) ***/
		}
	}

	doubleBuffer.release();
	ImageData_release(&image_data);
	ImageData_release(&upload_image);
	
	PacketData_release(&recv_packet_data);

	// キャンバス用メモリを解放
	canvas.release();
	draw_area.release();
	chat_area.release();

	_endthreadex(0);

	return 0;
}

// メッセージ入力用スレッド
unsigned __stdcall keyboard_input_thread(void* p) {

	// 通信ソケットを受け取る
	SOCKET sock = *((SOCKET*)p);

	char message[MAX_MESSAGE_LEN];

	ChatData chat_data;

	PacketData packet_data;
	PacketData_init(&packet_data);

	// 永久ループ
	for (;;) {
		int ret;

		// 入力用プロンプトを表示
		fprintf(stdout, "Please Input: ");

		// 標準入力から１行入力
		memset(message, '\0', MAX_MESSAGE_LEN);
		fgets(message, MAX_MESSAGE_LEN, stdin);
		// fgetsでは，"文字列\n\0"となるので，以下のようにして改行コード('\n')を'\0'で上書きする
		for (unsigned int i = 0; i < strlen(message); i++)
		{
			if (message[i] == '\n')
			{
				message[i] = '\0';
				break;
			}
		}

		// メッセージが'quit'のとき，connected変数をfalseにした上で，break
		if (strcmp(message, "quit") == 0) {

			/*** ユーザデータをデータベースサーバから削除 ***/
			// UserDataを生成

			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);
			
			ret = PacketData_send(sock, &packet_data);
			
			connected = false;

			break;
		}

		// メッセージ送信
		// 入力からメッセージを作成
		ChatData_make(&chat_data, 12, user_data.user_name, message);
		
		ChatData2PacketData(&chat_data, &packet_data);

		
		ret = PacketData_send(sock, &packet_data);
		
		ChatData_print(&chat_data);
	}

	PacketData_release(&packet_data);

	_endthreadex(0);

	return 0;
}

int main(int argc, char* argv[]) {
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

	sprintf_s(winname, MAX_WINNAME_LEN, "IntegratedClient - %s", user_name);

	// ユーザデータ(登録用)を作成
	UserData_make(user_name, &user_data);
	// ユーザデータ情報を表示
	UserData_print(&user_data);
	// パケットデータに変換
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// パケットデータ情報を表示
	PacketData_print(&packet_data);
	// パケットデータ情報(バイナリ情報)を表示
	PacketData_print_binary(&packet_data);

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
		fprintf(stderr, "IntegratedClient: Cannot connect to server\n");
		return -1;
	}

	/* スタンプ画像の読み込み */
	char filename[80];
	for (int i = 0; i < NUM_OF_STAMP; i++) {
		sprintf_s(filename, 80, "..\\..\\images\\stamp%02d.png", i);
		stamp[i] = cv::imread(filename);
		if (stamp[i].empty()) {
			fprintf(stderr, "Cannot open %s\n", filename);
			system("PAUSE");
			return -1;
		}
	}

	MouseData_make(&send_mouse_data, 0, 0, false, false, 12);

	// 各スレッドを開始
	HANDLE hThread[2];
	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, keyboard_input_thread, &sock, 0, NULL);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// すべてのスレッドが終了するまで待機
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	for (int i = 0; i < 2; i++) {
		CloseHandle(hThread[i]);
	}

	PacketData_release(&packet_data);

	// 通信用ソケットを閉じる
	Socket_close(sock);

	// ソケット用リソースを解放（サーバ，クライアント共用）
	Socket_release();

	return 0;
}
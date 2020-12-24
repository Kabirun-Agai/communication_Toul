#include "Command.h"
#include "UserData.h"
#include "ChatData.h"

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

char* winname = "Chat Client"; // ウィンドウ名

// クライアントスレッド
unsigned __stdcall client_thread(void* p) {

	// 通信ソケットを受け取る
	SOCKET sock = *(SOCKET*)p;

	// ソケットを非ブロッキングモードにする
	Socket_setNonBlockingMode(sock);

	PacketData packet_data;
	PacketData_init(&packet_data);

	ChatData chat_data;

	/***** ドローエリア用画像の準備 *****/
	cv::Mat draw_area;
	// メモリ割り当て
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// 白で塗りつぶす
	draw_area = cv::Scalar(255, 255, 255);

	/***** チャットエリア用画像の準備 *****/
	cv::Mat chat_area;
	// メモリ割り当て
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// 薄青色で塗りつぶす
	chat_area = cv::Scalar(255, 127, 127);

	/***** 全体表示用画像（キャンバス）の準備 *****/
	cv::Mat canvas;
	// メモリ割り当て
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// 薄青色で塗りつぶす
	canvas = cv::Scalar(255, 255, 255);

	// ウィンドウを開く
	cv::namedWindow(winname);
	// リサイズする
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);
	// 画像の表示予約
	cv::imshow(winname, canvas);
	// 画像の描画&キー入力待ち(引数0の時ブロック，1以上のとき待機時間(ms))
	int key = cv::waitKey(1);

	cvPutTextJP pt("MS ゴシック", FONT_SIZE);
	cv::Rect src_rect(0, 0, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);
	cv::Rect dst_rect(0, FONT_SIZE, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);

	// ダブルバッファをメッセージ表示用画像領域のクローンとして，別途確保
	// ダブル（裏面）用バッファ
	cv::Mat doubleBuffer;
	doubleBuffer = chat_area.clone();
	doubleBuffer = cv::Scalar(255, 127, 127);

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
			cv::Mat src_roi, dst_roi;

			switch (packet_data.cmd) {
			case DAT_CHATDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_CHATDATA\n");

				PacketData2ChatData(&packet_data, &chat_data);
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
			default:

				break;
			}

			// 画像の表示予約
			cv::imshow(winname, canvas);
			// 画像の表示&キー入力
			key = cv::waitKey(1);

			if (key == 0x1b || key == 'q' || key == 'Q') {
				connected = false;
			}
		}
	}

	// キャンバス用メモリを解放
	canvas.release();
	draw_area.release();
	chat_area.release();

	// 受信データから色情報を取得
	doubleBuffer.release();

	_endthreadex(0);

	return 0;
}

// メッセージ入力用スレッド
unsigned __stdcall message_input_thread(void* p) {

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
		fprintf(stderr, "Chat Client: Cannot connect to server\n");
		return -1;
	}

	// 各スレッドを開始
	HANDLE handle[2];
	handle[0] = (HANDLE)_beginthreadex(NULL, 0, message_input_thread, &sock, 0, NULL);
	handle[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// すべてのスレッドが終了するまで待機
	WaitForMultipleObjects(2, handle, TRUE, INFINITE);

	for (int i = 0; i < 2; i++) {
		CloseHandle(handle[i]);
	}

	// 通信用ソケットを閉じる
	Socket_close(sock);

	// ソケット用リソースを解放（サーバ，クライアント共用）
	Socket_release();

	return 0;
}
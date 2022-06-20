/***************************************************************************************
**		第5章　ミニゲームを作る
**				レース＆避けゲー
**************************************************************************************/
#include "DxLib.h"
#define RANKING_DATA 5

/*************************************************************************
*	定数の宣言
**************************************************************************/
//画面領域の大きさ
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/*************************************************************************
*	変数の宣言
**************************************************************************/

int g_OldKey;	//前回の入力キー
int g_NowKey;	//今回の入力キー
int g_KeyFlg;	//入力キー情報

int g_GameState = 0;	//ゲームモード
int g_Score = 0; //スコア

int g_WaitTime = 0;

int g_TitleImage;	//画像用変数
int g_EndImage;
int g_Menu;
int g_RankingImage;  //ランキング画像用変数

int g_StageImage;

//ランキングデータ（構造体）
struct RankingData {
	int no;
	char name[11];
	long score;
};

//	ランキングデータの変数宣言
struct RankingData g_Ranking[RANKING_DATA];


/*************************************************************************
*	関数のプロトタイプ宣言
**************************************************************************/

void DrawGameTitle(void);		//タイトル描画処理
void DrawHelp(void);			//ゲームヘルプ描画処理
void GameInit(void);		//ゲーム初期処理
void GameMain(void);		//ゲームメイン処理
void DrawGameOver(void);		//ゲームオーバー画面処理
void DrawEnd(void);			//ゲームエンド描画処理

void DrawRanking(void);			//ランキング描画処理
void InputRanking(void);		//ランキング入力
void SortRanking(void);				//ランキング並べ替え
int SaveRanking(void);			//ランキングデータの保存
int ReadRanking(void);				//ランキングデータ読み込み

int LoadImages();		//画像読込み


/*************************************************************************
*	プログラムの開始
**************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpcmdLine, int nCmdShow) {
	SetMainWindowText("GettingOverApple");

	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)return -1;

	SetDrawScreen(DX_SCREEN_BACK);

	if (LoadImages() == -1)return -1;

	//ランキングデータ読み込み
	if (ReadRanking() == -1) return -1;

	//ゲームループ
	while (ProcessMessage() == 0 && g_GameState != 99 && !(g_KeyFlg & PAD_INPUT_START)) {
		//入力キー取得
		g_OldKey = g_NowKey;
		g_NowKey = GetJoypadInputState(DX_INPUT_KEY_PAD1);
		g_KeyFlg = g_NowKey & ~g_OldKey;

		ClearDrawScreen();		//画面の初期化

		GameMain();				//ゲームメイン処理

		ScreenFlip();				//裏画面の内容を表画面に反映
	}
	DxLib_End();					//DXライブラリ使用の終了処理
	return 0;				//ソフト終了
}

/*************************************************************************
*	ゲームタイトル表示（メニュー画面）
**************************************************************************/
void DrawGameTitle(void) {
	static int MenuNo = 0;

	//メニューカーソル移動処理
	if (g_KeyFlg & PAD_INPUT_DOWN) {
		if (++MenuNo > 3)MenuNo = 0;
	}
	if (g_KeyFlg & PAD_INPUT_UP) {
		if (--MenuNo < 0)MenuNo = 3;
	}
	//Zキーでメニュー選択
	if (g_KeyFlg & PAD_INPUT_A)g_GameState = MenuNo + 1;

	//タイトル画像表示
	DrawGraph(0, 0, g_TitleImage, FALSE);

	//メニュー
	DrawGraph(120, 200, g_Menu, TRUE);

	//メニューカーソル
	//DrawRotaGraph(90, 220 + MenuNo * 40, 0.7f, M_PI / 2, g_Cone, TRUE);

}

/*************************************************************************
*　ゲームヘルプ描画処理
**************************************************************************/
void DrawHelp(void) {
	//スペースキーでメニューに戻る
	if (g_KeyFlg & PAD_INPUT_M)g_GameState = 0;
}

/*************************************************************************
*	ゲーム初期処理
**************************************************************************/
void GameInit(void) {
	//スコアの初期化
	g_Score = 0;

	//ゲームメイン処理へ
	g_GameState = 5;
}

/*************************************************************************
*　ゲームメイン
**************************************************************************/
void GameMain(void) {

}

/*************************************************************************
*　ゲームエンド描画処理
**************************************************************************/
void DrawEnd(void) {
	//エンド画像表示
	DrawGraph(0, 0, g_EndImage, FALSE);

	SetFontSize(24);
	DrawString(360, 480 - 24, "Thank you for playing", 0xffffff, 0);

	//タイムの加算処理＆終了(3秒後)
	if (++g_WaitTime > 180)g_GameState = 99;

}


/*************************************************************************
*　ゲームオーバー画面処理
**************************************************************************/
void DrawGameOver(void) {
	//スペースキーでメニューに戻る
	if (g_KeyFlg & PAD_INPUT_M) {
		if (g_Ranking[RANKING_DATA - 1].score >= g_Score) {
			g_GameState = 0;
		}
		else {
			g_GameState = 7;
		}
	}
	DrawBox(150, 150, 490, 330, 0x009900, TRUE);
	DrawBox(150, 150, 490, 330, 0x000000, FALSE);

	SetFontSize(20);
	DrawString(310, 310, "スコア　", 0x000000);
	DrawFormatString(260, 310, 0xFFFFFF, "             =%6d", g_Score);

	DrawString(150, 450, "---- スペースキーを押してタイトルへ戻る ----", 0xffffff, 0);

}

/*************************************************************************
*　ゲームランキング描画表示
**************************************************************************/
void DrawRanking(void) {
	//スペースキーでメニューに戻る
	if (g_KeyFlg & PAD_INPUT_M)g_GameState = 0;

	//ランキング画像表示
	DrawGraph(0, 0, g_RankingImage, FALSE);

	//ランキング一覧を表示
	SetFontSize(18);
	for (int i = 0; i < RANKING_DATA; i++) {
		DrawFormatString(50, 170 + i * 25, 0xffffff, "%2d %-10s %10d",
			g_Ranking[i].no,
			g_Ranking[i].name,
			g_Ranking[i].score);
	}

	DrawString(100, 450, "----スペースキーを押してタイトルへ戻る----", 0xffffff, 0);
}

/*************************************************************************
*　ランキング入力処理
**************************************************************************/
void InputRanking(void) {
	//ランキング画像表示
	DrawGraph(0, 0, g_RankingImage, FALSE);

	// フォントサイズの設定
	SetFontSize(20);

	// 名前入力指示文字列の描画
	DrawString(150, 240, "ランキングに登録します", 0xFFFFFF);
	DrawString(150, 270, "名前を英字で入力してください", 0xFFFFFF);

	// 名前の入力
	DrawString(150, 310, "> ", 0xFFFFFF);
	DrawBox(160, 305, 300, 335, 0x000055, TRUE);
	if (KeyInputSingleCharString(170, 310, 10, g_Ranking[RANKING_DATA - 1].name, FALSE) == 1) {
		g_Ranking[RANKING_DATA - 1].score = g_Score;	// ランキングデータの5番目にスコアを登録
		SortRanking();		// ランキング並べ替え
		SaveRanking();		// ランキングデータの保存
		g_GameState = 2;		// ゲームモードの変更
	}
}

/***********************************************
 * ランキング並べ替え
 ***********************************************/
void SortRanking(void)
{
	int i, j;
	RankingData work;

	// 選択法ソート
	for (i = 0; i < RANKING_DATA - 1; i++) {
		for (j = i + 1; j < RANKING_DATA; j++) {
			if (g_Ranking[i].score <= g_Ranking[j].score) {
				work = g_Ranking[i];
				g_Ranking[i] = g_Ranking[j];
				g_Ranking[j] = work;
			}
		}
	}

	// 順位付け
	for (i = 0; i < RANKING_DATA; i++) {
		g_Ranking[i].no = 1;
	}
	// 得点が同じ場合は、同じ順位とする
	// 同順位があった場合の次の順位はデータ個数が加算された順位とする
	for (i = 0; i < RANKING_DATA - 1; i++) {
		for (j = i + 1; j < RANKING_DATA; j++) {
			if (g_Ranking[i].score > g_Ranking[j].score) {
				g_Ranking[j].no++;
			}
		}
	}
}

/***********************************************
 * ランキングデータの保存
 ***********************************************/
int  SaveRanking(void)
{
	FILE* fp;
#pragma warning(disable:4996)
	// ファイルオープン
	if ((fp = fopen("dat/rankingdata.txt", "w")) == NULL) {
		/* エラー処理 */
		printf("Ranking Data Error\n");
		return -1;
	}

	// ランキングデータ分配列データを書き込む
	for (int i = 0; i < RANKING_DATA; i++) {
		fprintf(fp, "%2d %10s %10d\n", g_Ranking[i].no, g_Ranking[i].name, g_Ranking[i].score);
	}

	//ファイルクローズ
	fclose(fp);

	return 0;
}

/*************************************
 * ランキングデータ読み込み
 *************************************/
int ReadRanking(void)
{
	FILE* fp;
#pragma warning(disable:4996)

	//ファイルオープン
	if ((fp = fopen("dat/rankingdata.txt", "r")) == NULL) {
		//エラー処理
		printf("Ranking Data Error\n");
		return -1;
	}

	//ランキングデータ分配列データを読み込む
	for (int i = 0; i < RANKING_DATA; i++) {
		int Q = fscanf(fp, "%2d %10s %10d", &g_Ranking[i].no, g_Ranking[i].name, &g_Ranking[i].score);
	}

	//ファイルクローズ
	fclose(fp);

	return 0;
}

/*************************************************************************
*　画像処理
**************************************************************************/
int LoadImages() {
	//タイトル
	if ((g_TitleImage = LoadGraph("images/Title.bmp")) == -1) return -1;				//未差し替え
	//メニュー
	if ((g_Menu = LoadGraph("images/menu.bmp")) == -1)return-1;							//未差し替え

	//ランキングデータ
	if ((g_RankingImage = LoadGraph("images/Ranking.bmp")) == -1)return -1;				//未差し替え

	//エンディング
	if ((g_EndImage = LoadGraph("images/End.bmp")) == -1)return -1;						//未差し替え

	//プレイヤー
	
	
	//ステージ背景
	if ((g_StageImage = LoadGraph("images/back.bmp")) == -1)return -1;					//未差し替え

	return 0;
}


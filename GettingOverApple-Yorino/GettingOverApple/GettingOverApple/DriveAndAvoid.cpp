/***************************************************************************************
**		第5章　ミニゲームを作る
**				レース＆避けゲー
**************************************************************************************/
#include "DxLib.h"
#define _USE_MATH_DEFINES
#include <math.h>
#define RANKING_DATA 5

/*************************************************************************
*	定数の宣言
**************************************************************************/
//画面領域の大きさ
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//自機の初期値
const int PLAYER_POS_X = SCREEN_WIDTH / 2;
const int PLAYER_POS_Y = SCREEN_HEIGHT - 100;
const int PLAYER_WIDTH = 58;
const int PLAYER_HEIGHT = 120;
const int PLAYER_SPEED = 5;
const int PLAYER_HP = 1000;
const int PLAYER_FUEL = 20000;
const int PLAYER_BARRIER = 3;
const int PLAYER_BARRIERUP = 10;

//敵機の最大数
const int ENEMY_MAX = 8;

//アイテムの最大数
const int ITEM_MAX = 3;

const int BikeMove_r = 50;
/*************************************************************************
*	変数の宣言
**************************************************************************/

int g_OldKey;	//前回の入力キー
int g_NowKey;	//今回の入力キー
int g_KeyFlg;	//入力キー情報

int g_GameState = 0;	//ゲームモード
int g_Score = 0; //スコア

int g_WaitTime = 0;

int g_Mileage; //走行距離
int g_EnemyCount1, g_EnemyCount2, g_EnemyCount3, g_EnemyCount4; //敵カウント

int g_TitleImage;	//画像用変数
int g_RankingImage;  //画像用変数
int g_EndImage;
int g_Menu, g_Cone;		//メニュー画像変数
int g_StageImage;
int g_Car, g_Barrier; //プレイヤーキャラ画像変数

int g_Item[2];//アイテム画像
int g_Teki[3]; //敵キャラ画像変数
int g_bikeimage;

int GameOverMileage = 0;

//自機の構造体
struct PLAYER {
	int flg;		//使用フラグ
	int x, y;		//座標x,y
	int w, h;		//幅w　高さy
	double angle;	//機体の向き
	int count;		//タイミング用
	int speed;		//移動速度
	int hp;			//体力
	int fuel;       //燃料
	int bari;       //バリア回数
	int baricnt;	//バリア継続時間
	int bariup;     //バリア回数（走行距離アップ）
};

//自機
struct PLAYER g_player;

//敵機の構造体
struct ENEMY {
	int flg;			//使用フラグ
	int type;			//タイプ
	int img;			//画像
	int x, y, w, h;		//座標x,y:幅w：高さh	
	int speed;			//移動速度
	int point;			//スコア加算
};

//敵機
struct ENEMY g_enemy[ENEMY_MAX];
struct ENEMY g_enemy00 = { TRUE,0,0,0,-50,58,120,0,1 };
struct ENEMY g_enemyCn = { TRUE, 4, 0, 0, -50, 18, 18, 0, 1 };
struct ENEMY g_enemyBike = { TRUE,3,0,0,-50,32,72,3,1 };

struct ENEMY g_item[ITEM_MAX];
struct ENEMY g_item00 = { TRUE,0,0,0,-50,50,50,0,1 };


//ランキングデータ（構造体）
struct RankingData {
	int no;
	char name[11];
	long score;
};

//	ランキングデータの変数宣言
struct RankingData g_Ranking[RANKING_DATA];

int BikeAnglecnt[ENEMY_MAX];
int BikePoint[ENEMY_MAX];
int Bike_v[ENEMY_MAX];
double makecos[ENEMY_MAX];

/*************************************************************************
*	関数のプロトタイプ宣言
**************************************************************************/
void GameInit(void);		//ゲーム初期処理
void GameMain(void);		//ゲームメイン処理
int LoadImages();		//画像読込み

void DrawGameTitle(void);		//タイトル描画処理
void DrawGameOver(void);		//ゲームオーバー画面処理
void DrawEnd(void);			//ゲームエンド描画処理
void DrawHelp(void);			//ゲームヘルプ描画処理
void BackScrool();					//背景画像スクロール処理

void DrawRanking(void);			//ランキング描画処理
void InputRanking(void);		//ランキング入力
void SortRanking(void);				//ランキング並べ替え
int SaveRanking(void);			//ランキングデータの保存
int ReadRanking(void);				//ランキングデータ読み込み

void PlayerControl();				//自機処理
void EnemyControl();				//敵機処理
int CreateEnemy();					//敵機生成処理
void ItemControl();					//アイテム処理
int CreateItem();					//アイテム生成処理

int HitBoxPlayer(PLAYER* p, ENEMY* e); //当たり判定

/*************************************************************************
*	プログラムの開始
**************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpcmdLine, int nCmdShow) {
	SetMainWindowText("Drive&Avoid");

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

		switch (g_GameState) {
		case 0:
			DrawGameTitle();		//ゲームタイトル描画処理
			break;
		case 1:
			GameInit();				//ゲーム初期処理
			break;
		case 2:
			DrawRanking();			//ランキング描画処理
			break;
		case 3:
			DrawHelp();				//ゲームヘルプ描画処理
			break;
		case 4:
			DrawEnd();				//ゲームエンド描画処理
			break;
		case 5:
			GameMain();				//ゲームメイン処理
			break;
		case 6:
			DrawGameOver();			//ゲームオーバー描画処理
			break;
		case 7:
			InputRanking();			//ランキング入力処理
			break;
		}


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
	DrawRotaGraph(90, 220 + MenuNo * 40, 0.7f, M_PI / 2, g_Cone, TRUE);

}


/*************************************************************************
*	ゲーム初期処理
**************************************************************************/
void GameInit(void) {
	//スコアの初期化
	g_Score = 0;

	//走行距離の初期化
	g_Mileage = 0;

	//敵１を避けた数の初期化
	g_EnemyCount1 = 0;
	g_EnemyCount2 = 0;
	g_EnemyCount3 = 0;
	g_EnemyCount4 = 0;

	//プレイヤーの初期設定
	g_player.flg = TRUE;
	g_player.x = PLAYER_POS_X;
	g_player.y = PLAYER_POS_Y;
	g_player.w = PLAYER_WIDTH;
	g_player.h = PLAYER_HEIGHT;
	g_player.angle = 0.0;
	g_player.count = 0;
	g_player.speed = PLAYER_SPEED;
	g_player.hp = PLAYER_HP;
	g_player.fuel = PLAYER_FUEL;
	g_player.bari = PLAYER_BARRIER;
	g_player.bariup = PLAYER_BARRIERUP;

	//エネミーの初期設定
	for (int i = 0; i < ENEMY_MAX; i++) {
		g_enemy[i].flg = FALSE;
	}

	//アイテムの初期設定
	for (int i = 0; i < ITEM_MAX; i++) {
		g_item[i].flg = FALSE;
	}

	//ゲームメイン処理へ
	g_GameState = 5;
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
*　ゲームヘルプ描画処理
**************************************************************************/
void DrawHelp(void) {
	//スペースキーでメニューに戻る
	if (g_KeyFlg & PAD_INPUT_M)g_GameState = 0;

	//タイトル画像処理
	DrawGraph(0, 0, g_TitleImage, FALSE);
	SetFontSize(16);
	DrawString(20, 120, "ヘルプ画面", 0xffffff, 0);

	DrawString(20, 160, "これは障害物を避けながら", 0xffffff, 0);
	DrawString(20, 180, "走り続けるゲームです", 0xffffff, 0);
	DrawString(20, 200, "燃料が尽きるか障害物に", 0xffffff, 0);
	DrawString(20, 220, "数回当たるとゲームオーバーです", 0xffffff, 0);
	DrawString(20, 250, "アイテム一覧", 0xffffff, 0);
	DrawGraph(20, 260, g_Item[0], TRUE);
	DrawString(20, 315, "取ると燃料が回復するよ", 0xffffff, 0);
	DrawGraph(20, 335, g_Item[1], TRUE);
	DrawString(20, 385, "ダメージを受けているとき時に取ると耐久回復", 0xffffff, 0);
	DrawString(20, 405, "耐久が減っていなかったら燃料が少し回復するよ", 0xffffff, 0);
	DrawString(150, 450, "----スペースキーを押してタイトルへ戻る----", 0xffffff, 0);
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
*　ゲームメイン
**************************************************************************/
void GameMain(void) {
	BackScrool();

	EnemyControl();

	PlayerControl();

	ItemControl();

	//スペースキーでメニューに戻る
	/*if (g_KeyFlg & PAD_INPUT_M)g_GameState = 6;

	SetFontSize(16);
	DrawString(20, 20, "ゲームメイン", 0xffffff, 0);
	DrawString(150, 450, "スペースキーを押してゲームオーバーへ", 0xffffff, 0);*/

}

/*************************************************************************
*　ゲームオーバー画面処理
**************************************************************************/
void DrawGameOver(void) {
	g_Score = (g_Mileage / 10 * 10) + g_EnemyCount3 * 50 +
		g_EnemyCount2 * 100 + g_EnemyCount1 * 200 + g_EnemyCount4 * 250;

	//スペースキーでメニューに戻る
	if (g_KeyFlg & PAD_INPUT_M) {
		if (g_Ranking[RANKING_DATA - 1].score >= g_Score) {
			g_GameState = 0;
		}
		else {
			g_GameState = 7;
		}
	}
	BackScrool();

	DrawBox(150, 150, 490, 330, 0x009900, TRUE);
	DrawBox(150, 150, 490, 330, 0x000000, FALSE);

	SetFontSize(20);
	DrawString(220, 170, "ゲームオーバー", 0xcc0000);
	SetFontSize(16);
	DrawString(180, 200, "走行距離　　", 0x000000);

	DrawRotaGraph(230, 230, 0.3f, M_PI / 2, g_Teki[2], TRUE, FALSE);
	DrawRotaGraph(230, 250, 0.3f, M_PI / 2, g_Teki[1], TRUE, FALSE);
	DrawRotaGraph(230, 270, 0.3f, M_PI / 2, g_Teki[0], TRUE, FALSE);
	DrawRotaGraph(230, 290, 0.3f, M_PI / 2, g_bikeimage, TRUE, FALSE);

	DrawFormatString(260, 200, 0xFFFFFF, "%6d x  10 = %6d",
		g_Mileage / 10, g_Mileage / 10 * 10);
	DrawFormatString(260, 222, 0xFFFFFF,
		"%6d x   50 = %6d", g_EnemyCount3, g_EnemyCount3 * 50);
	DrawFormatString(260, 243, 0xFFFFFF,
		"%6d x  100 = %6d", g_EnemyCount2, g_EnemyCount2 * 100);
	DrawFormatString(260, 264, 0xFFFFFF,
		"%6d x  200 = %6d", g_EnemyCount1, g_EnemyCount1 * 200);
	DrawFormatString(260, 285, 0xFFFFFF,
		"%6d x  250 = %6d", g_EnemyCount4, g_EnemyCount4 * 250);

	DrawString(310, 310, "スコア　", 0x000000);
	DrawFormatString(260, 310, 0xFFFFFF, "             =%6d", g_Score);

	DrawString(150, 450, "---- スペースキーを押してタイトルへ戻る ----", 0xffffff, 0);

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
 * プレイヤーの移動
 * 引　数：なし
 * 戻り値：なし
 ***********************************************/
void PlayerControl() {
	//燃料の消費
	g_player.fuel -= g_player.speed;
	//ゲームオーバー処理へ
	if (g_player.fuel <= 0)		g_GameState = 6;

	//Zキーで加速
	if (g_KeyFlg & PAD_INPUT_A && g_player.speed < 10)	g_player.speed += 1;

	//Xキーで減速
	if (g_KeyFlg & PAD_INPUT_B && g_player.speed > 1)	g_player.speed -= 1;

	//上下左右移動
	if (g_player.flg == TRUE) {
		if (g_NowKey & PAD_INPUT_UP)	g_player.y -= g_player.speed;
		if (g_NowKey & PAD_INPUT_DOWN)	g_player.y += g_player.speed;
		if (g_NowKey & PAD_INPUT_LEFT)	g_player.x -= g_player.speed;
		if (g_NowKey & PAD_INPUT_RIGHT)	g_player.x += g_player.speed;
	}

	//画面をはみ出さないようにする
	if (g_player.x < 32)	g_player.x = 32;
	if (g_player.x > SCREEN_WIDTH - 180)		g_player.x = SCREEN_WIDTH - 180;
	if (g_player.y < 60)	g_player.y = 60;
	if (g_player.y > SCREEN_HEIGHT - 60)		g_player.y = SCREEN_HEIGHT - 60;

	//プレイヤーの表示
	if (g_player.flg == TRUE) {
		if (g_NowKey & PAD_INPUT_LEFT) {
			DrawRotaGraph(g_player.x, g_player.y, 1.0f, -M_PI / 18, g_Car, TRUE, FALSE);
		}
		else if (g_NowKey & PAD_INPUT_RIGHT) {
			DrawRotaGraph(g_player.x, g_player.y, 1.0f, M_PI / 18, g_Car, TRUE, FALSE);
		}
		else {
			DrawRotaGraph(g_player.x, g_player.y, 1.0f, 0, g_Car, TRUE, FALSE);
		}
		if (g_KeyFlg & PAD_INPUT_C && g_player.baricnt <= 0) {
			g_player.bari--;
			g_player.baricnt = 1000;
		}
		if (g_player.baricnt > 0) {
			g_player.baricnt -= g_player.speed;
			DrawRotaGraph(g_player.x, g_player.y, 1.0f, 0, g_Barrier, TRUE, FALSE);
		}
		else {
			g_player.baricnt = 0;
		}
	}
	else {
		DrawRotaGraph(g_player.x, g_player.y, 1.0f, M_PI / 8 * (++g_player.count / 5)
			, g_Car, TRUE, FALSE);
		if (g_player.count >= 80)	g_player.flg = TRUE;
	}

	//敵を避けた数を表示
	SetFontSize(16);
	DrawFormatString(510, 20, 0x000000, "ハイスコア");
	DrawFormatString(560, 40, 0xFFFFFF, "%08d", g_Ranking[0].score);
	DrawFormatString(510, 80, 0x000000, "避けた数");
	DrawRotaGraph(523, 120, 0.3f, 0, g_Teki[0], TRUE, FALSE);
	DrawRotaGraph(573, 120, 0.3f, 0, g_Teki[1], TRUE, FALSE);
	DrawRotaGraph(623, 120, 0.3f, 0, g_Teki[2], TRUE, FALSE);
	DrawRotaGraph(523, 170, 0.3f, 0, g_bikeimage, TRUE, FALSE);

	DrawFormatString(510, 140, 0xFFFFFF, "%03d", g_EnemyCount1);
	DrawFormatString(560, 140, 0xFFFFFF, "%03d", g_EnemyCount2);
	DrawFormatString(610, 140, 0xFFFFFF, "%03d", g_EnemyCount3);
	DrawFormatString(510, 180, 0xFFFFFF, "%03d", g_EnemyCount4);
	DrawFormatString(510, 200, 0x000000, "走行距離");
	DrawFormatString(555, 220, 0xFFFFFF, "%08d", g_Mileage / 10);
	DrawFormatString(510, 240, 0x000000, "スピード");
	DrawFormatString(555, 260, 0xFFFFFF, "%08d", g_player.speed);

	//バリアの表示
	for (int i = 0; i < g_player.bari; i++) {
		DrawRotaGraph(520 + i * 25, 340, 0.2f, 0, g_Barrier, TRUE, FALSE);

	}

	//燃料ゲージの表示
	int F_X = 510; int F_Y = 390; int F_W = 100; int F_H = 20;
	DrawString(F_X, F_Y, "FUEL METER", 0x000000, 0);
	//内側のゲージ
	DrawBox(
		F_X,
		F_Y + 20,
		F_X + (int)(g_player.fuel * F_W / PLAYER_FUEL),
		F_Y + 20 + F_H,
		0x0066cc, TRUE);
	//外側のゲージ枠
	DrawBox(
		F_X,
		F_Y + 20,
		F_X + F_W,
		F_Y + 20 + F_H,
		0x000000, FALSE);

	//体力ゲージの表示
	int X = 510; int Y = 430; int W = 100; int H = 20;
	DrawString(X, Y, "PLAYER HP", 0x000000, 0);
	//内側のゲージ
	DrawBox(
		X,
		Y + 20,
		X + (int)(g_player.hp * W / PLAYER_HP),
		Y + 20 + H,
		0xff0000, TRUE);
	//外側のゲージ枠
	DrawBox(
		X,
		Y + 20,
		X + W,
		Y + 20 + H,
		0x000000, FALSE);

}

/***********************************************
 * エネミーの移動
 * 引　数：なし
 * 戻り値：なし
 ***********************************************/
void EnemyControl() {
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (g_enemy[i].flg == TRUE) {

			//敵の表示
			DrawRotaGraph(g_enemy[i].x, g_enemy[i].y,
				1.0f, 0, g_enemy[i].img, TRUE, FALSE);

			if (g_player.flg == FALSE)continue;
			//まっすぐ下に移動
			g_enemy[i].y += g_enemy[i].speed + g_player.speed - PLAYER_SPEED + 1;

			if (g_enemy[i].type == 3) {
				//蛇行
				++BikeAnglecnt[i];

				//90～270°でcosθを作成
				makecos[i] = cos((M_PI / 2) / 90 * BikeAnglecnt[i]);

				//cosθ*rで基準点xからの距離を計算する
				g_enemy[i].x =
					BikePoint[i] - (int)(makecos[i] * BikeMove_r);
			}
			//画面をはみ出したら消去
			if (g_enemy[i].y > SCREEN_HEIGHT + g_enemy[i].h)
				g_enemy[i].flg = FALSE;

			//敵機を追い越したらカウントする
			if (g_enemy[i].y > g_player.y && g_enemy[i].point == 1) {
				g_enemy[i].point = 0;
				if (g_enemy[i].type == 0) g_EnemyCount1++;
				if (g_enemy[i].type == 1) g_EnemyCount2++;
				if (g_enemy[i].type == 2) g_EnemyCount3++;
				if (g_enemy[i].type == 3) g_EnemyCount4++;
			}

			//当たり判定
			if (HitBoxPlayer(&g_player, &g_enemy[i]) == TRUE && g_player.baricnt <= 0) {
				g_player.flg = FALSE;
				g_player.speed = PLAYER_SPEED;
				g_player.count = 0;
				g_player.hp -= 100;
				g_enemy[i].flg = FALSE;
				if (g_player.hp <= 0) g_GameState = 6;
			}
		}
	}

	//走行距離ごとに敵出現パターンを制御する
	if (g_Mileage / 10 % 50 == 0) {
		CreateEnemy();
	}

}

/***********************************************
 * 敵機の生成
 * 引　数：なし
 * 戻り値：TRUE:成功　FALSE:失敗
 ***********************************************/
int CreateEnemy() {
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (g_enemy[i].flg == FALSE) {
			g_enemy[i] = g_enemy00;
			g_enemy[i].type = GetRand(3);
			if (g_enemy[i].type < 3) {
				g_enemy[i].img = g_Teki[g_enemy[i].type];
				g_enemy[i].x = GetRand(4) * 105 + 45;
				g_enemy[i].speed = g_enemy[i].type * 2;
			}
			else {
				g_enemy[i] = g_enemyBike;
				BikeAnglecnt[i] = GetRand(361);
				g_enemy[i].img = g_bikeimage;
				g_enemy[i].x = GetRand(3) * 100 + 100;//車線上にx座標を取る
				BikePoint[i] = g_enemy[i].x;//車線上に基準をコピーする
			}
			//成功
			return TRUE;
		}
	}

	//失敗
	return FALSE;
}

/***********************************************
 * アイテムの制御
 * 引　数：なし
 * 戻り値：なし
 ***********************************************/
void ItemControl() {
	for (int i = 0; i < ITEM_MAX; i++) {
		if (g_item[i].flg == TRUE) {

			//アイテムの表示
			DrawRotaGraph(g_item[i].x, g_item[i].y, 1.0f, 0, g_item[i].img, TRUE);
			if (g_player.flg == FALSE)continue;

			//真っすぐ下に移動
			g_item[i].y += g_item[i].speed + g_player.speed - PLAYER_SPEED;

			//画面をはみ出したら消去
			if (g_item[i].y > SCREEN_HEIGHT)g_item[i].flg = FALSE;

			//当たり判定
			if (HitBoxPlayer(&g_player, &g_item[i]) == TRUE) {
				g_item[i].flg = FALSE;
				if (g_item[i].type == 0)g_player.fuel += g_item[i].point;
				if (g_item[i].type == 1) {
					g_player.hp += g_item[i].point;
					if (g_player.hp > PLAYER_HP)g_player.hp = PLAYER_HP;
				}
			}

		}
	}

	//走行距離ごとにアイテム出現パターンを制御する
	if (g_Mileage / 10 % 500 == 0) {
		CreateItem();
	}
}


/***********************************************
 * アイテムの生成
 * 引　数：なし
 * 戻り値：TRUE:成功　FALSE:失敗
 ***********************************************/
int CreateItem() {
	for (int i = 0; i < ITEM_MAX; i++) {
		if (g_item[i].flg == FALSE) {
			g_item[i] = g_item00;
			g_item[i].type = GetRand(1);
			g_item[i].img = g_Item[g_item[i].type];
			g_item[i].x = GetRand(4) * 105 + 40;
			g_item[i].speed = 1 + g_item[i].type * 3;
			if (g_item[i].type == 0)g_item[i].point = 500;
			if (g_item[i].type == 1)g_item[i].point = 50;

			//成功
			return TRUE;
		}
	}

	//失敗
	return FALSE;
}


/*************************************************************************
*	背景画像スクロール処理
*　 引　数　：　なし
* 　戻り値　：　なし
**************************************************************************/
void BackScrool() {
	//g_Mileage += 5;
	GameOverMileage += 5;
	if (g_GameState != 6)
		g_Mileage += g_player.speed;

	//ステージ画像表示

	//描画可能エリアを設定
	SetDrawArea(0, 0, 500, 480);
	if (g_GameState != 6) {
		DrawGraph(0, g_Mileage % 480 - 480, g_StageImage, FALSE);
		DrawGraph(0, g_Mileage % 480, g_StageImage, FALSE);
	}
	else {
		//ゲームオーバー画面でのスクロール
		DrawGraph(0, GameOverMileage % 480 - 480, g_StageImage, FALSE);
		DrawGraph(0, GameOverMileage % 480, g_StageImage, FALSE);
	}
	//エリアを戻す
	SetDrawArea(0, 0, 640, 480);

	//スコア等表示領域
	DrawBox(500, 0, 640, 480, 0x009900, TRUE);
}

/*************************************************************************
*　画像処理
**************************************************************************/
int LoadImages() {
	//タイトル
	if ((g_TitleImage = LoadGraph("images/Title.bmp")) == -1) return -1;
	//メニュー
	if ((g_Menu = LoadGraph("images/menu.bmp")) == -1)return-1;
	if ((g_Cone = LoadGraph("images/cone.bmp")) == -1)return-1;

	//ランキングデータ
	if ((g_RankingImage = LoadGraph("images/Ranking.bmp")) == -1)return -1;

	//アイテム
	if ((g_Item[0] = LoadGraph("images/gasoline.bmp")) == -1)return -1;
	if ((g_Item[1] = LoadGraph("images/supana.bmp")) == -1)return -1;

	//エンディング
	if ((g_EndImage = LoadGraph("images/End.bmp")) == -1)return -1;

	//敵
	if (LoadDivGraph("images/Car.bmp", 3, 3, 1, 63, 120, g_Teki) == -1)return -1;
	if ((g_bikeimage = LoadGraph("images/gentuki.bmp")) == -1)return -1;

	//プレイヤー
	if ((g_Car = LoadGraph("images/car1pol.bmp")) == -1)return -1;
	if ((g_Barrier = LoadGraph("images/barrier.png")) == -1)return -1;

	//ステージ背景
	if ((g_StageImage = LoadGraph("images/back.bmp")) == -1)return -1;

	return 0;
}


/***********************************************
 * 自機と敵機の当たり判定(四角)
 * 引　数：PLAYER　ポインタ
 * 戻り値：TRUE:当たり，FALSE:なし
 ***********************************************/
int HitBoxPlayer(PLAYER* p, ENEMY* e) {
	//x,yは中心座標とする
	int sx1 = p->x - (p->w / 2);
	int sy1 = p->y - (p->h / 2);
	int sx2 = sx1 + p->w;
	int sy2 = sy1 + p->h;

	int dx1 = e->x - (e->w / 2);
	int dy1 = e->y - (e->h / 2);
	int dx2 = dx1 + e->w;
	int dy2 = dy1 + e->h;

	//短形が重なっていれば当たり
	if (sx1 < dx2 && dx1 < sx2 && sy1 < dy2 && dy1 < sy2) {
		return TRUE;
	}

	return FALSE;
}
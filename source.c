/*
=ビットボードオセロプログラム=


*/

#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<limits.h>
#include<stdlib.h>
#include<stdbool.h>


#define WHITE 'O'
#define BLACK 'X'
#define EMPTY ' '

#define MAX_INPUT 256 //入力の最大文字数(実際にはNULL文字'\0'があるため、1少なくなる)
#define LAST 48 //完全読み
#define PRE_LAST 48 //終局読み
#define FOLLOW_LAST 35 //終盤読み

typedef unsigned long long int BITBOARD;

int evalution[8][8] = {
	{ 30,-12,  0, -1, -1,  0,-12, 30},
	{-12,-15, -3, -3, -3, -3,-15,-12},
	{  0, -3,  0, -1, -1,  0, -3,  0},
	{ -1, -3, -1, -1, -1, -1, -3, -1},
	{ -1, -3, -1, -1, -1, -1, -3, -1},
	{  0, -3,  0, -1, -1,  0, -3,  0},
	{-12,-15, -3, -3, -3, -3,-15,-12},
	{ 30,-12,  0, -1, -1,  0,-12, 30}
};

//現在の手番
char now_color;
//棋譜を書き込むか
bool kihu = false;

typedef struct DATA{
	BITBOARD move; //動き
	int eva; //評価
}DATA;


void make_kihu(void) {
	FILE* fp;
	fp = fopen("kihu.txt", "w");

	if (fp == NULL) {
		printf("ファイルの初期化に失敗しました。");
		exit(-1);
	}

	fclose(fp);
}

void write_kihu(int row, int col) {
	if (kihu) {
		FILE* fp;
		fp = fopen("kihu.txt", "a");

		if (fp == NULL) {
			printf("ファイルの初期化に失敗しました。");
			exit(-1);
		}

		fprintf(fp, "%c%c", 'a' + col, '1' + row);

		fclose(fp);
	}
	else {

	}
}

void bitboard_to_rowcol(int* row, int* col, BITBOARD board) {
	BITBOARD temp = 0x8000000000000000;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (board & (temp >> (j + i * 8))) {
				*row = i;
				*col = j;
				goto end;
			}
		}
	}

end:
	;
}

void bitboard_to_char(BITBOARD *W,BITBOARD *B,char board[8][8]) {
	BITBOARD temp = 0x8000000000000000;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (*W & (temp >> (j + i * 8))) {
				board[i][j] = WHITE;
			}
			else if (*B & (temp >> (j + i * 8))) {
				board[i][j] = BLACK;
			}
			else {
				board[i][j] = EMPTY;
			}
		}
	}
}

//盤面の初期化
void init_board(BITBOARD* W, BITBOARD* B) {
	*W = 0x0000001008000000;
	*B = 0x0000000810000000;
}

//盤面の表示
void print_board(BITBOARD W, BITBOARD B) {
	BITBOARD temp = 0x8000000000000000;
	char first_r = '1', first_c = 'a';

	printf(" |");
	for (int i = 0; i < 8; i++) {
		printf("%c|", first_c + i);
	}
	printf("\n");

	printf("--");
	for (int i = 0; i < 8; i++) {
		printf("--");
	}
	printf("\n");


	for (int i = 0; i < 8; i++) {
		printf("%c|",first_r+i);
		for (int j = 0; j < 8; j++) {
			if (W & (temp >> (j + i * 8))) {
				printf("%c|", WHITE);
			}
			else if (B & (temp >> (j + i * 8))) {
				printf("%c|", BLACK);
			}
			else {
				printf("%c|", EMPTY);
			}
		}
		printf("\n");
	}
}

//デバッグ情報(合法手)を含めた盤面の表示
void print_debug_board(BITBOARD W, BITBOARD B,BITBOARD leg) {
	BITBOARD temp = 0x8000000000000000;
	char first_r = '1', first_c = '1';

	printf(" |");
	for (int i = 0; i < 8; i++) {
		printf("%c", first_c + i);
	}
	printf("\n");

	printf("--");
	for (int i = 0; i < 8; i++) {
		printf("-");
	}
	printf("\n");


	for (int i = 0; i < 8; i++) {
		printf("%c|", first_r + i);
		for (int j = 0; j < 8; j++) {
			if (W & (temp >> (j + i * 8))) {
				printf("%c", WHITE);
			}
			else if (B & (temp >> (j + i * 8))) {
				printf("%c", BLACK);
			}
			else if (leg & (temp >> (j + i * 8))) {
				printf("%c", '@');
			}
			else {
				printf("%c", EMPTY);
			}
		}
		printf("\n");
	}
}

//ビットカウントを行う
int bitcount(BITBOARD board) {
	board = board - ((board >> 1) & 0x5555555555555555);

	board = (board & 0x3333333333333333) + ((board >> 2) & 0x3333333333333333);

	board = (board + (board >> 4)) & 0x0f0f0f0f0f0f0f0f;
	board = board + (board >> 8);
	board = board + (board >> 16);
	board = board + (board >> 32);
	return board & 0x0000007f;
}

//石の数を数える
int count_stones(BITBOARD W, BITBOARD B) {
	int re = 0;
	re += bitcount(W);
	re += bitcount(B);

	return re;
}

//n番目の合法手を返す
BITBOARD n_leg(BITBOARD leg, int n) {
	BITBOARD temp = 0x8000000000000000;
	int num = 0;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (leg & (temp >> (j + i * 8))) {
				if (num == n) {
					temp = (temp >> (j + i * 8));
					return temp;
				}
				num++;
			}
		}
	}

	return 0;
}


//mycolorの合法手を返す
BITBOARD legal(BITBOARD W, BITBOARD B, char mycolor) {
	BITBOARD my, en, leg;
	leg = 0;
	if (mycolor == BLACK) {
		my = B;
		en = W;
	}
	else {
		my = W;
		en = B;
	}

	//空きマス
	BITBOARD empty = ~(my | en);

	//端っこの相手の石はひっくり返せない
	BITBOARD en_temp_lr = 0x7e7e7e7e7e7e7e7e & en; //左右探索用(lrはleft,rightの略)
	BITBOARD en_temp_rc = 0x00ffffffffffff00 & en; //上下探索用(rcはrow,colの略)
	BITBOARD en_temp_ob = 0x007e7e7e7e7e7e00 & en; //斜め探索用(obはobliqueの略)


	//左にある相手の石を探索(tは使い捨て)
	BITBOARD t = en_temp_lr & (my << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	leg |= empty & (t << 1); //ひっくり返せる石は最大でも6個

	//右にある相手の石を探索(tは使い捨て)
	t = en_temp_lr & (my >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	leg |= empty & (t >> 1); //ひっくり返せる石は最大でも6個

	//下にある相手の石を探索(tは使い捨て)
	t = en_temp_rc & (my >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	leg |= empty & (t >> 8); //ひっくり返せる石は最大でも6個

	//上にある相手の石を探索(tは使い捨て)
	t = en_temp_rc & (my << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	leg |= empty & (t << 8); //ひっくり返せる石は最大でも6個

	//左斜め下にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	leg |= empty & (t >> 7); //ひっくり返せる石は最大でも6個

	//右斜め下にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	leg |= empty & (t >> 9); //ひっくり返せる石は最大でも6個

	//右斜め上にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	leg |= empty & (t << 7); //ひっくり返せる石は最大でも6個

	//左斜め上にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	leg |= empty & (t << 9); //ひっくり返せる石は最大でも6個

	return leg;
}

//座標をビットボードに変換
BITBOARD coordinate_to_bitboard(int row, int col) {
	BITBOARD re = 0x8000000000000000;
	return re >> (col + row * 8);
}

//mycolorがmoveに石をおいた時、反転される石のビットボードを返す
BITBOARD return_flip_board(BITBOARD W, BITBOARD B, BITBOARD move, char mycolor) {
	BITBOARD my, en, flip, leg;
	flip = 0;
	leg = 0;
	if (mycolor == BLACK) {
		my = B;
		en = W;
	}
	else {
		my = W;
		en = B;
	}

	//空きマス
	BITBOARD empty = ~(my | en);

	//端っこの相手の石はひっくり返せない
	BITBOARD en_temp_lr = 0x7e7e7e7e7e7e7e7e & en; //左右探索用(lrはleft,rightの略)
	BITBOARD en_temp_rc = 0x00ffffffffffff00 & en; //上下探索用(rcはrow,colの略)
	BITBOARD en_temp_ob = 0x007e7e7e7e7e7e00 & en; //斜め探索用(obはobliqueの略)

	//左にある相手の石を探索(tは使い捨て)
	BITBOARD t = en_temp_lr & (my << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	t |= en_temp_lr & (t << 1);
	leg = empty & (t << 1); //ひっくり返せる石は最大でも6個

	//左にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) >> 1) & en;
	t |= t >> 1 & en;
	t |= t >> 1 & en;
	t |= t >> 1 & en;
	t |= t >> 1 & en;
	t |= t >> 1 & en;
	flip |= t; //ひっくり返せる石は最大でも6個



	//右にある相手の石を探索(tは使い捨て)
	t = en_temp_lr & (my >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	t |= en_temp_lr & (t >> 1);
	leg = empty & (t >> 1); //ひっくり返せる石は最大でも6個

	//右にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) << 1) & en;
	t |= t << 1 & en;
	t |= t << 1 & en;
	t |= t << 1 & en;
	t |= t << 1 & en;
	t |= t << 1 & en;
	flip |= t; //ひっくり返せる石は最大でも6個



	//上にある相手の石を探索(tは使い捨て)
	t = en_temp_rc & (my << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	t |= en_temp_rc & (t << 8);
	leg = empty & (t << 8); //ひっくり返せる石は最大でも6個

	//上にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) >> 8) & en;
	t |= t >> 8 & en;
	t |= t >> 8 & en;
	t |= t >> 8 & en;
	t |= t >> 8 & en;
	t |= t >> 8 & en;
	flip |= t; //ひっくり返せる石は最大でも6個



	//下にある相手の石を探索(tは使い捨て)
	t = en_temp_rc & (my >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	t |= en_temp_rc & (t >> 8);
	leg = empty & (t >> 8); //ひっくり返せる石は最大でも6個

	//下にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) << 8) & en;
	t |= t << 8 & en;
	t |= t << 8 & en;
	t |= t << 8 & en;
	t |= t << 8 & en;
	t |= t << 8 & en;
	flip |= t; //ひっくり返せる石は最大でも6個


	//右斜め下にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	t |= en_temp_ob & (t >> 9);
	leg = empty & (t >> 9); //ひっくり返せる石は最大でも6個


	//下にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) << 9) & en;
	t |= t << 9 & en;
	t |= t << 9 & en;
	t |= t << 9 & en;
	t |= t << 9 & en;
	t |= t << 9 & en;
	flip |= t; //ひっくり返せる石は最大でも6個


	//左斜め下にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	t |= en_temp_ob & (t >> 7);
	leg = empty & (t >> 7); //ひっくり返せる石は最大でも6個

	//下にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) << 7) & en;
	t |= t << 7 & en;
	t |= t << 7 & en;
	t |= t << 7 & en;
	t |= t << 7 & en;
	t |= t << 7 & en;
	flip |= t; //ひっくり返せる石は最大でも6個


	//左斜め上にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	t |= en_temp_ob & (t << 9);
	leg = empty & (t << 9); //ひっくり返せる石は最大でも6個

	//下にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) >> 9) & en;
	t |= t >> 9 & en;
	t |= t >> 9 & en;
	t |= t >> 9 & en;
	t |= t >> 9 & en;
	t |= t >> 9 & en;
	flip |= t; //ひっくり返せる石は最大でも6個


	//右斜め上にある相手の石を探索(tは使い捨て)
	t = en_temp_ob & (my << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	t |= en_temp_ob & (t << 7);
	leg = empty & (t << 7); //ひっくり返せる石は最大でも6個

	//下にある相手の石をひっくり返す(tは使い捨て)
	t = ((move & leg) >> 7) & en;
	t |= t >> 7 & en;
	t |= t >> 7 & en;
	t |= t >> 7 & en;
	t |= t >> 7 & en;
	t |= t >> 7 & en;
	flip |= t; //ひっくり返せる石は最大でも6個

	return flip;
}

//mycolorがmoveに石をおいたときの反転処理を行う。
void flip(BITBOARD* W, BITBOARD* B, BITBOARD leg, BITBOARD move, char mycolor) {
	//反転される石を求める
	BITBOARD flipped = return_flip_board(*W, *B, move, mycolor);
	
	if (mycolor == BLACK) {
		*W ^= flipped;
		*B ^= flipped | move;
	}
	else {
		*B ^= flipped;
		*W ^= flipped | move;
	}
}

//人間の手番。キーボード入力を促し、合法手ならひっくり返しfalseを返す。パスならtrueを返す。
bool human_phase(BITBOARD* W, BITBOARD* B, char mycolor,int hoge) {
	BITBOARD leg = legal(*W, *B, mycolor);

	if (leg == 0) {
		//合法手がなければ(=パスならば)
		printf(">>合法手がないため、パスします。\n");
		return true;
	}

	char str[MAX_INPUT];
	int row = 0, col = 0;

	while (true){
		printf(">>%cの手番です。どこに置きますか？(例d3)",mycolor);
		if (scanf("%s", str) != 1) {
			//入力エラーなら
			printf("|!|有効な文字を入力してください。\n");
		}
		else {

			col = str[0] - 'a';
			row = str[1] - '1';

			if (row < 0 || col < 0 || row >= 8 || col >= 8) {
				//範囲外の入力
				printf("|!|有効な範囲の入力をしてください。\n");
			}
			else {
				if ((leg & coordinate_to_bitboard(row, col)) == 0) {
					//合法手ではないなら
					printf("|!|そこには置けません。\n");
				}
				else {
					break;
				}
			}
		}
	}

	//裏返し処理
	BITBOARD move = coordinate_to_bitboard(row, col);
	flip(W, B, leg, move, mycolor);
	write_kihu(row, col);
	
	return false;
}

//石の位置による評価
void position_eva(BITBOARD* W, BITBOARD* B, int* w_eva, int* b_eva) {
	BITBOARD temp = 0x8000000000000000;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (*W & (temp >> (j + i * 8))) {
				*w_eva += evalution[i][j];
			}
			else if (*B & (temp >> (j + i * 8))) {
				*b_eva += evalution[i][j];
			}
		}
	}
}

//着手可能位置による評価
void canput_eva(BITBOARD* W, BITBOARD* B, int* w_eva, int* b_eva) {
	BITBOARD W_leg = legal(*W, *B, WHITE);
	BITBOARD B_leg = legal(*W, *B, BLACK);

	*w_eva += bitcount(W_leg);
	*b_eva += bitcount(B_leg);
}



void corner_eva(BITBOARD* W, BITBOARD* B, int* w_eva, int* b_eva) {
	char board[8][8];
	bitboard_to_char(W, B, board);

	//角
	char corner[12][3];

	corner[0][0] = board[0][0];
	corner[0][1] = board[0][1];
	corner[0][2] = board[0][2];

	corner[1][0] = board[0][0];
	corner[1][1] = board[1][1];
	corner[1][2] = board[2][2];

	corner[2][0] = board[0][0];
	corner[2][1] = board[1][0];
	corner[2][2] = board[2][0];


	corner[3][0] = board[0][7];
	corner[3][1] = board[0][6];
	corner[3][2] = board[0][5];

	corner[4][0] = board[0][7];
	corner[4][1] = board[1][6];
	corner[4][2] = board[2][5];

	corner[5][0] = board[0][7];
	corner[5][1] = board[1][7];
	corner[5][2] = board[2][7];


	corner[6][0] = board[7][0];
	corner[6][1] = board[7][1];
	corner[6][2] = board[7][2];

	corner[7][0] = board[7][0];
	corner[7][1] = board[6][0];
	corner[7][2] = board[5][0];

	corner[8][0] = board[7][0];
	corner[8][1] = board[6][1];
	corner[8][2] = board[5][2];


	corner[9][0] = board[7][7];
	corner[9][1] = board[7][6];
	corner[9][2] = board[7][5];

	corner[10][0] = board[7][7];
	corner[10][1] = board[6][7];
	corner[10][2] = board[5][7];

	corner[11][0] = board[7][7];
	corner[11][1] = board[6][6];
	corner[11][2] = board[5][5];

	for (int i = 0; i < 12; i++) {
		if (corner[i][0] != BLACK && corner[i][1] != BLACK && corner[i][2] != BLACK) { goto b_end; }
		if (corner[i][0] != BLACK && corner[i][1] != BLACK && corner[i][2] == BLACK) { *b_eva +=  3; goto b_end; }
		if (corner[i][0] != BLACK && corner[i][1] == BLACK && corner[i][2] != BLACK) { *b_eva -= 10; goto b_end; }
		if (corner[i][0] != BLACK && corner[i][1] == BLACK && corner[i][2] == BLACK) { *b_eva -= 15; goto b_end; }
		if (corner[i][0] == BLACK && corner[i][1] != BLACK && corner[i][2] != BLACK) { *b_eva += 15; goto b_end; }
		if (corner[i][0] == BLACK && corner[i][1] != BLACK && corner[i][2] == BLACK) { *b_eva += 16; goto b_end; }
		if (corner[i][0] == BLACK && corner[i][1] == BLACK && corner[i][2] != BLACK) { *b_eva += 20; goto b_end; }
		if (corner[i][0] == BLACK && corner[i][1] == BLACK && corner[i][2] == BLACK) { *b_eva += 30; goto b_end; }

	b_end:

		while(1) {
			if (corner[i][0] != WHITE && corner[i][1] != WHITE && corner[i][2] != WHITE) { break; }
			if (corner[i][0] != WHITE && corner[i][1] != WHITE && corner[i][2] == WHITE) { *w_eva += 3; break; }
			if (corner[i][0] != WHITE && corner[i][1] == WHITE && corner[i][2] != WHITE) { *w_eva -= 10; break; }
			if (corner[i][0] != WHITE && corner[i][1] == WHITE && corner[i][2] == WHITE) { *w_eva -= 15; break; }
			if (corner[i][0] == WHITE && corner[i][1] != WHITE && corner[i][2] != WHITE) { *w_eva += 15; break; }
			if (corner[i][0] == WHITE && corner[i][1] != WHITE && corner[i][2] == WHITE) { *w_eva += 16; break; }
			if (corner[i][0] == WHITE && corner[i][1] == WHITE && corner[i][2] != WHITE) { *w_eva += 20; break; }
			if (corner[i][0] == WHITE && corner[i][1] == WHITE && corner[i][2] == WHITE) { *w_eva += 30; break; }
		}
	}
}


//評価関数
int calc_evaluion(BITBOARD* W, BITBOARD* B, char mycolor) {
	int w_eva = 0, b_eva = 0;
	if (count_stones(*W, *B) >= LAST) {
		w_eva += bitcount(*W);
		b_eva += bitcount(*B);
	}
	else if (count_stones(*W, *B) >= FOLLOW_LAST) {
		int w_p = 0, b_p = 0, w_c = 0, b_c = 0, w_k = 0, b_k = 0;
		

		position_eva(W, B, &w_p, &b_p);
		canput_eva(W, B, &w_c, &b_c);
		corner_eva(W, B, &w_k, &b_k);
		

		//理論値の合計はおよそ0。
		w_p *= 4;	b_p *= 4;
		w_c *= 15; b_c *= 15;
		w_k *= 2; b_k *= 2;

		b_eva = (int)(w_p + w_c + w_k);
		w_eva = (int)(b_p + b_c + b_k);
	}

	else {
		int w_p = 0, b_p = 0, w_c = 0, b_c = 0;

		position_eva(W, B, &w_p, &b_p);
		canput_eva(W, B, &w_c, &b_c);

			//理論値の合計はおよそ0。
			w_p *= 17;	b_p *= 17;
			w_c *= 10; b_c *= 10;	
		
		w_eva = (int)(w_p  + w_c);
		b_eva = (int)(b_p  + b_c);
	}

	if (mycolor == BLACK) {
		return b_eva - w_eva;
	}
	else {
		return w_eva - b_eva;
	}
}


//アルファベータ法を用いた評価関数
/*
・depth:読みの深さ
・player:trueならmax、falseならmin
・alpha:アルファ
・beta:ベータ
*/
int alphabeta(BITBOARD W, BITBOARD B, int depth, char put_color, bool player, int alpha, int beta) {
	if (depth == 0) {
		return calc_evaluion(&W, &B, now_color);
	}

	char reverse_color;
	if (put_color == WHITE) {
		reverse_color = BLACK;
	}
	else {
		reverse_color = WHITE;
	}

	BITBOARD pleg = legal(W, B, put_color);
	BITBOARD rleg = legal(W, B, reverse_color);
	if (pleg == 0 && rleg == 0) {
		//終局なら
		if (now_color == WHITE) {
			return bitcount(W);
		}
		else {
			return bitcount(B);
		}
	}

	if (pleg == 0) {
		//この局面における手番がパスなら相手に交代
		if (player) {
			return alphabeta(W, B, depth, reverse_color, false, alpha, beta);
		}
		else {
			return alphabeta(W, B, depth, reverse_color, true, alpha, beta);
		}
	}

	//置ける場所の総数
	int count = bitcount(pleg);

	DATA* data = (DATA*)malloc(sizeof(DATA) * count);

	if (data == NULL) {
		printf("メモリ不足により、エラーが発生しました。プログラムを強制終了します。");
		exit(-1);
	}

	BITBOARD W_tmp, B_tmp;
	int val;

	if (player) {
		//自分の手の時
		for (int i = 0; i < count; i++) {
			data[i].move = n_leg(pleg, i);

			W_tmp = W, B_tmp = B;
			flip(&W_tmp, &B_tmp, pleg, data[i].move, put_color);


			val = alphabeta(W_tmp, B_tmp, depth - 1, reverse_color, false, alpha, beta);
			alpha = max(alpha, val);
			if (beta <= alpha) {
				free(data);
				return alpha;
			}
		}
		free(data);
		return alpha;

	}
	else{
		//相手の時
		for (int i = 0; i < count; i++) {
			data[i].move = n_leg(pleg, i);

			W_tmp = W, B_tmp = B;
			flip(&W_tmp, &B_tmp, pleg, data[i].move, put_color);


			val = alphabeta(W_tmp, B_tmp, depth - 1, reverse_color, true, alpha, beta);
			beta = min(beta, val);
			if (beta <= alpha) {
				free(data);
				return beta;
			}
		}
		free(data);
		return beta;
	}
}


//コンピュータ手番。評価が最も良い手を打ちfalseを返す。パスならtrueを返す。
bool computer_phase(BITBOARD* W, BITBOARD* B, char mycolor, int depth) {
	BITBOARD leg = legal(*W, *B, mycolor);

	if (leg == 0) {
		//合法手がなければ(=パスならば)
		printf(">>合法手がないため、パスします。\n");
		return true;
	}

	printf(">>%cの番です。\n",mycolor);

	//合法手の数
	int count = bitcount(leg);
	DATA* data = (DATA*)malloc(sizeof(DATA) * count);
	
	if (data == NULL) {
		printf("メモリ不足により、エラーが発生しました。プログラムを強制終了します。");
		exit(-1);
	}
	else {
		BITBOARD W_tmp, B_tmp;
		char enemycolor;
		if (mycolor == WHITE) {
			enemycolor = BLACK;
		}
		else {
			enemycolor = WHITE;
		}

		if (count_stones(*W, *B) >= LAST) {
			depth = INT_MAX;
			for (int i = 0; i < count; i++) {
				data[i].move = n_leg(leg, i);

				W_tmp = *W, B_tmp = *B;
				flip(&W_tmp, &B_tmp, leg, data[i].move, mycolor);

				//print_board(W_tmp, B_tmp);

				data[i].eva = alphabeta(W_tmp, B_tmp, depth, enemycolor, false, INT_MIN, INT_MAX);
				//printf("eva:%d\n", data[i].eva);
				//printf("\n");

			}
		}
		else if (count_stones(*W, *B) >= PRE_LAST) {
			//move ordering
			depth = (int)(PRE_LAST / 2);
			for (int i = 0; i < count; i++) {
				data[i].move = n_leg(leg, i);

				W_tmp = *W, B_tmp = *B;
				flip(&W_tmp, &B_tmp, leg, data[i].move, mycolor);

				//print_board(W_tmp, B_tmp);

				data[i].eva = alphabeta(W_tmp, B_tmp, depth, enemycolor, false, INT_MIN, INT_MAX);
				//printf("eva:%d\n", data[i].eva);
				//printf("\n");
			}

			for (int i = 0; i < count - 1; i++) {
				for (int j = 0; j < count - 1 - i; j++) {
					if (data[j].eva < data[j + 1].eva) {
						DATA temp = data[j];
						data[j] = data[j + 1];
						data[j + 1] = temp;
					}
				}
			}

			depth = PRE_LAST;
			for (int i = 0; i < count; i++) {
				data[i].move = n_leg(leg, i);

				W_tmp = *W, B_tmp = *B;
				flip(&W_tmp, &B_tmp, leg, data[i].move, mycolor);

				//print_board(W_tmp, B_tmp);

				data[i].eva = alphabeta(W_tmp, B_tmp, depth, enemycolor, false, INT_MIN, INT_MAX);
				//printf("eva:%d\n", data[i].eva);
				//printf("\n");
			}
		}
		else {
			//move ordering
			int depth_temp = depth;
			depth = (int)(depth / 2);
			for (int i = 0; i < count; i++) {
				data[i].move = n_leg(leg, i);

				W_tmp = *W, B_tmp = *B;
				flip(&W_tmp, &B_tmp, leg, data[i].move, mycolor);

				//print_board(W_tmp, B_tmp);

				data[i].eva = alphabeta(W_tmp, B_tmp, depth, enemycolor, false, INT_MIN, INT_MAX);
				//printf("eva:%d\n", data[i].eva);
				//printf("\n");
			}

			for (int i = 0; i < count - 1; i++) {
				for (int j = 0; j < count - 1 - i; j++) {
					if (data[j].eva < data[j + 1].eva) {
						DATA temp = data[j];
						data[j] = data[j + 1];
						data[j + 1] = temp;
					}
				}
			}

			depth = depth_temp;

			for (int i = 0; i < count; i++) {
				data[i].move = n_leg(leg, i);

				W_tmp = *W, B_tmp = *B;
				flip(&W_tmp, &B_tmp, leg, data[i].move, mycolor);

				//print_board(W_tmp, B_tmp);

				data[i].eva = alphabeta(W_tmp, B_tmp, depth, enemycolor, false, INT_MIN, INT_MAX);
				//printf("eva:%d\n", data[i].eva);
				//printf("\n");

			}
		}

		



		for (int i = 0; i < count - 1; i++) {
			for (int j = 0; j < count - 1 - i; j++) {
				if (data[j].eva < data[j + 1].eva) {
					DATA temp = data[j];
					data[j] = data[j + 1];
					data[j + 1] = temp;
				}
			}
		}

		int row, col;

		for (int i = 0; i < count; i++) {
			bitboard_to_rowcol(&row, &col, data[i].move);
			printf("move:%c%c eva:%d\n", 'a'+col, '1'+row, data[i].eva);
		}

		bitboard_to_rowcol(&row, &col, data[0].move);

		//裏返し処理
		flip(W, B, leg, data[0].move, mycolor);
		
		write_kihu(row, col);

		free(data);

		return false;
	}
}


//勝敗を決定
void judge(BITBOARD W, BITBOARD B) {
	printf("====ゲーム終了====\n");

	int w_stones = 0, b_stones = 0;
	BITBOARD temp = 0x8000000000000000;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (W & (temp >> (j + i * 8))) {
				w_stones++;
			}
			else if (B & (temp >> (j + i * 8))) {
				b_stones++;
			}
		}
	}

	if (w_stones > b_stones) {
		printf("%c:%c = %d:%dで%cの勝ち！",WHITE,BLACK,w_stones,b_stones,WHITE);
	}
	else if (w_stones < b_stones) {
		printf("%c:%c = %d:%dで%cの勝ち！", WHITE, BLACK, w_stones, b_stones, BLACK);
	}
	else {
		printf("%c:%c = %d:%dで引き分け！", WHITE, BLACK, w_stones, b_stones);
	}
}

//勝敗を決定し、黒の石がどれだけ多いか返す。
int judge_for_B(BITBOARD W, BITBOARD B) {
	printf("====ゲーム終了====\n");

	int w_stones = 0, b_stones = 0;
	BITBOARD temp = 0x8000000000000000;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (W & (temp >> (j + i * 8))) {
				w_stones++;
			}
			else if (B & (temp >> (j + i * 8))) {
				b_stones++;
			}
		}
	}

	if (w_stones > b_stones) {
		printf("%c:%c = %d:%dで%cの勝ち！\n", WHITE, BLACK, w_stones, b_stones, WHITE);
	}
	else if (w_stones < b_stones) {
		printf("%c:%c = %d:%dで%cの勝ち！\n", WHITE, BLACK, w_stones, b_stones, BLACK);
	}
	else {
		printf("%c:%c = %d:%dで引き分け！\n", WHITE, BLACK, w_stones, b_stones);
	}

	return b_stones - w_stones;
}

void last_debug(BITBOARD W,BITBOARD B) {
	static bool first_L = true;
	if (first_L) {
		if (count_stones(W, B) >= LAST) {
			printf("!!!!終局読み開始!!!!\n");
			first_L = false;
		}
	}

	static bool first_P = true;
	if (first_P) {
		if (count_stones(W, B) >= LAST) {
			printf("!!!!必勝読み開始!!!!\n");
			first_P = false;
		}
	}
}

int main(void) {
	BITBOARD W = 0;
	BITBOARD B = 0;
	bool b_pass = false, w_pass = false;

	init_board(&W, &B);

	BITBOARD leg = legal(W, B, BLACK);

	int depth = 8;
	char ch = 'n';

	printf("棋譜を作成しますか？(y/n)");
	if (scanf("%c", &ch) == 1) {

	}

	if (ch == 'y') {
		make_kihu();
		kihu = true;
	}

	while (true) {
		printf("何手読みしますか？(デフォルトで8)");
		if (scanf("%d", &depth) == 1) {

		}

		if (depth > 10) {
			printf("|!|%d手読みでは莫大な時間がかかるおそれがありますが、よろしいですか？(y/n)", depth);
			if (scanf("\n%c", &ch) == 1) {

			}

			if (ch == 'y') {
				break;
			}
		}
		else {
			break;
		}
	}

	while (true){
		print_board(W, B);
		now_color = BLACK;
		last_debug(W, B);
		b_pass = computer_phase(&W, &B, BLACK, depth);
		printf("\n");

		if (b_pass && w_pass) {
			//どちらも置ける場所がない
			break;
		}

		print_board(W, B);
		now_color = WHITE;
		last_debug(W, B);
		w_pass = human_phase(&W, &B, WHITE, depth);
		printf("\n");

		if (b_pass && w_pass) {
			//どちらも置ける場所がない
			break;
		}
	}

	//ゲーム終了
	judge(W,B);

	return 0;
}


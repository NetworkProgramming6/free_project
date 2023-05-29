#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include "card.h"


extern int turn; //누구 차례인지 확인
extern int tt; //차례temp
extern char *name[4]; //이름
extern struct card clientCards[4][56]; //카드
extern int num; //접속인원 확인
extern int clientCardNumber[4]; //카드 개수.
extern int tableCardNum[4]; //테이블 카드 개수
extern pthread_t thread[4]; //4명 쓰레드
extern int status_num; //누구 상태가 변했는지
extern char playerStatus; //현재 상태 r,y,g,p:빨강,노랑,초록,보라색 카드 뒤집힘 /  o:벨 맞음 / x:벨 잘못누름 / e:게임종료
extern int statusCheck[4]; //1 : 클라이언트로 보내야할거 있음, 0 : 다 보냄

int bell(); //벨 누를 상황인지 아닌지 체크
void bell_O(int); //벨 누를 상황일 경우 테이블 카드 옮겨주기
void bell_X(int); //벨 누를 상황 아닐 경우 테이블 카드 옮겨주기
void pullCard(int,int); //카드 앞으로 당겨줌
void updateCardCounts(); // 각 색상 카드의 개수를 업데이트
int storeCards(struct card**); // 카드를 임시 저장하고 총 카드 개수를 계산
void distributeCards(int, struct card*, int); // 플레이어에게 카드를 분배하고 테이블 카드를 초기화
void checkGameEnd(); //게임끝났을 때 체크

void updateCardCounts(struct card C[], const char* colorCheck, int i) {
    if (strcmp(colorCheck, "red") == 0)
        C[0].num += clientCards[i][tableCardNum[i] - 1].num;
    else if (strcmp(colorCheck, "yellow") == 0)
        C[1].num += clientCards[i][tableCardNum[i] - 1].num;
    else if (strcmp(colorCheck, "green") == 0)
        C[2].num += clientCards[i][tableCardNum[i] - 1].num;
    else if (strcmp(colorCheck, "purple") == 0)
        C[3].num += clientCards[i][tableCardNum[i] - 1].num;
}

int bell()
{
    struct card C[4] = {
        {"red", 0},
        {"yellow", 0},
        {"green", 0},
        {"purple", 0}
    };

    for (int i = 0; i < 4; i++) {
        if (clientCards[i][tableCardNum[i] - 1].color != NULL) {
            updateCardCounts(C, clientCards[i][tableCardNum[i] - 1].color, i);
        }
    }

    for (int i = 0; i < 4; i++) {
        if (C[i].num == 5)
            return 1;
    }

    return 0; // 5개인 과일이 없는 경우
}

int storeCards(struct card** temp) {
    int sum = 0;
    for (int i = 0; i < 4; i++)
        sum += tableCardNum[i]; // 옮길 카드 개수 세는 부분

    *temp = malloc(sum * sizeof(struct card)); // 메모리 할당
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < tableCardNum[i]; j++) {
            (*temp)[k] = clientCards[i][j]; // 옮길 카드 temp에 임시 저장
            k++;
        }
    }
    return sum;
}

// 플레이어에게 카드를 분배하고 테이블 카드를 초기화
void distributeCards(int threadNum, struct card* temp, int sum) {
    for (int i = 0; i < sum; i++) {
        clientCards[threadNum][clientCardNumber[threadNum] + tableCardNum[threadNum] + i] = temp[i]; // 테이블에 있던 카드 벨 누른 플레이어에게 주기
    }
    for (int i = 0; i < 4; i++) {
        if (i == threadNum)
            clientCardNumber[threadNum] += sum; // 벨 누른 사람의 경우 카드 개수 증가
        pullCard(i, tableCardNum[i]); // 테이블에 있던 카드 수 만큼 카드 구조체 끌어옴
        tableCardNum[i] = 0; // 테이블의 카드 없어짐
    }
}

void bell_O(int threadNum) {
    struct card* temp;
    int sum = storeCards(&temp);
    distributeCards(threadNum, temp, sum);
    free(temp); // 메모리 해제
}

void bell_X(int threadNum)
{
	int i,j=0;
	struct card temp[3];
	for(i=(clientCardNumber[threadNum]+tableCardNum[threadNum]-1);i>=clientCardNumber[threadNum]+tableCardNum[threadNum]-3;i--)
	{
		temp[j]=clientCards[threadNum][i]; //옮길 카드 3장 temp에 임시로 저장
		clientCards[threadNum][i].num=0; //옮긴 카드 없애주기
		clientCards[threadNum][i].color=NULL;
		j++;
	}
	j=0;
	for(i=0;i<4;i++)
	{
		if(threadNum!=i) //벨 잘못 누르지 않은 플레이어의 경우
		{
			clientCards[i][clientCardNumber[i]+tableCardNum[i]]=temp[j]; //카드 하나 획득
			j++;
			clientCardNumber[i]++; //카드 1개 획득
		}
		else //벨 잘못 누른 플레이어의 경우
		{
			clientCardNumber[i]-=3; //카드 3개 손실 발생
		}
	}
}

void pullCard(int threadNum, int howmany)
{
	for(int i=0;i<clientCardNumber[threadNum];i++)
	{
		clientCards[threadNum][i]=clientCards[threadNum][i+howmany];
	}

	for(int i=clientCardNumber[threadNum]+tableCardNum[threadNum]-1;i>=clientCardNumber[threadNum];i--)
	{
		clientCards[threadNum][i].color=NULL;
		clientCards[threadNum][i].num=0;
	}
}

void checkGameEnd()
{
	for(int i=0;i<4;i++)
		if(clientCardNumber[i]<=0) //플레이어 카드 0이하인지 체크
		{
			playerStatus='e';
			for(int j=0;j<4;j++)
				statusCheck[j]=1;
		}
}
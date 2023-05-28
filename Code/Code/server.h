
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


extern int turn; //누구 차례인지
extern int tt; //차례temp
extern char *name[4]; //플레이어 이름
extern struct card clntCards[4][56]; //플레이어 카드
extern int num; //접속인원
extern int clientCardNumber[4]; //플레이어 카드 개수
extern int tableCardNum[4]; //테이블 카드 개수
extern pthread_t thread[4]; //플레이어 4명 쓰레드
extern int statusThreadNumber; //몇번 플레이어 상태가 변했는지
extern char playerStatus; //현재 상태 r,y,g,p:빨강,노랑,초록,보라색 카드 뒤집힘 /  o:벨 맞음 / x:벨 잘못누름 / e:게임종료
extern int statusCheck[4]; //1 : 클라이언트로 보내야할거 있음, 0 : 다 보냄

int bell(); //벨 누를 상황인지 체크
void bell_O(int); //벨 누를 상황일때 테이블 카드 옮겨주기
void bell_X(int); //벨 누를 상황 아닐때 테이블 카드 옮겨주기
void pullCard(int,int); //카드 앞으로 당겨줌
void checkGameEnd(); //게임끝 체크

int bell() //벨을 누를 상황인지 체크
{
    struct card C[4];
    C[0].color="red";
    C[1].color="yellow";
    C[2].color="green";
    C[3].color="purple";
    for(int i=0;i<4;i++)
        C[i].num=0;

    char* colorCheck;
    for(int i=0;i<4;i++)
    {
        if(clntCards[i][tableCardNum[i]-1].color!=NULL)
        {
            colorCheck=clntCards[i][tableCardNum[i]-1].color;
            if(strcmp(colorCheck,"red")==0)
                C[0].num+=clntCards[i][tableCardNum[i]-1].num;
                else if(strcmp(colorCheck,"yellow")==0)
                C[1].num+=clntCards[i][tableCardNum[i]-1].num;
            else if(strcmp(colorCheck,"green")==0)
                C[2].num+=clntCards[i][tableCardNum[i]-1].num;
            else if(strcmp(colorCheck,"purple")==0)
                C[3].num+=clntCards[i][tableCardNum[i]-1].num;
        }
    }

    for(int i=0;i<4;i++)
    {
        if(C[i].num==5) //5개인 과일이 있다
            return 1;
        }
    return 0; //5개인 과일이 없다
}

void bell_O(int threadNum)
{
    int sum=0,k=0,i,j;
    for(i=0;i<4;i++)
    sum+=tableCardNum[i]; //옮길 카드 개수 세기
    struct card temp[sum];
    for(i=0;i<4;i++)
    {
        for(j=0;j<tableCardNum[i];j++)
        {
            temp[k]=clntCards[i][j]; //옮길 카드 temp에 임시 저장
            k++;
        }
    }
    for(i=0;i<sum;i++)
    {
        clntCards[threadNum][clientCardNumber[threadNum]+tableCardNum[threadNum]+i]=temp[i]; //테이블에 있던 카드 벨 누른 플레이어에게 주기
    }
    
    for(i=0;i<4;i++)
    {
        if(i==threadNum)
            clientCardNumber[threadNum]+=sum; //벨 누른 사람 카드 개수 증가
        pullCard(i,tableCardNum[i]); //테이블에 있던 카드 수 만큼 카드 구조체 당겨줌
        tableCardNum[i]=0; //테이블 카드 없어짐
    }
}

void bell_X(int threadNum)
{
    int i,j=0;
    struct card temp[3];
    for(i=(clientCardNumber[threadNum]+tableCardNum[threadNum]-1);i>=clientCardNumber[threadNum]+tableCardNum[threadNum]-3;i--)
    {
        temp[j]=clntCards[threadNum][i]; //옮길 카드 3장 temp에 임시저장
        clntCards[threadNum][i].num=0; //옮긴 카드 없애기
        clntCards[threadNum][i].color=NULL;
        j++;
    }
    j=0;
    for(i=0;i<4;i++)
    {
        if(threadNum!=i) //벨 잘못 누르지 않은 플레이어
        {
            clntCards[i][clientCardNumber[i]+tableCardNum[i]]=temp[j]; //카드 하나 받음
            j++;
            clientCardNumber[i]++; //카드 한개 획득
        }
        else //벨 잘못 누른 플레이어
        {
            clientCardNumber[i]-=3; //카드 3개 손실
        }
    }
}

void pullCard(int threadNum, int howmany)
{
    for(int i=0;i<clientCardNumber[threadNum];i++)
    {
        clntCards[threadNum][i]=clntCards[threadNum][i+howmany];
    }
    
    for(int i=clientCardNumber[threadNum]+tableCardNum[threadNum]-1;i>=clientCardNumber[threadNum];i--)
    {
        clntCards[threadNum][i].color=NULL;
        clntCards[threadNum][i].num=0;
    }
}

void checkGameEnd()
{
    for(int i=0;i<4;i++)
        if(clientCardNumber[i]<=0) //플레이어 카드 0이하인지 확인
        {
            playerStatus='e';
            for(int j=0;j<4;j++)
            statusCheck[j]=1;
        }
}



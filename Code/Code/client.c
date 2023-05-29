#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "card.h"

int clientSocketDescriptor; //클라이언트 소켓 디스크립터
int i=0;
int numberOfCards;
int clientCardNumber[4]={0,}; // 카드 개수
char playerStatus; //상태 확인
int statusThreadNumber; //상태변화있는 쓰레드 넘버
struct card cardTable[4]; //출력할 테이블위 카드들
char playerName[4][30]; //이름

void *writeSrv(void * parm) //계속 쓰기 쓰레드
{
    int clientSocketDescriptor;
    clientSocketDescriptor=*((int*)parm);
    char wBuff[BUFSIZ];
    int readLen;

    while(playerStatus!='e'){ //e(종료)가 아닐때 서버로 보냄
        //fgets(wBuff, BUFSIZ-1, stdin);
		//readLen=strlen(wBuff);
		usleep(10000);
		wBuff[0] = 't';
		wBuff[1] = '\0';
		readLen = 2;
        write(clientSocketDescriptor,wBuff,readLen-1);
        wBuff[0]='\0';
    }
}

void *readSrv(void * parm) //계속 읽기 쓰레드
{
    int clientSocketDescriptor;
    clientSocketDescriptor=*((int*)parm);

    for(int i=0;i<4;i++) //인쇄할 카드 0으로 초기화
    {
        cardTable[i].num=0;
        cardTable[i].color=NULL;
    }


    int readLen;
    while(1){ 
        recv(clientSocketDescriptor,&playerStatus,sizeof(char),0); //서버로부터 정보 받아옴
        recv(clientSocketDescriptor,&statusThreadNumber,sizeof(int),0);
        recv(clientSocketDescriptor,clientCardNumber,sizeof(int)*4,0);
        recv(clientSocketDescriptor,&numberOfCards,sizeof(int),0);
        
        if(playerStatus=='r' || playerStatus=='y' || playerStatus=='g' || playerStatus=='p') //각 색깔의 카드를 뒤집음
        {
            if(playerStatus=='r')
                cardTable[statusThreadNumber].color="red";
            else if(playerStatus=='y')
                cardTable[statusThreadNumber].color="yellow";
            else if(playerStatus=='g')
                cardTable[statusThreadNumber].color="green";
            else if(playerStatus=='p')
                cardTable[statusThreadNumber].color="purple";
            else ;

            cardTable[statusThreadNumber].num=numberOfCards;
            
            printAliveCard(clientCardNumber,cardTable);
            printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",playerName[0],playerName[1],playerName[2],playerName[3]);
            printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clientCardNumber[0],clientCardNumber[1],clientCardNumber[2],clientCardNumber[3]);
            printf("%s님 카드를 뒤집어주세요!\n",playerName[(statusThreadNumber+1)%4]);
            
            playerStatus='\0';
            statusThreadNumber=-1;
            numberOfCards=0;
        }


        else if(playerStatus=='o') //플레이어가 벨을 눌렀는데 벨누를 상황이 맞는 경우
        {
            printNullCard(); 
            printf("%s님이 벨을 울렸습니다! 테이블 위 카드를 가져갑니다!\n",playerName[statusThreadNumber]);
            printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",playerName[0],playerName[1],playerName[2],playerName[3]);
            printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clientCardNumber[0],clientCardNumber[1],clientCardNumber[2],clientCardNumber[3]);
            for(int i=0;i<4;i++)
            {
                cardTable[i].num=0;
                cardTable[i].color=NULL;
            }
        }

        else if(playerStatus=='x') //플레이어가 벨을 잘못눌렀을 경우
        {
            printf("%s님이 벨을 잘못 눌렀습니다! 한 장씩 카드를 나눠줍니다!\n",playerName[statusThreadNumber]);
            printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",playerName[0],playerName[1],playerName[2],playerName[3]);
            printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clientCardNumber[0],clientCardNumber[1],clientCardNumber[2],clientCardNumber[3]);
        }

        else if(playerStatus=='e') //카드가 없는 플레이어가 생겨 게임종료
        {
            break;
        }
        

    } 
    struct card rank[4]; //랭킹
    struct card temp;
    for(int i=0;i<4;i++) 
    {
        rank[i].color=playerName[i];    //card 구조체의 color에 플레이어이름넣음
        rank[i].num=clientCardNumber[i]; //card 구조체의 num에 카드 수 넣음
    }
    
    for(int i=0;i<3;i++){ 
        for(int j=i+1;j<4;j++)
        {
            if(rank[j].num>rank[i].num){ //rank의 num을 보고 내림차순 정렬
                temp=rank[i];
                rank[i]=rank[j];
                rank[j]=temp;
            }
        }
    }
    printf("\n");
    char* gameoverMsg="GAME OVER";
    printf("< < < < < < < < < < < %s > > > > > > > > > > >\n",gameoverMsg);
    
    printf("🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 RANKING 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇\n");
    printf("          NAME           CARDNUM\n");
    int r=0;
    for(int i=0;i<4;i++){ //랭킹 출력
        if(rank[i].num!=rank[i-1].num)
            r++;
        printf("%d위 : %12s    %5d\n",r, rank[i].color, rank[i].num);
        if(i==3)
        {
            printf("종료하시려면 엔터를 눌러주세요\n");
        }
    }
}

//////////////// 메인 ////////////////
int main(int argc, char** argv)
{
    char *name;
    
    struct sockaddr_in clntAddr;
    int clntAddrLen, readLen, recvByte, maxBuff;
    char wBuff[BUFSIZ];
    char rBuff[BUFSIZ];
    pthread_t thread[2];// clnt 2 thread
    

    if(argc!=2) {
        printf("Usage : %s [IP Address]\n", argv[0]);
    }
    clientSocketDescriptor=socket(AF_INET, SOCK_STREAM,0);//클라이언트 소켓 선언
    
    printf("🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇\n\n");
    printf("                        Halli Galli                        \n\n");
    printf("🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇\n\n");
    
    printf("t : 카드 뒤집기 / b : 벨 누르기\n");

    clientSocketDescriptor=socket(AF_INET, SOCK_STREAM,0); //클라이언트 소켓 선언
    memset(&clntAddr,0,sizeof(clntAddr));
    clntAddr.sin_family=AF_INET;
    clntAddr.sin_addr.s_addr=inet_addr(argv[1]);
    clntAddr.sin_port=htons(9000);
    if(connect(clientSocketDescriptor,(struct sockaddr*)&clntAddr, sizeof(clntAddr))==-1)
    {
        close(clientSocketDescriptor);
    
    }//connect함수를 통해 서버와의 연결을 기다림
    
    do{ printf("영어이름을 입력해주세요(최대12자) : ");
    fgets(wBuff,BUFSIZ-1,stdin);
    readLen=strlen(wBuff);
    }while(readLen>13); //12자 이상이면 다시 입력
    write(clientSocketDescriptor,wBuff,readLen);
    //이름을 입력받아 서버로 write해줌
 
    printf("waiting for other players...\n");
    int playerNum;
    while(1)
    {
        recv(clientSocketDescriptor,(int*)&playerNum,sizeof(int),0); //접속한 플레이어 수 서버로부터 계속 받아옴
        if(playerNum==4) //4명 참가
        {
            printf("GAME START\n");
            break;
        }//게임에 접속한 client 수가 4일 때까지 기다린 후 4인경우 게임 시작
    }

    for(int i=0;i<4;i++) //이름 받아옴
    {
        int tempSize=-1;
        recv(clientSocketDescriptor,(int*)&tempSize,sizeof(int),0); //이름 크기
        if(tempSize>0) {
            recv(clientSocketDescriptor,(char*)playerName[i],tempSize,0); //이름받아옴
            playerName[i][tempSize-1]='\0'; }
    }
 
    printNullCard();
    printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",playerName[0],playerName[1],playerName[2],playerName[3]);

    pthread_create(&thread[0], NULL, readSrv, (void*)&clientSocketDescriptor); //읽기 쓰레드
    pthread_create(&thread[1], NULL, writeSrv, (void*)&clientSocketDescriptor); //쓰기 쓰레드
    
    int res;
    pthread_join(thread[0],(void**) &res); 
    pthread_join(thread[1],(void**) &res);

    printf("END\n");
    close(clientSocketDescriptor);
    return 0;
}



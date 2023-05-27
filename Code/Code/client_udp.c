#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "card.h"

int clntSd; //클라이언트 소켓 디스크립터
int i=0;
int cardNum;
int clntCardNum[4]={0,}; // 플레이어들 카드 개수
char status; //상태
int status_num; //상태변화있는 쓰레드 넘버
struct card CARD[4]; //출력할 테이블위 카드들
char clntName[4][30]; //플레이어들 이름
int clientNum; // my client number

struct sockaddr_in srvAddr[2]; // 0: connect socket, 1: data socket

void *writeSrv(void * parm) //계속 쓰기 쓰레드
{
	int clntSd;
	clntSd=*((int*)parm);
	char wBuff[BUFSIZ];
    int readLen;
	
    while(status!='e'){ //e(종료)가 아닐때 계속 서버로 보냄
		//fgets(wBuff, BUFSIZ-1, stdin);
		//readLen=strlen(wBuff);
		sleep(0.1);
		wBuff[0] = 't';
		wBuff[1] = '\0';
		readLen = 2;
		sendto(clntSd, wBuff, readLen - 1, 0, (struct sockaddr *)&srvAddr[1], sizeof(srvAddr[1]));
		wBuff[0]='\0';
    }
}

void *readSrv(void * parm) //계속 읽기 쓰레드
{
	clock_t start_time, end_time;
    double execution_time;
    int clntSd;
	clntSd=*((int*)parm);

	for(int i=0;i<4;i++) //인쇄할 카드 0으로 초기화
	{
		CARD[i].num=0;
		CARD[i].color=NULL;
	}

    int readLen;
	socklen_t srvAddrLen = sizeof(srvAddr);
	struct sockaddr_in tmp;
	start_time = clock();

    while(1){
		recvfrom(clntSd, &status, sizeof(char), 0, (struct sockaddr *)&tmp, &srvAddrLen);
    	recvfrom(clntSd, &status_num, sizeof(int), 0, (struct sockaddr *)&tmp, &srvAddrLen);
    	recvfrom(clntSd, clntCardNum, sizeof(int) * 4, 0, (struct sockaddr *)&tmp, &srvAddrLen);
    	recvfrom(clntSd, &cardNum, sizeof(int), 0, (struct sockaddr *)&tmp, &srvAddrLen);


		if(status=='r' || status=='y' || status=='g' || status=='p') //플레이어가 각 색깔의 카드를 뒤집음
		{
			if(status=='r')
				CARD[status_num].color="red";
			else if(status=='y')
				CARD[status_num].color="yellow";
			else if(status=='g')
				CARD[status_num].color="green";
			else if(status=='p')
				CARD[status_num].color="purple";
			else ;

			CARD[status_num].num=cardNum;
			
			printAliveCard(clntCardNum,CARD);
			printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",clntName[0],clntName[1],clntName[2],clntName[3]);
			printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clntCardNum[0],clntCardNum[1],clntCardNum[2],clntCardNum[3]);
			printf("%s님 카드를 뒤집어주세요!\n",clntName[(status_num+1)%4]);
			
			status='\0';
			status_num=-1;
			cardNum=0;
		}


		else if(status=='o') //플레이어가 벨을 눌렀음, 벨누를때맞음
		{
			printNullCard(); 
			printf("%s님이 벨을 울렸습니다! 테이블 위 카드를 가져갑니다!\n",clntName[status_num]);
			printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",clntName[0],clntName[1],clntName[2],clntName[3]);
			printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clntCardNum[0],clntCardNum[1],clntCardNum[2],clntCardNum[3]);
			for(int i=0;i<4;i++)
			{
				CARD[i].num=0;
				CARD[i].color=NULL;
			}
		}

		else if(status=='x') //플레이어가 벨을 잘못눌렀음
		{
			printf("%s님이 벨을 잘못 눌렀습니다! 한 장씩 카드를 나눠줍니다!\n",clntName[status_num]);
			printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",clntName[0],clntName[1],clntName[2],clntName[3]);
			printf("[%6d장    ] [%6d장    ] [%6d장    ] [%6d장    ]\n",clntCardNum[0],clntCardNum[1],clntCardNum[2],clntCardNum[3]);
		}

		else if(status=='e') //카드가 없는 플레이어가 생겨 게임종료
		{
			break;
		}
		

	} 
	struct card rank[4]; //랭킹용 구조체
	struct card temp;
	for(int i=0;i<4;i++) 
	{
		rank[i].color=clntName[i];	//card 구조체의 color에 플레이어이름넣음
		rank[i].num=clntCardNum[i]; //card 구조체의 num에 카드 수 넣음
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
	for(int i=0;i<4;i++){ //랭킹 순서대로 출력
		if(rank[i].num!=rank[i-1].num)
			r++;
		printf("%d위 : %12s    %5d\n",r, rank[i].color, rank[i].num);
		if(i==3)
		{
			printf("종료하시려면 엔터를 눌러주세요\n");
		}
	}
	end_time = clock();

	// 실행 시간 계산
	execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	
	// 실행 시간 출력
    printf("Execution Time: %f seconds\n", execution_time);

}

//////////////// M A I N ////////////////
int main(int argc, char** argv)
{
    char *name;
    int readLen, recvByte, maxBuff;
	socklen_t srvAddrLen;
    char wBuff[BUFSIZ];
    char rBuff[BUFSIZ];
    pthread_t thread[2];// clnt 2 thread
    

    if(argc!=2) {
        printf("Usage : %s [IP Address]\n", argv[0]);
    }
    
    printf("🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇\n\n");
    printf("                        Halli Galli                        \n\n");
    printf("🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇 🍓 🍋 🍈 🍇\n\n");
 	
	printf("t : 카드 뒤집기 / b : 벨 누르기\n");

	clntSd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //클라이언트 소켓 선언함
	memset(&srvAddr[0],0,sizeof(srvAddr[0]));
    srvAddr[0].sin_family=AF_INET;
    srvAddr[0].sin_addr.s_addr=inet_addr(argv[1]);
    srvAddr[0].sin_port=htons(9000);

	sprintf(wBuff, "CONNECT");
	sendto(clntSd, wBuff, strlen(wBuff), 0, (struct sockaddr *)&srvAddr[0], sizeof(srvAddr[0]));
	recvfrom(clntSd, &clientNum, sizeof(int), 0, (struct sockaddr *)&srvAddr[0], &srvAddrLen); // receive my client number
    clientNum += 1;
	recvfrom(clntSd, &wBuff, strlen(wBuff), 0, (struct sockaddr *)&srvAddr[1], &srvAddrLen); // receive data socket INIT

	do{ printf("영어이름을 입력해주세요(최대12자) : ");
    fgets(wBuff,BUFSIZ-1,stdin);
    readLen=strlen(wBuff);
	}while(readLen>13); //12자 이상이면 다시 입력

	sendto(clntSd, wBuff, readLen, 0, (struct sockaddr *)&srvAddr[1], sizeof(srvAddr[1]));
    //이름을 입력받아 서버로 sendto()를 이용해 보냄
 
    printf("waiting for other players...\n");
    int playerNum;
    while(1)
    {
        recvfrom(clntSd, (int *)&playerNum, sizeof(int), 0, (struct sockaddr *)&srvAddr, &srvAddrLen); //접속한 플레이어 수 서버로부터 계속 받아옴
        if(playerNum==4) //4명 참가
        {
            printf("GAME START\n");
            break;
        }//게임에 접속한 client 수가 4일 때까지 기다린 후 4인경우 게임 시작
    }

	for(int i=0;i<4;i++) //플레이어들 이름 받아옴
	{
		int tempSize=-1;
		recvfrom(clntSd, (int *)&tempSize, sizeof(int), 0, (struct sockaddr *)&srvAddr, &srvAddrLen); // 이름 크기
		if(tempSize>0) {
			recvfrom(clntSd, (char *)clntName[i], tempSize, 0, (struct sockaddr *)&srvAddr, &srvAddrLen); // 이름 받아옴
			clntName[i][tempSize-1]='\0';
			//printf("I am %s\n",clntName[i]);
		}
	}
 
	printNullCard();
	printf("[%-12s] [%-12s] [%-12s] [%-12s]\n",clntName[0],clntName[1],clntName[2],clntName[3]);

    pthread_create(&thread[0], NULL, readSrv, (void*)&clntSd); //읽기 쓰레드
    pthread_create(&thread[1], NULL, writeSrv, (void*)&clntSd); //쓰기 쓰레드
	
	int res;
	pthread_join(thread[0],(void**) &res); 
    pthread_join(thread[1],(void**) &res);

	printf("END\n");
    close(clntSd);
    return 0;
}

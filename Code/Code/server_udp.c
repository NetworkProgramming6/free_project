#include "server.h"

int turn=0; //누구 차례인지
int tt=-1; //차례temp
char *name[4]; //플레이어 이름
struct card clntCards[4][56]; //플레이어 카드
int num=0; //접속인원
int clntCardNum[4]; //플레이어 카드 개수
int tableCardNum[4]; //테이블 카드 개수
pthread_t thread[4]; //플레이어 4명 쓰레드
int status_num=-1; //몇번 플레이어 상태가 변했는지
char status='\0'; //현재 상태 r,y,g,p:빨강,노랑,초록,보라색 카드 뒤집힘 /  o:벨 맞음 / x:벨 잘못누름 / e:게임종료
int statusCheck[4]={0,}; //1 : 클라이언트로 보내야할거 있음, 0 : 다 보냄
struct sockaddr_in sockets[4];
struct sockaddr_in clientModuleAddr[4];
static pthread_mutex_t mutex; //뮤텍스

int checkSockList(struct sockaddr_in *entry, struct sockaddr_in *list, int count)
{
	int i = 0;
	struct sockaddr_in *ptrSockAddr;
	for(i = 0; i < count; i++) {
		ptrSockAddr = list + i;
		if(memcmp(ptrSockAddr, entry, 
			sizeof(struct sockaddr_in)) == 0)
			return i;
	}
	return -1;
} 

void * client_module(void * data)
{

	int scheduleSd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP
	struct sockaddr_in srvAddr; // each module must have each socket
	struct sockaddr_in clntAddr;
	
	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(9000+num+1);

	char rBuff[BUFSIZ];
	char wBuff[BUFSIZ];
	int readLen=0;
	int res;

	int threadNum=num; //본인 쓰레드 번호 0,1,2,3
	//struct sockaddr_in clientAddr = *((struct sockaddr_in *) data);
	socklen_t clientAddrLen;
	
	//sd = *((int *) data);
	clntAddr = *((struct sockaddr_in *)data);
	clientAddrLen = sizeof(clntAddr);

	sprintf(wBuff, "CONNECT");
	sendto(scheduleSd, wBuff, strlen(wBuff), 0, (struct sockaddr *)&clntAddr, clientAddrLen);

	while(1){
		readLen = recvfrom(scheduleSd, rBuff, sizeof(rBuff) - 1, 0, (struct sockaddr *)&(clntAddr), &clientAddrLen); //플레이어 이름 읽어옴
		res = checkSockList(&clntAddr, sockets, num+1);
		if (res == num)
			break;
	}

	rBuff[readLen]='\0';
	name[num]=rBuff; //name[쓰레드번호]에 이름 저장
	num++;
	printf("welcome %s\n",name[num-1]);
	if(num!=4)
		printf("waiting for other players....\n");
	else
		printf("GAME START\n");
	
	while(1)
	{
		if(num==4)
		{
			sendto(scheduleSd, (int *)&num, sizeof(int), 0, (struct sockaddr *)&clntAddr, clientAddrLen);
			break;
		}
	}

	printf("%s Module Start. SdNumber: %d\n",name[threadNum],scheduleSd);

	int sd = scheduleSd;

	for(int i=0;i<4;i++) //각 플레이어들한테 플레이어들 이름 알려주기
	{
		for (int j=0;j<4;j++)
		{
			int tempSize=0;
			while(name[j][tempSize]!='\0')
			{
				tempSize++;
			}
			sendto(sd, (int *)&tempSize, sizeof(int), 0, (struct sockaddr *)&sockets[i], clientAddrLen);
			sendto(sd, name[j], tempSize, 0, (struct sockaddr *)&sockets[i], clientAddrLen);
		}
	}
	//플레이어 다 모임, 게임 시작

	while(1)
	{
		if(statusCheck[threadNum]==1)
		{
			//printf("%d ",clientModuleAddr[i].sin_port);
			for (int i=0;i<4;i++)
			{
				sendto(sd, &status, sizeof(char), 0, (struct sockaddr *)&sockets[i] ,clientAddrLen);
				sendto(sd, &status_num, sizeof(int), 0, (struct sockaddr *)&sockets[i], clientAddrLen);
				sendto(sd, clntCardNum, sizeof(int) * 4, 0, (struct sockaddr *)&sockets[i], clientAddrLen);
				sendto(sd, &clntCards[tt - 1][tableCardNum[tt - 1] - 1].num, sizeof(int), 0, (struct sockaddr *)&sockets[i], clientAddrLen);
			}

			printf("\n");
			if(status=='e') //종료
			{
				break;
			}
			turn%=4;
			pthread_mutex_lock(&mutex);
			statusCheck[threadNum]=0;
			pthread_mutex_unlock(&mutex);
			if(statusCheck[0]==0 && statusCheck[1]==0 && statusCheck[2]==0 && statusCheck[3]==0)
			{
				status_num=-1;
				status='\0';
				tt=1;
			}
		}
		else ;
		
		///////계속 읽음
		readLen = recvfrom(sd, rBuff, sizeof(rBuff) - 1, 0, (struct sockaddr *)&clntAddr, &clientAddrLen);
        rBuff[readLen] = '\0';
		rBuff[readLen]='\0';
		if(strcmp(rBuff,"t")==0 && thread[turn]==pthread_self())
		{
			tableCardNum[turn]++;
			clntCardNum[turn]--;
			status=*clntCards[turn][tableCardNum[turn]-1].color;
			status_num=turn;
			for(int i=0;i<4;i++){
				pthread_mutex_lock(&mutex);
				statusCheck[i]=1;
				pthread_mutex_unlock(&mutex);
			}

			turn++;
			tt=turn;
			//turn=turn%4;
			rBuff[0]='\0'; 
			checkGameEnd(); //게임 끝나는지 확인
		}
		else if(strcmp(rBuff,"t")==0 && thread[turn]!=pthread_self())	
		{
			;
		}
		else if(strcmp(rBuff,"b")==0)// && status!='o' && status!='x')
		{ //bell을 둘이서 눌렀을때 처리하기?
			if(bell()==1 && status!='o') //벨 누를 상황임, 그턴에  처음누른사람
			{
				bell_O(threadNum);
				status='o';
				status_num=threadNum;
				checkGameEnd(); //게임 끝나는지 확인
			}
			else if(bell()==0) //벨 누를 상황 아님
			{
				bell_X(threadNum);
				status='x';
				status_num=threadNum;
				checkGameEnd(); //게임 끝나는지 확인
			}
			rBuff[0]='\0';
			for(int i=0;i<4;i++)
				statusCheck[i]=1;
		}
			
	}
	close(sd);
	printf("The client is disconnected.\n");
}


int main(int argc, char** argv)
{
	int sd;
	struct sockaddr_in srvAddr, clntAddr;
	int strLen;
	socklen_t clntAddrLen;
 
	struct card firstCards[56]; //순서대로 정렬된 카드
	struct card Cards[56]; //나눠줄 랜덤 카드
	setCard(firstCards); //카드 순서대로 생성
	mixCard(firstCards,Cards); //카드 섞기
	
	int i=0,j=0,k=0;
	for(j=0;j<4;j++) //섞은 카드 14장씩 분배
	{
		for(k=0;k<14;k++)
		{
			clntCards[j][k].color=Cards[i].color;
			clntCards[j][k].num=Cards[i].num;
			i++;
		}
		clntCardNum[j]=14;
		tableCardNum[j]=0;
	}
	//printInformation();

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	printf("                           Halli Galli SERVER\n\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
 
	pthread_mutex_init(&mutex,NULL);
	printf("waiting for players...\n");
	
	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(9000);
 
	if(bind(sd, (struct sockaddr *) &srvAddr, sizeof(srvAddr))==-1)
	{
		printf("BIND ERROR\n");
		return 0;
	}
	
	/*
	if(listen(listenSd, 4)==-1)
	{
		printf("LISTEN ERROR\n");
		return 0;
	}
	*/ // UDP
	clntAddrLen = sizeof(clntAddr);

	while (num < 3) {
		char rBuff[BUFSIZ];
		ssize_t readLen = recvfrom(sd, rBuff, sizeof(rBuff) - 1, 0, (struct sockaddr *)&clntAddr, &clntAddrLen);
		memcpy(&sockets[num], &clntAddr, sizeof(clntAddr));
		//printf("Hello, %s\n",rBuff);

		if (readLen == -1) {
			continue;
		} else {
			printf("A client is connected... %d/4\n",num+1);
		}
		sendto(sd, (int *)&num, sizeof(int), 0, (struct sockaddr *)&clntAddr, clntAddrLen); //client number send
		pthread_create(&thread[num], NULL, client_module, (struct sockaddr *)&clntAddr);
	}
	
	int res[4];
	pthread_join(thread[0],(void**)&res[0]);
	pthread_join(thread[1],(void**)&res[1]);
	pthread_join(thread[2],(void**)&res[2]);
	pthread_join(thread[3],(void**)&res[3]);
	
	pthread_mutex_destroy(&mutex);
	close(sd);
	return 0;
}

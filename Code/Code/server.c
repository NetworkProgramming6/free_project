#include "server.h"

int turn=0; //누구 차례인지
int tt=-1; //차례temp
char *name[4]; //플레이어 이름
struct card clntCards[4][56]; //플레이어 카드
int num=0; //접속인원
int clntCardNum[4]; // 사용자 카드 개수
int tableCardNum[4]; //테이블 카드 개수
pthread_t thread[4]; //플레이어 4명 쓰레드
int status_num=-1; //몇번 플레이어 상태가 변했는지
char status='\0'; //현재 상태 r,y,g,p:빨강,노랑,초록,보라색 카드 뒤집힘 /  o:벨 맞음 / x:벨 잘못누름 / e:게임종료
int statusCheck[4]={0,}; //1 : 클라이언트로 보내야할거 있음, 0 : 다 보냄
static pthread_mutex_t mutex; //뮤텍스

void * client_module(void * data)
{
	clock_t start_time, end_time;
    double execution_time;
	char rBuff[BUFSIZ];
	char wBuff[BUFSIZ];
	int readLen=0;
	int connectSd;
	int threadNum=num; //본인 쓰레드 번호 0,1,2,3

	connectSd = *((int *) data);
	readLen=read(connectSd,rBuff , sizeof(rBuff)); //플레이어 이름 읽어옴
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
			send(connectSd,(int*)&num,sizeof(int),0);
			break;
		}
	}

	start_time = clock();
	for(int i=0;i<4;i++) //각 플레이어들한테 플레이어들 이름 알려주기
	{
		int tempSize=0;
		while(name[i][tempSize]!='\0')
		{
			tempSize++;
		}
		send(connectSd,(int*)&tempSize,sizeof(int),0);
		send(connectSd,name[i],tempSize,0);
	}
	//플레이어 다 모임, 게임 시작
	 
	while(1)
	{
		if(statusCheck[threadNum]==1)
		{
			send(connectSd,&status,sizeof(char),0);
			send(connectSd,&status_num,sizeof(int),0);
			send(connectSd,clntCardNum,sizeof(int)*4,0);
			send(connectSd,&clntCards[tt-1][tableCardNum[tt-1]-1].num,sizeof(int),0);
			if(status=='e') //종료
			{
			//	send(connectSd,&status, sizeof(char),0);
			//	exit(1);
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
		readLen=recv(connectSd, rBuff,sizeof(rBuff)-1,MSG_DONTWAIT);
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

	end_time = clock();
	// 실행 시간 계산
	execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	
	// 실행 시간 출력
    printf("Execution Time: %f seconds\n", execution_time);
	printf("The client is disconnected.\n");
}


int main(int argc, char** argv)
{
	int listenSd, connectSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, strLen;
 
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
	
	listenSd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(9000);
 
	if(bind(listenSd, (struct sockaddr *) &srvAddr, sizeof(srvAddr))==-1)
	{
		printf("BIND ERROR\n");
		return 0;
	}
	if(listen(listenSd, 4)==-1)
	{
		printf("LISTEN ERROR\n");
		return 0;
	}
	
	clntAddrLen = sizeof(clntAddr);
	while(num<3) 
	{
		if(num==4)
			break;
		connectSd = accept(listenSd,(struct sockaddr *) &clntAddr, &clntAddrLen);		
		printf("%d %d\n",listenSd,connectSd);
		if(connectSd == -1)
		{
			continue;
		}
		else
		{
			printf("A client is connected... %d/4\n",num+1);
		}	
		pthread_create(&thread[num], NULL, client_module, (void *) &connectSd);	
			
			
	}
	
	int res[4];
	pthread_join(thread[0],(void**)&res[0]);
	pthread_join(thread[1],(void**)&res[1]);
	pthread_join(thread[2],(void**)&res[2]);
	pthread_join(thread[3],(void**)&res[3]);
	
	pthread_mutex_destroy(&mutex);
	close(listenSd);
	close(connectSd);
	return 0;
}

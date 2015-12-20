#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
#define TRUE true
#define FALSE false
#define MAX_BUFF 256

//Carlos Daniel de Sousa Pinho 2014208236
//Dinis Coelho Marques 2014206409

//DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number
 
    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
 
//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
//Structure of a Query
struct QUERY
{
    unsigned char *name;
    struct QUESTION *ques;
};

//Fila dos pedidos
typedef struct prioridade_list *PLIST;
typedef struct prioridade_list{
	struct QUERY *name;
	struct sockaddr_in dest;
	unsigned short id;
	int sockfd;
	PLIST next;
}Prioridade_list;

//Estrutura da configuração
typedef struct dns_config{
	bool flag_start;
	bool flag_stop;
	bool flag_init;
	int n_threads;
	char localdomain[16];
	char namedpipestatistics[32];
	char domains[][16];
}DNS_config;

//Estrutura data e hora
typedef struct data_hora{
	int dia, mes, ano, hora, min, sec;
}Data_Hora;

/*------ dns_server.c ------*/
void dns_server(int , char *[]);
void convertName2RFC (unsigned char*,unsigned char*);
unsigned char* convertRFC2Name (unsigned char*,unsigned char*,int*);
void sendReply(unsigned short, unsigned char*, int, int, struct sockaddr_in);
int verifica(unsigned char *,char *);

/*------ g_config.c ------*/
void read_config();

/*------ g_statistics.c ------*/
void get_stats();
void sigAlarm(int);

/*------ g_requests.c ------*/
char* mmap_localdns();
void sigUSR1(int);
void sigINT(int);
void killSons(int);
void apagalistas(PLIST);
void killThread(int );

void cria_threads();

void *worker(void *);
void pipe_estatisticas();
void cleanup();

Data_Hora data_sistema();

void add_request(PLIST, struct QUERY , struct sockaddr_in , int ,unsigned short);

sem_t *mutex;
char *local;
int shmid;
DNS_config *config;
PLIST local_list;
PLIST ext_list;
Data_Hora inicial;
Data_Hora final;
int t_pedidos,n_pedidos_r, n_d_local, n_d_externos;
pthread_t *p_threads;
char *aux_pbuffer;
int estatisticas;
pid_t pid_estat;
pid_t pid_config;
int size_mmap;


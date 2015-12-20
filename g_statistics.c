#include "main.h"
//Carlos Daniel de Sousa Pinho 2014208236
//Dinis Coelho Marques 2014206409

int fd;
/*LER E IMPRIMIR O PIPE*/
void get_stats(){
    signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGINT, sigINT);
	signal(SIGTERM,SIG_IGN);

    char buf[MAX_BUFF];
    if ((fd = open(config->namedpipestatistics,O_RDONLY)) < 0) {
        perror("Cannot open pipe for reading: ");
    	exit(0);
    }
    printf("\nGestor de estatísticas iniciado...\n");
    while(1){
        read(fd, buf,MAX_BUFF);
        printf("\n%s\n", buf);
    }
    close(fd);
}

void sigAlarm(int sig) {
 	signal(SIGALRM, sigAlarm);
	signal(SIGUSR1,sigUSR1);
	signal(SIGINT, sigINT);
	signal(SIGTERM,SIG_IGN);
	if((fd = open(config->namedpipestatistics,O_WRONLY))<0){
        perror("Cannot open pipe for writing: ");
        exit(0);
    }
   	char buffer_temp[MAX_BUFF];
	sprintf(buffer_temp,"Data arranque= %d:%d:%d\nPedidos, total =%d, recusados=%d\nEnderecos, local=%d, externos=%d\nData final=%d:%d:%d\n",inicial.hora,inicial.min,inicial.sec,t_pedidos,n_pedidos_r,n_d_local,n_d_externos,final.hora,final.min,final.sec);
	write(estatisticas,buffer_temp,MAX_BUFF);
	alarm(30);
	close(fd);
}

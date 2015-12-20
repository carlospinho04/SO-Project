#include "main.h"
//Carlos Daniel de Sousa Pinho 2014208236
//Dinis Coelho Marques 2014206409

void killSons(int sig){
	signal(SIGTERM,killSons);
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);


    exit(0);
}
void sigINT(int sig) {
	signal(SIGTERM,SIG_IGN);
	signal(SIGINT, sigINT);
	signal(SIGALRM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    kill(pid_estat,SIGTERM);
    kill(pid_config,SIGTERM);
	cleanup();

}
void killThread(int sig){
   	pthread_exit(NULL);
}

void sigUSR1(int sig){

	signal(SIGALRM, sigAlarm);
    signal(SIGUSR1, sigUSR1);
    signal(SIGTERM,killSons);
	signal(SIGINT, sigINT);

	if(config->flag_start==FALSE){
		config->flag_start=TRUE;
		config->flag_stop=FALSE;
        printf("Modo de manutencao iniciado\n");
	} else if (config->flag_start==TRUE){
		config->flag_stop=TRUE;
        printf("Modo de manutencao concluido\n");
	}
}



int main(int argc , char *argv[]){

	inicial=data_sistema();
	final=data_sistema();
	signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM,SIG_IGN);

	/*Criar e Mapear MEMORIA*/
	if ((shmid=shmget(IPC_PRIVATE,sizeof(DNS_config),IPC_CREAT | 0777))<0){
        perror("Error in shmget with IPC_CREAT\n"); 
        exit(0);
    }

	config=(DNS_config*)shmat(shmid,NULL,0);
	sem_unlink("mutex");
	mutex = sem_open("mutex", O_CREAT | O_EXCL, 0700, 0);
	config->flag_start=FALSE;
	config->flag_stop=FALSE;
	config->flag_init=TRUE;

	/*Pipe estatisticas*/
	t_pedidos=0;n_pedidos_r=0; n_d_local=0;n_d_externos=0;



	/*Fork()*/
	int gestor_estat;
	int gestor_config=fork();	
	if(gestor_config!=0){
		gestor_estat=fork();
		if(gestor_estat!=0){
			/*GESTOR PEDIDOS*/
			pid_t pid_pai;
			pid_pai=getpid();
			printf("\nPID do processo de pedidos: %d\n",pid_pai);
			sleep(1);
			signal(SIGALRM,sigAlarm);
			signal(SIGUSR1, sigUSR1);
			pipe_estatisticas();
			alarm(30);
			/*Mapear o ficheiro localdns*/
			printf("\nMapeamento da memoria a iniciar ...\n");
			local=malloc(sizeof(char));
			local=mmap_localdns();
			printf("\nMapeamento da memoria bem sucedida ...\n");
 
			if ((estatisticas = open(config->namedpipestatistics, O_WRONLY)) < 0) {
        		perror("Cannot open pipe for writing: ");
        		exit(1);
    		}

 			char temp_buf[MAX_BUFF];
			sprintf(temp_buf,"\nData arranque= %d:%d:%d\nPedidos, total =%d, recusados=%d\nEnderecos, local=%d, externos=%d\nData final=%d:%d:%d",inicial.hora,inicial.min,inicial.sec,t_pedidos,n_pedidos_r,n_d_local,n_d_externos,final.hora,final.min,final.sec);
			write(estatisticas,temp_buf,MAX_BUFF);

			signal(SIGINT,sigINT);
			/*PEDIDOS*/
			cria_threads();
			dns_server(argc,argv);
			

		}
		else{
			/*GESTOR CONFIGURAÇOES*/
			/*Ler a config para a memoria partilhada*/
			signal(SIGALRM,SIG_IGN);
			signal(SIGINT, sigINT);
			signal(SIGTERM,killSons);
			signal(SIGUSR1,SIG_IGN);
			pid_config=getpid();
			while(1){
				if(config->flag_init==TRUE||(config->flag_start==TRUE && config->flag_stop==TRUE)){
					read_config();
					printf("\nFicheiro congig.txt carregado com sucesso para a memoria partilhada!\n");
					config->flag_init=FALSE;
					config->flag_stop=FALSE;
					config->flag_start=FALSE;
				}
			}

			exit(0);
			

		}
	}
	else{
		/*GESTOR ESTATISTICAS*/
		pid_estat=getpid();
		sleep(2);
		signal(SIGALRM,SIG_IGN);
		signal(SIGINT, sigINT);
		signal(SIGTERM,killSons);
		signal(SIGUSR1,SIG_IGN);
		get_stats();
		exit(0);
	}

	return 0;
}

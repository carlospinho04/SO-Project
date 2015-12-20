#include "main.h"
//Carlos Daniel de Sousa Pinho 2014208236
//Dinis Coelho Marques 2014206409

/*MAPEAR O FICHEIRO LOCALDNS.TXT*/
char* mmap_localdns(){
	signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	int ldns;
	struct stat size;
	char *localdns;
	ldns=open("localdns.txt",O_RDONLY);
	stat("localdns.txt", &size);
	size_mmap=size.st_size;
	localdns = mmap((caddr_t)0, size.st_size*sizeof(char),PROT_WRITE, MAP_PRIVATE, ldns, 0);
	close(ldns);
	return localdns;
}


/* THREADS */
void cria_threads(){
	signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	pthread_t threads[config->n_threads];
    p_threads=threads;
	int id[config->n_threads];
	int i=0;
	for(i=0;i<config->n_threads;i++){
		id[i]=i;
		pthread_create(&threads[i],NULL,worker,&id[i]);
	}
}

void pipe_estatisticas(){
	signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	if (mkfifo(config->namedpipestatistics,O_CREAT|O_EXCL|0600)<0 && (errno != EEXIST)){
		perror("MKFIFO ERROR");
		exit(1);
	}
}
void* worker(void* idp){
	signal(SIGINT, sigINT);
	signal(SIGALRM, sigAlarm);
    signal(SIGUSR1, sigUSR1);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR2,killThread);

    	while(1){
                sem_wait(mutex);
                PLIST lant, latual, eant,eatual;
    			latual=local_list->next;
    			lant=local_list;
    			eatual=ext_list->next;
    			eant=ext_list;
                if(latual!=NULL){
                        //Procurar o dominio no localdns.txt
                        char *aux_domainName=(char*)latual->name->name;
                        char *aux_ip;
                        aux_ip=malloc(sizeof(char));
                        char *aux_buffer;
                        aux_buffer=malloc(strlen(local)+1);
                        strcpy(aux_buffer,local);
                        const char tok_1[2]=" ";
                        const char tok_2[2]="\n";
                        char *token;
                		int check=0;
                        token=strtok(aux_buffer,tok_1);
                        while(token!=NULL){
                                if (strcmp(token,aux_domainName)==0){
                                        token=strtok(NULL,tok_2);
                                        strcpy(aux_ip,token);
                                        check=1;
                                        break;
                                }
                                token=strtok(NULL,tok_2);
                                token=strtok(NULL,tok_1);
                        }
                        if(check==1){
                                sendReply(latual->id,latual->name->name,inet_addr(aux_ip),latual->sockfd,latual->dest);
                                n_d_local++;
                            }
                        else{
                                sendReply(latual->id,latual->name->name,inet_addr("0.0.0.0"),latual->sockfd,latual->dest);
                                n_d_local++;
                            }
                        lant->next=latual->next;
                }
                else if(eatual!=NULL){
                        //Utilizar o dig para obter o IP da resposta
                        FILE *p_dig;
                        char *aux_domainName=(char*)eatual->name->name;
                        char aux_ip[64];
                        char dig_cmd[64];
                        sprintf(dig_cmd,"dig +short %s",aux_domainName);
                        p_dig=popen(dig_cmd,"r");
                        if(!p_dig){
                                perror("Erro pipe dig");
                                exit(1);
                        }
                        if(fscanf(p_dig,"%[^\n]",(char*)&aux_ip)!=1){
                                sendReply(eatual->id,eatual->name->name,inet_addr("0.0.0.0"),eatual->sockfd,eatual->dest);
                                n_d_externos++;
                        }
                        else{
                                sendReply(eatual->id,eatual->name->name,inet_addr(aux_ip),eatual->sockfd,eatual->dest);
                                n_d_externos++;
                        }
                        pclose(p_dig);
                        eant->next=eatual->next;
                }
                t_pedidos++;
                final=data_sistema();
        }
}


/*CLEANUP*/
void cleanup(){
	signal(SIGTERM,SIG_IGN);
	signal(SIGINT, sigINT);
	signal(SIGALRM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    int i;
	for(i=0; i < config->n_threads; i++){
		pthread_kill(p_threads[i], SIGUSR2);
	}
   for(i=0;i<config->n_threads;i++){
    	printf("\nThread[%d] is going to die!\n",i);
		pthread_join(p_threads[i],NULL);
	}
     while(wait(NULL) != -1){
		printf("\nChild processe terminated\n");
    }
    apagalistas(local_list);
    apagalistas(ext_list);

    munmap(local,size_mmap);
    printf("\nRemocao da memoria mapeada com sucesso!\n");
    unlink(config->namedpipestatistics);

    sem_close(mutex);
    unlink("mutex");
    printf("\nRemocao do semaforo com sucesso!\n");

    shmdt(config);

    shmctl(shmid, IPC_RMID,NULL);
    printf("\nRemocao da memoria partilhada com sucesso!\n");
    exit(0);

}

/*DATA DO SISTEMA*/
Data_Hora data_sistema(){
	signal(SIGALRM, sigAlarm);
	signal(SIGUSR1,sigUSR1);
	signal(SIGINT, sigINT);
	signal(SIGTERM,SIG_IGN);
	Data_Hora data;

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	data.dia = t->tm_mday;
	data.mes = t->tm_mon+1;
	data.ano = t->tm_year+1900;
	data.hora = t->tm_hour;
	data.min = t->tm_min;
	data.sec = t->tm_sec;
	
	return data;
}

void apagalistas(PLIST ldomains){

    PLIST aux;
    if (ldomains!=NULL){
        while (ldomains->next!=NULL){
            aux=ldomains;
            ldomains=ldomains->next;
            free(aux);
        }
        printf("\nLista apagada com sucesso!\n");
    }
    else 
    	printf("\nErro ao apagar as listas!\n");
}
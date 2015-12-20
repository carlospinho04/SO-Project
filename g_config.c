#include "main.h"
//Carlos Daniel de Sousa Pinho 2014208236
//Dinis Coelho Marques 2014206409

/*LER FICHEIRO CONFIG.TXT*/

void read_config(){
	signal(SIGALRM,SIG_IGN);
	signal(SIGUSR1,sigUSR1);
	signal(SIGINT, SIG_IGN);
	signal(SIGUSR2,SIG_IGN);
	FILE *c;
	c=fopen("config.txt","r");


	char domains_aux[512];
	if(config->flag_init==TRUE){
		fscanf(c,"Threads = %d\n",&config->n_threads);
		fscanf(c,"Domains = %[^\n]%*c",(char*)&domains_aux);
		fscanf(c,"LocalDomain = %[^\n]%*c",(char*)&config->localdomain);
		fscanf(c,"NamedPipeEstatisticas = %[^\n]%*c",(char*)&config->namedpipestatistics);
	}
	else{
		int n_threads_aux;
		char *name_pipe_aux;
		fscanf(c,"Threads = %d\n",&n_threads_aux);
		fscanf(c,"Domains = %[^\n]%*c",(char*)&domains_aux);
		fscanf(c,"LocalDomain = %[^\n]%*c",(char*)&config->localdomain);
		fscanf(c,"NamedPipeEstatisticas = %[^\n]%*c",(char*)&name_pipe_aux);

	}
	
	fclose(c);
	int i=0;
	char *token = strtok(domains_aux,"; ");
	while (token) {
		strcpy(config->domains[i],token);
    	i++;
    	token = strtok(NULL, "; ");
	}
	strcpy(config->domains[i],"\0");
}

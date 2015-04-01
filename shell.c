#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <errno.h> 

int global_pipes;

struct que
{
 int id;
 int pid;
 char pfname[100];
 char pname[100];
 struct que *next;
};
typedef struct que pinfo;

struct linklist
{	
	char  *argv[64];
	char  *out[64];
	char  *in[64];
	struct linklist * next;
};
typedef struct linklist process;	

char* insert_process( process **head, char *line)
{
	//printf("%c\n",*line);
	int i=0;
	int flag=0;
	process *node , *temp;
	
	node=(process*) malloc(sizeof(process));
	node-> next= NULL;
		
	while (*line != '\0') 
	{     
		//printf("%c\n",*line);
		
		while (*line == ' ' || *line == '\t' || *line == '\n' || *line =='&' || *line == '>' || *line=='<')
		{
			if(*line == '>')
			{
				*line++ = '\0';
				
				while (*line == ' ' || *line == '\t')
					*line++ = '\0';
				
				node->out[0] = line;
				
			//	printf("%s\n",node->out);
				
				while (*line != '\0' && *line != ' ' &&  *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
					line++;   
			}
			else if(*line == '<')
			{
				*line++ = '\0';
				
				while (*line == ' ' || *line == '\t')
					*line++ = '\0';
				
				node->in[0] = line;
				//printf("%s\n",node->in);
				
				while (*line != '\0' && *line != ' ' &&  *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
					line++;   
			}
			else
				*line++ = '\0';   
		}
		
		if(*line == '|')
		{
			flag++;
			global_pipes++;
			*line++ = '\0';  
			break; 
		}
		
		if(*line != '\0')
		{
			node->argv[i] = line; 
			//printf("%s\n",node->argv[i]);
			i++;
		}
		

		while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
			line++;          
	}
		
	node->argv[i] = '\0';   
	node->out[1] = '\0';   
	node->in[1] = '\0';
	
	if ( *head==NULL)
		*head=node;		
	else
	{
		temp=*head;
		while(temp->next !=NULL)
			temp=temp->next;
		
		temp-> next=node;	
	}
	return line;
}

void display_process(process *head)
{
	while(head!= NULL)
	{
		//int i=0;
		/*printf("\nin dis\targv =%s\t out=%s \t in=%s\n",head->argv[i],head->out[0],head->in[0]);
		while(head->argv[i]!='\0')
		{
			i++;
			printf("\targv=%s",head->argv[i]);
		}
		printf("\n");*/
		head=head->next;
	}
	fflush(stdin);
	fflush(stdout);
	
}

pinfo* enque(pinfo *head,int cpid,char cpname[],char cpfname[])
{
	pinfo *node,*temp;
	int j=2;
	if(head==NULL)
	{
		head=(pinfo*) malloc(sizeof(pinfo));
		head->pid=cpid;
		head->id=1;
		strcpy(head->pname,cpname);
		strcpy(head->pfname,cpfname);
		head->next=NULL;
	}
	else
	{
		node=(pinfo*) malloc(sizeof(pinfo));
		temp=head;
		while(temp->next!=NULL)
		{
			temp=temp->next;
			j++;
		}
		
		node->pid=cpid;
		node->id=j;
		strcpy(node->pname,cpname);
		strcpy(node->pfname,cpfname);
		node->next=NULL;
		temp->next=node;
		//printf("node: %s",node->pname);
	}
	return head;
}

pinfo* display(pinfo *head)
{
	pinfo *temp;

	temp=head;
	int n;
	while(temp!= NULL)
	{
		fflush(stdin);
		fflush(stdout);
		n=kill( (pid_t)temp->pid , 0 );
		if(n==0)
			printf("[%d] %s  [%d]\n",temp->id,temp->pname,temp->pid);
		temp=temp->next;
	}
	
	return head;
}

process * parse( char *line)
{
	//puts(line);
	process * head=NULL;

	while(*line != '\0')
		line= insert_process(&head, line);
		
	display_process(head);
	
	return head;
}

void  parser(char *line, char **argv,char **in,char **out)
{
	while (*line != '\0') 
	{     	
		while (*line == ' ' || *line == '\t' || *line == '\n' || *line =='&' || *line == '>' || *line=='<')
		{
			if(*line == '>')
			{
				*line++ = '\0';
				
				while (*line == ' ' || *line == '\t')
					*line++ = '\0';
				
				*out++ = line;
				
				while (*line != '\0' && *line != ' ' &&  *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
					line++;   
			}
			else if(*line == '<')
			{
				*line++ = '\0';
				
				while (*line == ' ' || *line == '\t')
					*line++ = '\0';
				
				*in++ = line;
				
				while (*line != '\0' && *line != ' ' &&  *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
					line++;   
			}
			else
				*line++ = '\0';   
		}
		
		if(*line != '\0')
			*argv++ = line;          
		 
		while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n' && *line != '&' && *line != '>' && *line != '<')
			line++;          
	}
	
	*argv = '\0';   
	*out = '\0';   
	*in = '\0';          
}

pinfo * execute1(pinfo *head,char **argv,char  **in,char ** out,int mode)
{	
	pid_t  pid;
	int    status;
	char command[100];
	char file[100];
	int output_fd;
	int input_fd;
	
	if ((pid = fork()) < 0)    
	{ 
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}	

	else if (pid == 0)    /* for the child process:         */
	{        
		//int i=0;
		while(*out!=NULL)
		{
			output_fd = open(*out, O_CREAT|O_WRONLY, S_IRWXU);
			dup2(output_fd,1);
			out++;
		}
		while(*in!=NULL)
		{
			input_fd = open(*in, O_CREAT|O_RDONLY, S_IRWXU);
			dup2(input_fd,0);

			in++;
		}
		
		if (execvp(*argv, argv) < 0) 
		{     /* execute the command  */
			printf("*** ERROR: no such comand found\n");
			exit(1);
	    }
	}
	
	else 					/* parent process */
	{
		if(mode==0)
		{		
			//head=display(head);			
			while (wait(&status) != pid)      
				;			
		}
		else
		{
			if(argv[1]!=NULL)
			{
				strcpy(command,argv[0]);
				strcpy(file,argv[1]);
				argv[2]=NULL;
				head=enque(head,pid,command,file);	
			}
			else
			{
				printf("** error invalid statement\n");
			}
		}	
	}
	return head;
}
   
pinfo * execute(process *root ,pinfo *head,int mode)
{
	pid_t  pid;
	int    status;
	
	char command[100];
	char file[100];
	
	int output_fd;
	int input_fd;
	
	int saved_stdout = dup(1);
	int saved_stdin= dup(0);
	
	if(root->in[0] != NULL)
	{
		close(STDIN_FILENO);
		input_fd = open(root->in[0], O_CREAT|O_RDWR, S_IRWXU);
		dup2(input_fd,0);
	}
	
	if(root->out[0] != NULL)
	{
		close(STDOUT_FILENO);
		output_fd = open(root->out[0], O_CREAT|O_WRONLY, S_IRWXU);
		dup2(output_fd,1);
	}
		
	if ((pid = fork()) < 0)    
	{ 
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}	

	else if (pid == 0)    /* for the child process:         */
	{   		
		if (execvp(root->argv[0], root->argv) < 0)  /* execute the command  */
		{  	
			printf("*** ERROR: no such comand found\n");
			exit(1);
		}
	}
	else		/* parent process */
	{
		if(mode==0)
		{			
			while (wait(&status) != pid)      
				;			
		}
		else
		{
			if(root->argv[1]!=NULL)
			{
				strcpy(command,root->argv[0]);
				strcpy(file,root->argv[1]);
				root->argv[2]=NULL;
				head=enque(head,pid,command,file);	
			}
			else
			{
				printf("** error invalid statement\n");
			}
		}	
	}

	dup2(saved_stdout, 1);
	close(saved_stdout);
	dup2(saved_stdin, 0);
	close(saved_stdin);
	
	return head;
}

pinfo * executepipes(process *root ,pinfo *head,int mode)
{	
	pid_t  pid;
	
	//int count;
	
	int output_fd;
	int input_fd;
	int status;
	int saved_stdout = dup(1);
	int saved_stdin= dup(0);
	
	int closepipe;
	
	//int fd[2];
	//pipe(fd);
	
	int fd[100];
	int pipes=0;
	int i;
	for(i=0; i< 50; i++)
		pipe(fd+i*2);
		
	if(root->in[0] != NULL)
	{
		close(STDIN_FILENO);
		input_fd = open(root->in[0], O_CREAT|O_RDWR, S_IRWXU);
		dup2(input_fd,0);
	}
	int count =0;
	pipes=0;
	while(root != NULL)
	{	
		if ((pid = fork()) < 0)    
		{ 
			fprintf(stderr, "can't fork, error %d\n", errno);
			exit(EXIT_FAILURE);
		}	

		else if (pid == 0)    /* for the child process:         */
		{   
			if (count>0)
			{
				dup2(fd[(pipes-1)*2],0);	
				if(root->next ==NULL)
				{
					for(closepipe=0;closepipe<100;closepipe++)
						close(fd[closepipe]);
				}
			}
			
			if(root->next == NULL)
			{
				if(root->out[0] != NULL)
				{
					//close(STDOUT_FILENO);
					output_fd = open(root->out[0], O_CREAT|O_WRONLY, S_IRWXU);
					dup2(output_fd,1);
				}
			}
			else
			{
				dup2(fd[pipes*2+1],1);   //redirect output of pipe to fd1
				
			}
			for(closepipe=0;closepipe<100;closepipe++)
					close(fd[closepipe]);
			
			if (execvp(root->argv[0], root->argv) < 0)  /* execute the command  */
			{  	
				printf("*** ERROR: no such comand found\n");
				exit(1);
			}
		}
		pipes=pipes+1;
		root=root->next;
		count++;		
		//dup2(fd[pipes],0);
		//pipes=pipes+2;
	}
	for(closepipe=0;closepipe<100;closepipe++)
		close(fd[closepipe]);
	
	int j;
	for(j=0;j<count;j++)
		wait(&status);
    	
	dup2(saved_stdout, 1);
	close(saved_stdout);
	dup2(saved_stdin, 0);
	close(saved_stdin);
	
	fflush(stdout);
	fflush(stdin);
	return head;
}

pinfo * kjob(pinfo *head,char **argv)
{
	if (head != NULL)
	{
		int kuchbhi;
		pinfo *temp;
		
		if(argv[1] == NULL || argv[2] == NULL)
		{
			printf("**ERROR not enought arguments\n");
			return head;
		}
		
		temp=head;
		
		kuchbhi=atoi(argv[1]);
			
		while(kuchbhi != temp->id && temp!=NULL)
			temp=temp->next;

		if(kuchbhi ==temp->id)
		{
			kill(temp->pid,atoi(argv[2]));
			return head;
		}
		else
		{
			printf("**error id not found **\n");
			return head;
		}

	}
	
	else
		printf("**ERROR no process found\n");
		
	return head;
}

pinfo * fg(pinfo *head,char **argv)
{
	if(head != NULL)
	{
		pinfo * temp;
		int kuchbhi;
		char  line1[1024] ;
		char  *out1[64];
		char  *in1[64];
		char  *argv1[64];
		int flag=0;
		if(argv[1] == NULL )
		{
			printf("**ERROR not enought arguments\n");
			return head;;
		}
		
		temp=head;
		
		kuchbhi=atoi(argv[1]);
		
		while(kuchbhi != temp->id && temp!=NULL)
			temp=temp->next;
		
		if(kuchbhi ==temp->id)
		{
			kill(temp->pid,9);
			sprintf(line1,"%s %s",temp->pname,temp->pfname);
			//strcpy(line1,temp->pname);
			parser(line1, argv1,in1,out1);
			execute1(head,argv1,in1,out1,flag);
			fflush(stdin);
			fflush(stdout);
			return head;;
		}
		else
		{
			printf("**error id not found **\n");
			return head;
		}
	}
	else
		printf("***ERROR no such process found\n");
	
	return head;
}

pinfo * overkill(pinfo *head)
{
	pinfo * temp;
	temp=head;
		while(temp!=NULL)
		{
			kill(temp->pid,9);
			temp=temp->next;
		}
	return head;
}
  
void ppinfo(char ** argv)
{
	char proc[100];
	char status[100];
	char path[100];
	pid_t pid;
	size_t len=0;
	ssize_t read;
	int count=0;
	char *line1;
	line1 = (char *)malloc(1000);
	strcpy(proc,"/proc/");
	strcpy(status,"/status");
	if(argv[1] != NULL)
		pid=atoi(argv[1]);
	else 
		pid=getpid();
	sprintf(path,"%s%d%s",proc,pid,status);

	//printf("%s",path);
	FILE * fp=fopen(path,"r");

	if (fp == NULL)
		exit(0);
	while ((read = getline(&line1, &len, fp)) != -1)
	{ 
		//printf("in loop\n");
		if(count==1)
			printf("%s", line1); 
		 if(count ==3 )
			printf("%s", line1); 
		 if(count ==11) 
			printf("%s", line1); 
		count++; 
	}
	//printf("\nExecutable path : %s\n",curDir);

		
		//char ps[]="ps";
		
		//char psu[]="ps u";
		
		//char u[]="u";
		//char temp[100];
		
		/*	    
		if(argv[1] == NULL)
		{
			strcpy(line1,psu);
			pid_t pid=getpid();
			sprintf(line1,"%s%d",psu,pid);
			parse(line1, argv1);
			
			execute(head,argv1,flag);
			
			continue;
		}
		
		else if(argv[1] != NULL)
		{
			printf("inpinfo");
			sprintf(line1,"%s%s",psu,argv[1]);
			parse(line1, argv1);
			execute(head,argv1,flag);
			
			strcpy(argv[0],ps);
			strcpy(,argv[1]);
			strcpy(argv[2],temp);
			strcpy(argv[1],u);	
				
		}
		*/
}   

void initials(char curDir1[])
{
	char hostname[100];
	char  curDir[100];             
	char *dir;
	int flag=0;
	

	gethostname(hostname,100);
	printf("%s@%s:%s", getlogin(),hostname,"~");
	getcwd(curDir,100);
	if(strlen(curDir) < strlen (curDir1))
	{
		flag++;
		printf("cannot go back from root dir\n");
		chdir(curDir1);
		return;
	}
	int j=0;
	
	while(curDir1[j] != '\0')
		j++;
	dir=&curDir[j];
	printf("<-%s-> ",dir);
	
}
	
int  main(void)
{	
	char  line[1024];           
	
	int mode;
	process * root;
	
	char  curDir[100];
	getcwd(curDir,100);
	
	pinfo *head;
	head=NULL;
	while (1)
	{     
		global_pipes=0;
		initials(curDir); 
		mode=0;            
		
		fflush(stdin);
		fflush(stdout);
		
		gets(line);              /*   read in the command line     */
		//printf("line ==%s\n",line);
		
		if (*line == '\0') 
			continue;

		printf("\n");
		
		int i=0;
		
		while((line[i]) != '\0')
		{
			if(line[i] == '&')
			{
				mode++;
				//line[i]='\0';
			}	
			i++;
		}
		
		//jobs
		if(strcmp(line, "jobs") == 0)
		{
			head=display(head);
			continue;
		}
			
		root=parse(line); 
		
		/*
		int p=0;
		while(argv[p] != NULL)
		{
			printf("argv = %s\n",argv[p]);
			p++;
		}    
		
		p=0	;
		while(out[p] !=NULL)
		{
			printf("out = %s\n",root->out);
			p++;
		} 
		p=0	;
		while(in[p] !=NULL)
		{
			printf("in = %s\n",root->in);
			p++;
		}
		*/
		//pinfo 
		if(strcmp(line, "pinfo") == 0)
		{	
			fflush(stdin);
			fflush(stdout);
			ppinfo(root-> argv);
			continue;	
		}
		
		//overkill
		if(strcmp(root->argv[0],"overkill")==0)
		{
			head=overkill(head);
			continue;
		}
		
		//fg
		else if(strcmp(root->argv[0],"fg")==0)
		{
			head=fg(head,root->argv);
			continue;
		}
		
		//kjob
		else if(strcmp(root->argv[0],"kjob")==0)
		{
			head=kjob(head,root->argv);
			continue;
		}	
		
		//exit
		else if (strcmp(root->argv[0], "quit") == 0) 
			exit(0);           			  
		
		//cd		
		if (strcmp(root->argv[0], "cd") == 0)
		{
				chdir(root->argv[1]);
				continue;
		}
		else
		{	if(global_pipes != 0)
				head=executepipes(root,head,mode);  
			else
				head=execute(root,head,mode);
		}
	}
}

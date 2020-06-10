#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <limits.h>

int postOrderApply(char* path,int sizepathfun(char *path1));
int is_regular_file(const char *path);
int is_dir(const char *path);
int is_link(const char *path);
int is_pipe(const char *path);
int sizepathfun(char *path);
int z=0;
char tut[256];
char s[256];
char pc[256];
int chp=0;
char buff[1024];
char * myfifo = "/tmp/141044012sizes";
int sizepipe[2];
int fd;

int main(int argc, char** argv) {
    
    char cwd[256];
    
    int size;
    int p=0;
    
    DIR *dr;
    
    mkfifo(myfifo,0666);
    fd = open(myfifo,O_RDWR);
    
    if(argc==3){
        if(!strcmp(argv[1],"-z")){
            z=1;

                if(argv[2][0]=='/'){
                    strcpy(cwd,argv[2]);
                }
                else{
                    getcwd(cwd,sizeof(cwd));
                    strcat(cwd,"/");
                    strcat(cwd,argv[2]);
                }
            
            size=postOrderApply(cwd,sizepathfun);

            printf("pid:%d -- %d B -- %s\n",getpid(),size,cwd);
            printf("%d child processes created. Main process is %d\n", chp,getpid());
        }
        else{
            printf("Wrong Format! Usage: ./buNeDu -z Path or ./buNeDu Path\n");
        }

    }
    else if(argc==2){
        z=0;
        if(!strcmp(argv[1],"-z")){
            printf("Wrong Format! Usage: ./buNeDu -z Path or ./buNeDu Path\n");
        }
        else{    

                if(argv[1][0]=='/'){
                    strcpy(cwd,argv[1]);
                }
                else{
                    getcwd(cwd,sizeof(cwd));
                    strcat(cwd,"/");
                    strcat(cwd,argv[1]);
                }
            


            size=postOrderApply(cwd,sizepathfun);

            printf("pid:%d -- %d B -- %s\n",getpid(),size,cwd);
            printf("%d child processes created.Main process is %d\n", chp,getpid());
        }
        
    }
    else{
        printf("Wrong Format! Usage: ./buNeDu -z Path or ./buNeDu Path \n");
    }
    printf("\nfifo:\n");
    while(wait(NULL)>0){
        
    }
    fd = open(myfifo, O_RDONLY | O_NONBLOCK);
   // for(int k=0;k<chp;k++){
    read(fd, buff, sizeof(buff));
        printf("%s",buff);
    //}
    close(fd);
    return 0;
    
}

int postOrderApply(char* path,int sizepathfun(char *path1)){
    
    struct dirent *de;
    char cwd[256];
    char old[256];
    int size1=0;
    int size2=0;
    int size3=0;
    int totalsize=0;
    int sum=0;
    int cp=0;
    char arr1[1024];
    int p[2];
    int p2[2];
    char buffer[PIPE_BUF];
    
    int fd;
 
    strcpy(cwd,path);
    chdir(cwd);
    DIR *dr = opendir(cwd); 
    strcpy(old,cwd);
    struct stat statbuf;
    if (dr == NULL)  
    { 

        printf("Could not open current directory -> %s",cwd); 
        return 0; 
        
    } 
  
    else{
        
        
        while ((de = readdir(dr)) != NULL){
        strcat(cwd,"/");
        strcat(cwd,de->d_name);
        if(is_regular_file(cwd) && !is_link(cwd) && !is_pipe(cwd)){             
            stat(cwd,&statbuf);
            
            size1+=sizepathfun(cwd);
            sum+=size1;
            totalsize+=size1;
            
            printf("pid:%d -- %d B -- ",getpid(),size1);
            printf("%s\n",cwd);
            size1=0;
            
            
        }
        else if(is_dir(cwd)){
            if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0){             
                stat(cwd,&statbuf);
                
                if (pipe(p) < 0)
                    exit(1);
                if (pipe(p2) < 0) {
                    exit(1);
                }
                
                if(fork()==0){                                
                    size2+=postOrderApply(cwd,sizepathfun);
                    
                    printf("pid:%d -- %d B -- ",getpid(),size2);
                    printf("%s\n",cwd);
                                        
                    sprintf(buffer,"pid:%d -- %d B -- %s\n",getpid(),size2,cwd);
                    fd = open(myfifo, O_WRONLY | O_NONBLOCK);
                    printf("*****pid:%d -- %d B -- ",getpid(),size2);
                    printf("%s\n",cwd);
                    
                    printf("--------%s\n", buffer);
                    
                    write(fd, buffer, strlen(buffer));
                    close(fd);
                    
                    sprintf(arr1, "%d",chp);
                    write(p2[1], arr1, 1024);
                    
                    sprintf(arr1, "%d",size2);
                    write(p[1], arr1, 1024);
                    exit(0);

                }
                else{
                    
                    read(p[0], arr1, 1024);
                    size2=atoi(arr1);
                    totalsize+=size2;
                    
                    read(p2[0], arr1, 1024);
                    cp+=atoi(arr1);
                    cp++;
                    
                    
                }
                
                size2=0;
                
                
            }
        }
        else if(is_link(cwd) || is_pipe(cwd)){            
            stat(cwd,&statbuf);
            
            printf("pid:%d -- Special File  ",getpid());
            printf("%s\n",cwd); 

            size3+=0;
            totalsize+=size3;
            size3=0;
            
            
        }
        strcpy(cwd,old);           
    }
        
        for (int k=0; k<cp; k++) {
            
            wait(NULL);
            
        }
        

    }
    if(z==0){
        totalsize=sum;
    }
    chp=cp;
    
    closedir(dr);
    return totalsize;
}
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
int is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}
int is_link(const char *path)
{
    struct stat path_stat;
    lstat(path, &path_stat);
    return S_ISLNK(path_stat.st_mode);
}
int is_pipe(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISFIFO(path_stat.st_mode);
}
int sizepathfun(char *path){
    struct stat statbuf;
    if(stat(path, &statbuf) == -1){
        perror("Failed to get file status");
        return -1;
    }
    else if(S_ISREG(statbuf.st_mode) > 0){
        return statbuf.st_size;
    }
    else if(S_ISDIR(statbuf.st_mode) > 0){
        return 0;
    }

    return 0;
            
}

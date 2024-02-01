#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int pfd[2];

int occur(char* filename, char key){
	int length = strlen(filename);
	int count = 0;
	for (int i = 0; i < length; i++){
		if (filename[i] == key){
			count ++;
		}
	}
	return count;
}

int* ls(char *root, char key, int* ans)
{
  char buf[202] = "\0", *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(root, 0)) < 0){
    fprintf(2, "%s [error opening dir]\n", root);
    return ans;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "%s [error opening dir]\n", root);
    close(fd);
    return ans;
  }
  if (ans[0] == 0 && ans[1] == 0){
       if(st.type == T_FILE){
    	   fprintf(2, "%s [error opening dir]\n", root);
	       return ans;
       }
       printf("%s %d\n", root, occur(root, key));
  }
  switch(st.type){
  case T_FILE:
    //fprintf(2, "%s [error opening dir]\n", root);
    printf("%s %d\n", root, occur(root, key));
    break;

  case T_DIR:
    if(strlen(root) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    if (strcmp(buf,".") == 1 || strcmp(buf, "..") == 1){
	    return ans;
	}
    //printf("root: %s\n", root);
    strcpy(buf, root);
    buf[strlen(root)] = '/';
    p = buf;
    int l = strlen(p);
    p[l] = '\0';
    //printf("p: %s\n", p);
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      //printf("dename: %s %s\n", p, de.name);
      if(de.inum == 0)
        continue;
      if (strcmp(de.name, "..") == 0){
	continue;	
      }
      if (strcmp(de.name, ".") == 0){
	continue;	
      }
      for(int i = 0; i < DIRSIZ; i++){
      	p[i + l] = de.name[i];
      }
      p[l + DIRSIZ] = '\0';
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d\n", buf, occur(buf, key));
      if (st.type == T_DIR){
    	//printf("in ls: %s\n", p);
        ans[0] ++;
      	ans = ls(p, key, ans);
      }
      else{
        ans[1]++;
      }
    }
    break;
  }
  close(fd);
  return ans;
}


int main(char argc, char *argv[]){
    pipe(pfd);
    if (fork() == 0){
       int* ans = malloc(sizeof(int) * 2);
       ans = ls(argv[1], argv[2][0], ans);
       write(pfd[1], &ans[0], sizeof(int));       
       write(pfd[1], &ans[1], sizeof(int));
       close(pfd[1]);
       exit(0);
    }
    
    int output[2] = {0, 0};
    read(pfd[0], &output[0], sizeof(int));
    read(pfd[0], &output[1], sizeof(int));
    close(pfd[0]);
    printf("\n%d directories, %d files\n", output[0], output[1]);
    
    exit(0);
}




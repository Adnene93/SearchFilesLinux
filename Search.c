#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef enum boolean{
    FALSE,
    TRUE
    }
boolean;

/* global variable definitions */
int recflag=-1;
int dflag=0;
int mflag=0;
int sflag=0;
int tflag=0;
int pflag=0;
int entete=0;
int nbres=0;

char regexp[512]="";
char *divideresult[512];
int sizeofdivideresult=0;
boolean startwith;
boolean endwith;

/* end of global variable definitions */


void divide(){
	int i=0;
	char* s;
	if (regexp[0]=='*') startwith=TRUE; else startwith=FALSE;
	if (regexp[strlen(regexp)-1]=='*') endwith=TRUE; else endwith=FALSE;	
	s = strtok(regexp,"*");
	if (s!=NULL){
		divideresult[i++]=s; 
		while ((s = strtok(NULL,"*"))!=NULL)
		{
			divideresult[i++]=s;
		}	
	}
	sizeofdivideresult=i;
}

boolean equalspart(char* partcmdarg,char* partfilename)
{
	int i=0;
	if (strlen(partcmdarg) == strlen(partfilename)){
		while (i<strlen(partcmdarg))
		{
			if (partcmdarg[i]=='?' || partcmdarg[i]==partfilename[i]) 
			{
            			i=i+1;
			}
			else return FALSE;
        	}
		return TRUE;		
	}	
	return FALSE;	

}

int myindexof(char *needle,char *filename){	
	int i=0,j=0;
	char s[512]="";
	while (i+strlen(needle)<=strlen(filename)){		
		strncpy(s,filename+i,strlen(needle));
		if (equalspart(needle,s)) return i;
		i=i+1;
	}
	return -1;
}

void recuplastchar(char *s,int n,char *ret){
	int i=0;
	int j =	strlen(s)-n;
	while (j<0) j++;
	for (i=j;i<strlen(s);i++) sprintf(ret,"%s%c",ret,s[i]);
}


boolean equals(char* filename){
	char *result[512];
	int n=0,ind=0;
	char subfilename[512];
	sprintf(subfilename,"%s",filename);
	int i=1;
	if(sizeofdivideresult>0){	
		ind = myindexof(divideresult[0],subfilename);
		if (ind<0) return FALSE;	
		if (!startwith && ind>0) return FALSE; 						
		sprintf(subfilename,"%s",subfilename+ind);
		while (i<n-1){
			ind = myindexof(divideresult[i],subfilename);
			if (ind<0) return FALSE;
			sprintf(subfilename,"%s",subfilename+ind);
			i=i+1;
		}
		ind = myindexof(divideresult[sizeofdivideresult-1],subfilename);
		if (ind<0) return FALSE;
		if (!endwith) {
			sprintf(subfilename,"%s",subfilename+ind);		
			char ret[512]="";
			recuplastchar(subfilename,strlen(divideresult[sizeofdivideresult-1]),ret);
			if (!equalspart(divideresult[sizeofdivideresult-1],ret)) return FALSE;
		}
		return TRUE; 
	}
	else{
		if (startwith && endwith) return TRUE;
		return FALSE;
	}
}

/*int equals(char *filename){
	return !fnmatch(regexp,filename,0);
}*/

void printtime(time_t ftime)
{
	char longstring[80];
	const char *format;
	format = "%3b %e %Y %R   ";
	strftime(longstring, sizeof(longstring), format, localtime(&ftime));
	fputs(longstring, stdout);
}

char filetype(char *filepath){
	struct stat infofile;
	char ret='-';
	stat(filepath, &infofile);
	if (S_ISREG(infofile.st_mode)) ret='-';
	else if (S_ISFIFO(infofile.st_mode)) ret='p';
	else if (S_ISCHR(infofile.st_mode)) ret='c';
	else if (S_ISBLK(infofile.st_mode)) ret='b';
	else if (S_ISDIR(infofile.st_mode)) ret='d';
	else if (S_ISLNK(infofile.st_mode)) ret='l';
	else if (S_ISSOCK(infofile.st_mode)) ret='s';
	return ret;
}

void permission(char *filepath,char* perm){
	struct stat infofile;
	strcpy(perm,"----------");
	int i=0;
	stat(filepath, &infofile);
	mode_t mode = infofile.st_mode;
	if (S_ISDIR(mode)) perm[0]='d';
	if ((mode & S_IRUSR) == S_IRUSR) perm[1]='r';
	if ((mode & S_IWUSR) == S_IWUSR) perm[2]='w';
	if ((mode & S_IXUSR) == S_IXUSR) perm[3]='x';
	if ((mode & S_IRGRP) == S_IRGRP) perm[4]='r';
	if ((mode & S_IWGRP) == S_IWGRP) perm[5]='w';
	if ((mode & S_IXGRP) == S_IXGRP) perm[6]='x';
	if ((mode & S_IROTH) == S_IROTH) perm[7]='r';
	if ((mode & S_IWOTH) == S_IWOTH) perm[8]='w';
	if ((mode & S_IXOTH) == S_IXOTH) perm[9]='x';    	
	
}


void afficher_entete() {

printf("\n");
if (pflag) printf("%10s", "Permission  ");
if (tflag)  printf("%4s","Type  ");
if (sflag) printf("%s","     Size ");
if (dflag) printf ("%22s","Last Use   ");
if (mflag) printf("%17s","Last modification   ");
 printf("File path\n");
}

void afficher_file_nostat(char *filepath) {

if (pflag) printf("%10s", "----------  ");
if (tflag)  printf("%4s  ","!!!!");
if (sflag) printf("%9s   ","!!!!");
if (dflag) printf ("%6s ---%10s"," ","  ");
if (mflag) printf("%6s ---%10s"," ","  ");
printf("%s --- Cannot get file's informations : %s \n",filepath,strerror(errno));
}



void afficher(char *filepath){
	struct stat infofile;
	char absolutepath[512];
	char filetypestring[512];
	stat(filepath, &infofile);


if (pflag) {
		mode_t mode = infofile.st_mode;
		int i=0;
		char permissionfile[11]="";
		permission(filepath,permissionfile);
		printf("%10s  ",permissionfile);
	}	

if (tflag) {
		char t = filetype(filepath);
		if (t=='-') strcpy(filetypestring,"REGU");
		else if (t=='p') strcpy(filetypestring,"FIFO");
		else if (t=='c') strcpy(filetypestring,"MCAR");
		else if (t=='b') strcpy(filetypestring,"BLOCK");
		else if (t=='d') strcpy(filetypestring,"REPR");
		else if (t=='l') strcpy(filetypestring,"LIEN");
		else if (t=='s') strcpy(filetypestring,"SOCK");
		printf("%4s  ",filetypestring);
	}

if (sflag) {
		long size=infofile.st_size;
		if ( (size/1024) <1 ) printf("%6ld oc   ",size);
		else if (( (size/1024)/1024) <1 ) printf("%6.2f Ko   ", (float)size/(float)1024);
		else printf("%6.2f Mo   ",(float)size/(float)1024/(float)1024);
	}	

if (dflag) {
		printtime(infofile.st_atime);
	}
if (mflag) {
		printtime(infofile.st_mtime);
	}
    printf("%s",filepath);
    printf("\n");
	
}  

int parcourir(char * path,int recniv) {
	DIR *dir = NULL;
        struct dirent * entry = NULL;
        struct stat infofile ;
        char filepath[512];
	

        if ( (dir = opendir(path)) == NULL) {
		perror(path);
                return 0;
        }

         while (entry = readdir(dir)) {
        if (path[strlen(path)-1]!='/') strcat(path,"/");
        sprintf(filepath,"%s%s", path, entry -> d_name);
        

            if (stat(filepath, &infofile) == -1) {      
			if (equals(entry->d_name)) {                        
			if (!entete) { 
					afficher_entete();
					entete=1;
				}			
			afficher_file_nostat(filepath);
			//printf("----------<!>.Cannot get file %s 's informations : %s.--------------<!>\n", filepath, strerror(errno));
			}                        
			continue;
                }
           if (S_ISDIR(infofile.st_mode)) {
                    ///Eviter la boucle infinie sur le même dossier
                    if (strcmp(entry -> d_name, ".") == 0 ||strcmp(entry -> d_name, "..") == 0) continue; 

					    // traverser récusivement
					    if (recniv==-1){
						
						parcourir(filepath,recniv);

					    }
					    else if (recniv>0){
						parcourir(filepath,recniv-1);
						
					    }
            }
        
			if (equals(entry->d_name)) {
				if (!entete) { 
					afficher_entete();
					entete=1;
				}
				afficher(filepath); 
				nbres++; 
				}
        }

        closedir(dir);
        return 1;
}

int parse_arguments(int argc, char * argv[], char* path){

	int ind=1;
	char c;
	if (argc >2 && argv[1][0]!='-' ) 	{		
		strcpy(path,argv[1]); 
		ind =2;
		
	}
	else strcpy(path,".");
	strcpy(regexp,argv[argc-1]);	
	char stringop[256];
	while (ind<argc-1) {
		if (argv[ind][0]=='-') {
			sprintf(stringop,"%s",argv[ind]+1);
			if (strlen(stringop)==1) {
				c=stringop[0];	
				switch(c) {
					case 'd':
						dflag=1;
					break;
					case 'm':
						mflag=1;
					break;
					case 's':
						sflag=1;
					break;
					case 't':
						tflag=1;
					break;
					case 'p':
						pflag=1;
					break;
					case 'a':
						dflag=1; mflag=1; sflag=1; tflag=1; pflag=1;
					break;
					default:
					if (atoi(stringop)!=0 ) recflag=atoi(stringop);
					else{
						if (stringop[0]=='0') recflag=0;
						else return -ind-1;
					}
					break;		
				}
				}
				else 
				{
					int i=0;	
					if (atoi(stringop)!=0 ) recflag=atoi(stringop);				
					else for (i=0;i<strlen(stringop);i++)
					{
						if (stringop[i]!='0') return -ind-1;					
					}					
				}
			}
		ind++;	
		}	
	return 0;
}


void afficher_erreur_cmd(int err,char *argv[]){
	printf("%s: invalid option -- '%s'\n",argv[0],argv[-(err+1)]+1);
}

void afficher_resultat(int argc,char *argv[]){
	if (nbres==0) printf("No files were found matching '%s'\n",argv[argc-1]);
	else printf("%d files were found matching '%s'\n",nbres,argv[argc-1]);
}

void afficher_help(){
	printf("\nNAME\n\t\t search - search for a file in a directory and its hiararchy\n\n");
	printf("SYNOPSIS\n\t\t search [Directory_path] [-Options] [File_Name]\n\n");
	printf("Description\n\t\tThe command searchs for a file in a directory. If the directory is omitted, the search begins from the 	current directory.\n\t\tThe searched file name accepts thewildcard characters: ?,*\n\n");
	printf("OPTIONS\n");
	printf("\t\t -N \t N is a positive integer, it represents the depth of the directory hiearchy of the query.\n\t\t\t If N is not 	mentionned, we search in the entire hierarchy\n");
	printf("\t\t -d \t to display the date of last use (consultation or modification) of the file\n");
	printf("\t\t -m \t to display the date of last modification of the file \n");
	printf("\t\t -s \t to display the size of the file\n");
	printf("\t\t -t \t to display the type of the file\n");
	printf("\t\t    \t Types are: REGU: regular file, DIREC: directory, LINK: symbolic link,SOCK: socket\n");
	printf("\t\t    \t FIFO: named pipe, MCAR: character device, BLOCK: block device\n");
	printf("\t\t -p \t to display the protection of the file\n");
	printf("\t\t -a \t to display all the information above\n");

	printf("\n");

	printf("AUTHORS\n\t\tWritten by Belfodil Adnane and Medjkoune Nawel\n\n");
	printf("Reporting Bugs\n\t\t Report command bugs to bn_medjkoune@esi.dz\n\n");
} 




int main (int argc, char * argv[]){ 		
 		char path[512];
		if (argc == 1) {
			afficher_help();		
		}
		else {
			int err = parse_arguments(argc, argv,path);					
			if (err<0) {
				afficher_erreur_cmd(err,argv);	
			}
			else {	
				divide();
									
				int f = parcourir(path,recflag);
				if (f) afficher_resultat(argc,argv);
			}
		}
		
		
	return 0;
        }


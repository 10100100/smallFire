/*
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <linux/tcp.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <arpa/inet.h>
#include <pwd.h>

#include <dirent.h> 	//DIR
#include <ctype.h>  	//isdigit
#include <ncurses.h> 	//ncurses
#include <netdb.h>  	//gethostbyaddr
#include <time.h>  	//ctime

//#include <dos.h>  //sound geht mit beep oder sound nicht

enum{
   TCP_ESTABLISHED = 1,
   TCP_SYN_SENT,
   TCP_SYN_RECV,
   TCP_FIN_WAIT1,
   TCP_FIN_WAIT2,
   TCP_TIME_WAIT,
   TCP_CLOSE,
   TCP_CLOSE_WAIT,
   TCP_LAST_ACK,
   TCP_LISTEN,
   TCP_CLOSING 
};

static const char *tcp_states_map[]={
   [TCP_ESTABLISHED] = "ESTABLISHED",
   [TCP_SYN_SENT] = "SYN-SENT",
   [TCP_SYN_RECV] = "SYN-RECV",
   [TCP_FIN_WAIT1] = "FIN-WAIT-1",
   [TCP_FIN_WAIT2] = "FIN-WAIT-2",
   [TCP_TIME_WAIT] = "TIME-WAIT",
   [TCP_CLOSE] = "CLOSE",
   [TCP_CLOSE_WAIT] = "CLOSE-WAIT",
   [TCP_LAST_ACK] = "LAST-ACK",
   [TCP_LISTEN] = "LISTEN",
   [TCP_CLOSING] = "CLOSING"
};

#define TCPF_ALL 0xFFF
#define SOCKET_BUFFER_SIZE (getpagesize() < 8192L ? getpagesize() : 8192L)

#define PFAD_PROC	"/proc"
#define PFAD_FD_END	"fd"
#define PFAD_EXE_END	"exe"

#define ZWI_NUM_STRINGS 24     
#define ZWI_MAX_LENGTH INET6_ADDRSTRLEN
    
#define PID_SOCKET_MENGE 1500
#define PFADK_MENGE 3000	

//--------------------------------------------------------------------------------------------------------
//include,define,enum ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------
//globale variablen  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------

//holen()
char **ch=NULL;
int iHO=0;
//zeilenmenge
int iRE=0;

//endergebniss inode()
struct pidSocket{
    int nummer;
    int pidP;
    int socket;
    char exEP[20];
    char exEPL[100];
}alle[PID_SOCKET_MENGE]; //			
static int zALLE=0,PIDMenge=0;


//--------------------------------------------------------------------------------------------------------
//globale variablen  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------
//hilfsfunktionen  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


//für liste.txt und verbindung.txt
int safen(char *exEPF, char *exEPLF, int zustF){
time_t jetzt;
time(&jetzt);
struct tm *local=localtime(&jetzt);
int stunden, minuten, sekunden, tag, monat, jahr;


	if(zustF==1){
	FILE *safefile;
	//datei öffnen
	safefile=fopen("verbindung.txt", "a+");
	if(safefile==NULL){
	printw("Konnte Datei \"verbindung.txt\" nicht anlegen!\n");
	return EXIT_FAILURE;
	}

	stunden=local->tm_hour;		
	minuten=local->tm_min;		
	sekunden=local->tm_sec;		
	tag=local->tm_mday;		
	monat=local->tm_mon+1;		
	jahr=local->tm_year+1900;	

//in verbindung.txt schreiben datum,uhrzeit,progname und pfad
fprintf(safefile,"%02d.%02d.%02d %02d:%02d:%02d\t%s\t%s\n",tag,monat,jahr,stunden,
							   minuten,sekunden,exEPF,exEPLF);
fclose(safefile);
}

	if(zustF==2){
	FILE *safefileL;
	safefileL=fopen("liste.txt", "a");
	if(safefileL==NULL){
	printw("Konnte Datei \"liste.txt\" nicht öffnen!\n");
	return EXIT_FAILURE;
	}
		if(strlen(exEPF)<19){
		fprintf(safefileL,"%s\n",exEPF);
		}else{
		printw("Programmname ist zu lang!\n");
		}
	fclose(safefileL);
	}
return EXIT_SUCCESS;
}	//safen()





//filter für holen(nur zahlen)
int ho_filter(const struct dirent *dir){
    	if(isdigit(*dir->d_name)==0)          
    	return EXIT_SUCCESS;
    	return EXIT_FAILURE;
}

//pid verzeichniss holen() aus /proc/?
//fd verzeichnisse holen() aus /proc/1234/fd/?
int holen(char *verz){   	
    
struct dirent **liste;	
int zae=0,len=0;
	
	zae=scandir(verz, &liste, ho_filter, 0);
     	if(zae<0){
     	return EXIT_FAILURE;
        }
        else if(zae!=0){
	//dynamisch Speicher bereitstellen für ch	
        ch = (char**) malloc (sizeof(int*)*zae);  
	if(ch == NULL)
     	return EXIT_FAILURE;
        
		for(iHO=0;iHO<zae;iHO++){
        	len=strlen(liste[iHO]->d_name);
        	len+=1;
        	*(ch+iHO) = (char *) malloc (sizeof(char*)*len);
        	if(*(ch+iHO) == NULL)
     		return EXIT_FAILURE;
       
        	//ch füllen mit den gesuchten Verzeichnissen
    		strncpy(ch[iHO],liste[iHO]->d_name,len-1);
        	ch[iHO][sizeof(len-1]='\0';
		free(liste[iHO]);  
        	} //for(iHO=0
        	        
        free(liste);
	} //else if(zae
return iHO;
} //holen


//--------------------------------------------------------------------------------------------------------
//hilfsfunktionen  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------
//funktionen  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


int inode(){

//variablen
int d=0,e=0,f=0,g=0,end=0;

//alle pid im /proc/??? verzeichniss 
holen(PFAD_PROC);

//ins. menge der verzeichnisse speichern
end=iHO; 
iHO=0;

char pfad[end][16];     //pid  /proc/123.../fd
char pfadEXE[end][17]; //alle /proc/123.../exe
//auf basis von pfad den letzten pfadK weiter bauen um die sockets zu holen    
char pfadK[PFADK_MENGE][23]; //alle /proc/123.../fd/8...    

	//pfade bauen
	for(int c=0;c<end;c++){
	if(ch!=NULL){
	snprintf(pfad[c],15,"%s/%s/%s", PFAD_PROC,ch[c],PFAD_FD_END);
	snprintf(pfadEXE[c],16,"%s/%s/%s", PFAD_PROC,ch[c],PFAD_EXE_END);
	free(ch[c]);
	}			
	}
//leeren für das nächste holen()
free(ch);
memset(&ch,0,sizeof(ch));  

	for(int c=0;c<end;c++){  
	///proc/123/fd/??? holen
	holen(pfad[c]);
	
		for(int b=0;b<iHO;b++){
		if(ch!=NULL && d<PFADK_MENGE){  	
		snprintf(pfadK[d],22,"%s/%s", pfad[c],ch[b]);
		//printw("%d.%s\t", d,pfadK[d]);
		d++;  
		free(ch[b]);		
		} 
		} 	
	//alles leeren für das nächste holen(pfad[k])
	iHO=0;          
	free(ch);
	memset(&ch,0,sizeof(ch));  		
	} //for int c=0
	

//----------------------- verzeichnisse holen ------------------------------------------------------------	
	
//----------------------- pid,socket,prg name in struct.alle ---------------------------------------------


//variablen
//zähler
int aE=0,bE=0;
//für exe namen    
char buffi1[100]={""}; 	
//für socket nummer 	
char buffi2[15]={""};
//zum entfernen   
char nadel[]="socket:[";
//trenner   
char trenner[] = "/";
//nach zahlen suchen
char suchset[]="1234567890";

//l um pid in int umzuwandeln, l1 socket in int umzuwandeln
char *l, *l1;		   
//socket nummer
char *ptrK=buffi2;
char *ptrKT;
//für prog namen
char *ptrPrgN;  
//pid zwichenergebnis
char *ptrE;
char *lE; 

//struct für prognamen und /kompletten/pfad/prognamen
struct pidSocketEXE{	   
    	int pidE;
    	char exE[20];
    	char exEL[100];  
}alleEXE[end]; 	      

                
//programm name in eine struct.alleEXE liste eintragen
for(int c=0;c<end;c++){

	//die symbolischen links der /proc/123../exe auslesen
	ssize_t len=readlink(pfadEXE[c],buffi1,sizeof(buffi1)-1);
     	buffi1[sizeof(buffi1)-1]='\0';
     	if(len!=-1){
	        	    
	//pid rausziehen aus pfadEXE[c] für struct alleEXE	        
	ptrE=strpbrk(pfadEXE[c],suchset); 	        
	//erhalten 1234/fd/8 dann teile nach /
	lE=strtok(ptrE,trenner);
	//erhalten 1234 dann wandle in int um
	aE=strtol(lE,&lE,10);

	 if(bE<end){
	 //pid in die struct.alleEXE schreiben
	 alleEXE[bE].pidE=aE;

	 //strrchr das letzte auftreten von / finden und das was danach kommt
	 //ascii zahl 47 für / geht besser für strrchr() 
	 ptrPrgN=strrchr(buffi1,47);
	 //ergebnis: ptrPrgN=/progname
	 //deswegen / entfernen mit strtok
	 ptrPrgN=strtok(ptrPrgN,trenner);
                
	 //langer exe name /snap/progname/1234/usw/progname
	 strncpy(alleEXE[bE].exEL,buffi1,sizeof(alleEXE[bE].exEL)-1); 
	 alleEXE[bE].exEL[sizeof(alleEXE[bE].exEL)-1]='\0';
	        
	 //kurzer exe progname
	 strncpy(alleEXE[bE].exE,ptrPrgN,sizeof(alleEXE[bE].exE)-1); 
	 alleEXE[bE].exE[sizeof(alleEXE[bE].exE)-1]='\0';	        

	 //jedem durchlauf geleert 
	 memset(&buffi1,0,sizeof(buffi1));
	 memset(&ptrPrgN,0,sizeof(ptrPrgN));
	 //zähler um eins erhöhen	        
	 bE++;
	 }else{
	 printw("Zuviele Programmnamen gleichzeitig geöffnet!\n");
	 } //if(bE<end)
	} //if(len
} //for(int c
	
        
//pfadK durchgehen und mit readlink den symbolischen link auslesen für die sockets 
///proc/1234/fd/8 --> 'socket:[30999]'
//dann endergebnis struct.alle bauen
for(int c=0;c<=d-1;c++){		
       
        ssize_t len1=readlink(pfadK[c],buffi2,sizeof(buffi2)-1);
        buffi2[sizeof(buffi2)-1]='\0';
        if(len1!=-1){
                     
       	 //zeige nur nur symb.links in den socket:[ steht
         if(strstr(buffi2,nadel)!=0){

         //pid rausholen 
         ptrKT=strpbrk(pfadK[c],suchset); 
         l=strtok(ptrKT,trenner); 
         f=strtol(l,&l,10);  //ergebnis: 1487
                
         //jetzt socket:[ entfernen          
	 //wir suchen alle zahlen in socket:[12345
	 ptrK=strpbrk(ptrK,suchset);
         //wir brauchen es als int also umwandeln
         l1=ptrK;
         e=strtol(l1,&l1,10); //ergebnis: 12345
        
        
        	//programm namen holen anhand der pid(f) in alleEXE[c].pidE
        	//alles zusammen fügen und struct.alle bauen
        	for(int c2=0;c2<=bE-1;c2++){		
        
        	 if(f==alleEXE[c2].pidE){
        
		  if(zALLE<PID_SOCKET_MENGE){
	
		  alle[zALLE].nummer=g;alle[zALLE].pidP=f;alle[zALLE].socket=e;
       		  strncpy(alle[zALLE].exEP,alleEXE[c2].exE,sizeof(alle[zALLE].exEP)-1);
        	  alle[zALLE].exEP[sizeof(alle[zALLE].exEP)-1]='\0';
       		  strncpy(alle[zALLE].exEPL,alleEXE[c2].exEL,sizeof(alle[zALLE].exEPL)-1);
       		  alle[zALLE].exEPL[sizeof(alle[zALLE].exEPL)-1]='\0';
       	 
       		  g++; //alle.nummer
		  zALLE++; //struct zähler
		  //prüfen wurde es korrekt geschreiben in alle
		  //printw("z:%d pidP:%d socket:%d exE:%s \n",alle[zALLE].nummer,alle[zALLE].pidP,
		  //					    alle[zALLE].socket,alle[zALLE].exEP);

		  }else{
		  printw("Zuviele Programme gleichzeitig geöffnet! PID Menge "
		         "über: %d\n",PID_SOCKET_MENGE);
		  exit(EXIT_FAILURE);
		  } //if(zALLE      
		 } //if(f==
        	} //for(int c2
    	} //if(strstr  
    	} //if(len1 	
} //for(int c

	zALLE=0;
	PIDMenge=g;
	g=0;
	memset(&pfad,0,sizeof(pfad));
	memset(&pfadK,0,sizeof(pfadK));
	memset(&pfadEXE,0,sizeof(pfadEXE));  
	return EXIT_SUCCESS;				
} //inode()	

//----------------------- pid,socket,prg name in struct.alle ---------------------------------------------




//--------------------------------------------------------------------------------------------------------
//Einstellung und Anfrage --------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------



int send_diag_msg(int sockfd){
//variablen
struct msghdr msg;
struct nlmsghdr nlh;
struct inet_diag_req_v2 conn_req;
struct sockaddr_nl sa;
struct iovec iov[2];
int retval=0;

//bzero ersatz alle structs auf null 0 setzen
memset(&sa,0,sizeof(sa));
memset(&conn_req,0,sizeof(conn_req));
memset(&nlh,0,sizeof(nlh));
memset(&msg,0,sizeof(msg));
 
	    //Socket Einstellungen für die Abfrage per sendmsg()
    	sa.nl_family=AF_NETLINK;
    	conn_req.sdiag_family=AF_INET;
    	conn_req.sdiag_protocol=IPPROTO_TCP;
    	conn_req.idiag_states=TCPF_ALL & 
        ~((1<<TCP_SYN_RECV) | (1<<TCP_CLOSE) );
    	conn_req.idiag_ext |= (1<<(INET_DIAG_INFO - 1));
        
      //Erstellen und initialisieren des Nachrichtenheaders
    	nlh.nlmsg_len=NLMSG_LENGTH(sizeof(conn_req));
    	nlh.nlmsg_flags=NLM_F_DUMP | NLM_F_REQUEST;
    	nlh.nlmsg_type=SOCK_DIAG_BY_FAMILY;
    	
    	//Erstellen des Nachrichtenvektors und Nachrichtenstruktur
    	iov[0].iov_base=(void*) &nlh;
    	iov[0].iov_len=sizeof(nlh);
    	iov[1].iov_base=(void*) &conn_req;
    	iov[1].iov_len=sizeof(conn_req);
    	msg.msg_name=(void*) &sa;
    	msg.msg_namelen=sizeof(sa);
    	msg.msg_iov=iov;
        msg.msg_iovlen=2;
   
    	//Abfrage von zusammengebauter msg
    	retval=sendmsg(sockfd, &msg, 0);

    	return retval;
} //send_diag_msg()



//--------------------------------------------------------------------------------------------------------
//Antwort und Ausgabe ------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------



int loop (int sock, int zu, int erlaub, int key, int breit, int hoehe){

//variablen
//recive
int numbytes = 0, rtalen = 0;
struct nlmsghdr *nlh;
uint8_t recv_buf[SOCKET_BUFFER_SIZE];
struct inet_diag_msg *diag_msg;
    
//user    
struct passwd *uid_info = NULL;
    
//attribute
struct rtattr *attr;
struct tcp_info *tcpi;
    
//prog schliessen     
char zwi[ZWI_NUM_STRINGS][ZWI_MAX_LENGTH] = {""};
char davor[54],danach[9];  

//ip verbindungen    
char local_addr_buf[INET6_ADDRSTRLEN];
char remote_addr_buf[INET6_ADDRSTRLEN]; 
memset(local_addr_buf, 0, sizeof(local_addr_buf));
memset(remote_addr_buf, 0, sizeof(remote_addr_buf));

//für den programmnamen usw. die liste struct.alle füllen
inode();

//verbindungszaehler
int r=0;
//fensterhöhe
hoehe=hoehe/2;
int nhoch=1;
//struct alle
int jop=0;
//vorzeichen
int vorz=0;
//mvprintw zeile zählen
int zeiR=0;
//dns
struct hostent *gethost;
struct hostent result;
struct in_addr ip;
int h_error;
char buff[1024];     
//dns
   
//filter normale localhost ip 
char filterIP[]="0.0.0.0";

//filterPRG
int findestr=1;    

//liste.txt
char bufferLiZ[20];
int zeilen=0;
char bufferLi[20];
int Li=0;
FILE *ListeP;

//löschen
char buf3[3];
int zeileLoe=0;


//----------------------- erlaubte programme -------------------------------------------------------------

    //datei öffnen um die menge der prognamen in zeilen einzutragen
    ListeP=fopen("liste.txt", "a+");
    if(NULL==ListeP){
    printw("Konnte Datei \"liste.txt\" nicht öffnen oder anlegen!\n");
    return EXIT_FAILURE;
    }
    //falls liste.txt leer füge geschlossen ein
    if(getc(ListeP)==-1){
    fprintf(ListeP,"%s\n","geschlossen");
    }
    while(fgets(bufferLiZ, 20, ListeP) != NULL){
    zeilen++;
    }
    
    char filterPRG[zeilen][20];
    fseek(ListeP, 0L, SEEK_SET);

    //filterPRG mit den erlauben Programmnamen füllen
    while(fgets(bufferLi, 19, ListeP) != NULL){
      	strncpy(filterPRG[Li], bufferLi, sizeof(filterPRG[Li])-1);
      	filterPRG[Li][sizeof(filterPRG[Li])-1]='\0';
      	//printw("%s\n",filterPRG[Li]);	
	Li++;
    } 
    fclose(ListeP);


    //erlaubte programmnamen anzeigen
    if(key==108){
    clear();
    mvprintw(3,0,"Erlaubte Programme:\n");
      for(int c=0;c<zeilen;c++){
      printw("%d.%s",c,filterPRG[c]);
      napms(200);            
      }
    }
    
    //einen programmnamen entfernen
    if(key==114){
    clear();
    	    printw("\n\nBitte die Nummer eingeben:\n");
    	    printw("Die neue Liste:\n");                 
            fgets(buf3, 3, stdin);
            sscanf(buf3, "%d", &zeileLoe);  
        
    ListeP = fopen("liste.txt", "w");
    	if(ListeP == NULL){
	printw("Unable to create file.\n");
        exit(EXIT_FAILURE);
        }  	
		for(int c=0;c<Li;c++){
		if(c==zeileLoe){
		strncpy(filterPRG[c],"",sizeof(filterPRG[c])-1);
                filterPRG[c][sizeof(filterPRG[c])-1]='\0';
		}		
		fputs(filterPRG[c],ListeP);				
		printw("%s",filterPRG[c]);
		}				
    fclose(ListeP);	
    }

//----------------------- erlaubte programme -------------------------------------------------------------
//----------------------- recive socket mit struct.alle vergleichen und anzeigen -------------------------

    while(1){
      
        //empfange
        numbytes=recv(sock, recv_buf, sizeof(recv_buf), 0);        
        //prüfe ob etwas empfangen wurde       
        if(numbytes<0)
        fprintf(stdout, "RECV ERROR\n");
                
        //packe alles in struct nlh
        nlh=(struct nlmsghdr*) recv_buf;
            
           //auslesen von nlh
	   while(NLMSG_OK(nlh,numbytes)){
           if(nlh->nlmsg_type==NLMSG_DONE)
           //werte unter sleep 2 steigt die cpu last bei 0 sind es schon 50%
           sleep(2);
            
           if(nlh->nlmsg_type==NLMSG_ERROR){
           fprintf(stderr, "Error in netlink message\n");
           break;
           } 
           
           //packe die daten in diag_msg
           diag_msg=(struct inet_diag_msg*) NLMSG_DATA(nlh);               
           //attribute dazu braucht man die länge
           rtalen=nlh->nlmsg_len-NLMSG_LENGTH(sizeof(*diag_msg));
           //user
           uid_info=getpwuid(diag_msg->idiag_uid);
            
             //local_addr_buf und remote_addr_buf füllen
             //inet_ntop wandelt ip4 und ip6 addressen von binär in text bzw. string um
             if(diag_msg->idiag_family==AF_INET){
             inet_ntop(AF_INET, (struct in_addr*) &(diag_msg->id.idiag_src), 
             local_addr_buf, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, (struct in_addr*) &(diag_msg->id.idiag_dst), 
             remote_addr_buf, INET_ADDRSTRLEN);
    	     }else if(diag_msg->idiag_family == AF_INET6){
             inet_ntop(AF_INET6, (struct in_addr6*) &(diag_msg->id.idiag_src),
             local_addr_buf, INET6_ADDRSTRLEN);
             inet_ntop(AF_INET6, (struct in_addr6*) &(diag_msg->id.idiag_dst),
             remote_addr_buf, INET6_ADDRSTRLEN);
    	     }else{
	     return EXIT_SUCCESS;
    	     }
    	     
    	     //prüfen ob was drin ist		
             if(local_addr_buf[0]==0 || remote_addr_buf[0]==0){
             fprintf(stderr, "Bekomme keine der benötigten Verbindungsinformationen\n");
    	     }else{

    	        //Ausgabe von local_addr_buf und remote_addr_buf
                //Filter 0.0.0.0
                if(strcmp(filterIP,remote_addr_buf)!=0){
                
		for(size_t c=0;c<PIDMenge;c++){
		if(alle[c].socket==diag_msg->idiag_inode)
		jop=c; 
		} 
					
		//wenn das überwachte programm geschlossen wird  
		if(diag_msg->idiag_inode==0){
                strncpy(alle[jop].exEP,"geschlossen",sizeof(alle[jop].exEP)-1);
                alle[jop].exEP[sizeof(alle[jop].exEP)-1]='\0';
                alle[jop].pidP=0;
                }
              
		//filter für erlaubte prognamen
		for(int c=0;c<zeilen;c++){
		if(strstr(filterPRG[c],alle[jop].exEP)!=0)
		findestr=0;              
		} 
                
		if(findestr==1 && key!=108){    
                
                //für das vorzeichen
                vorz=62; //ASCII für >
		
                if(diag_msg->idiag_inode==0)
                vorz=60; //ASCII für <
                                                      
                r++;                
                attrset(COLOR_PAIR(1));

		//Fensterhöhe prüfen
		if(r<hoehe){
		//Fensterbreite unter 90 dann lasse die src IP weg                
		if(breit>90){
                mvprintw(zeiR,0,"%c%d user:%s(uid:%u) src:%s:%d ", 
                vorz,
                r,
                uid_info==NULL ? "Not found" : uid_info->pw_name,
                diag_msg->idiag_uid,
                local_addr_buf,
                ntohs(diag_msg->id.idiag_sport));
		}else{
                mvprintw(zeiR,0,"%c%d user:%s(uid:%u) srcPort:%d ", 
                vorz,
                r,
                uid_info==NULL ? "Not found" : uid_info->pw_name,
                diag_msg->idiag_uid,
                ntohs(diag_msg->id.idiag_sport));
		}
		
		//den rest in fettschrift und gedimmt anzeigen
                //um deutlicher zu machen was wichtig ist
                attron(A_BOLD | A_DIM);
                
		//dns anzeigen
		if(key==100 && inet_aton(remote_addr_buf, &ip)!=0){
			//ip auflösen um dns anzuzueigen
			gethostbyaddr_r((const void *)&ip,sizeof(ip),AF_INET,&result,buff,
		                 	sizeof(buff),&gethost,&h_error); 
		
			if(h_error==0){    		
       			printw("dst:%s:%d prg:%s\n",
                	gethost->h_name,
                	ntohs(diag_msg->id.idiag_dport),
                	alle[jop].exEP);          
                	}else{
                	//z.B. bei nxdomain steht für non-existent Domain
                	printw("DNS Fehler:%d\n",h_error);
                	}	              
                        
                //IP anzeigen
		}else{   
                printw("dst:%s:%d pid:%d prg:%s\n",
                remote_addr_buf, 
                ntohs(diag_msg->id.idiag_dport),
                alle[jop].pidP,
                alle[jop].exEP);

		//schreibe das programm in eine log datei zu sicherheit und späterer prüfung
		safen(alle[jop].exEP,alle[jop].exEPL,1);
                } //if(key==100
                
                //Fensterhöhe prüfen
		}else{clear();printw("Bei dieser Fentergrösse können maximal %d gleichzeitig geöffnete " 
		"Verbindungen angezeigt werden!\nDie Anzeige wartet nun solange bis es weniger "
		"sind oder Sie das Fenster vergrössern.\n",hoehe-1);nhoch=0;}
		                
                //schrift einstellung ausstellen
                attroff(A_BOLD | A_DIM);
                attroff(COLOR_PAIR(1));   
                //zeile hochzählen für mvprintw  
		zeiR++;
		zeiR++;
   		
                //schliessen der unerwünschten verbindung 
                strncpy(zwi[r],remote_addr_buf,sizeof(zwi[r])-1);
                zwi[r][sizeof(zwi[r])-1]='\0';
                
                if(zu!=0 && r==zu){
                //string für iptables bauen
                strncpy(davor,"iptables -A OUTPUT -p tcp -d ",30-1);
                davor[30-1]='\0';
                strncpy(danach," -j DROP",9-1);
                danach[9-1]='\0';                
                strncat(davor,zwi[zu],strlen(zwi[zu]));
                strncat(davor,danach,strlen(danach));
                printw("%s\n",davor);
                system(davor);
                printw("Die Verbindungnr.%d wurde geschlossen\n",zu);              
                } //if(zu
                                
                //erlauben von programmen 
                if(erlaub!=0 && r==erlaub){
                safen(alle[jop].exEP,alle[jop].exEPL,2);
                printw("Die Verbindungnr.%d mit dem Anwendungsnamen %s wurde erlaubt\n",
                						 erlaub,alle[jop].exEP);
                }                    
		} //if(findestr 
		findestr=1;                       
              } //if(strcmp(filterIP
              } //else if(local_addr


              //attribute auslesen
              if(rtalen>0){
              attr=(struct rtattr*) (diag_msg+1);

              while(RTA_OK(attr,rtalen)){
              if(attr->rta_type==INET_DIAG_INFO){                
              tcpi=(struct tcp_info*) RTA_DATA(attr);

	      //filterliste der erlaubten programmnamen
	      for(int c=0;c<zeilen;c++){
	      if(strstr(filterPRG[c],alle[jop].exEP)!=0)
	      findestr=0;
	      }
		
	        //Attriubute anzeigen	
		if(findestr==1 && strcmp(filterIP,remote_addr_buf)!=0 && key!=108 && nhoch==1){		
                printw("   State:%s Recv:%ldkB Send:%ldkB RTT:%gms "
                "Recv.RTT:%gms Snd_cwnd:%u/%u\n",
                tcp_states_map[tcpi->tcpi_state],
                (long) tcpi->tcpi_bytes_received/1024,
                (long) tcpi->tcpi_bytes_sent/1024,
                (double) tcpi->tcpi_rtt/1000, 
                (double) tcpi->tcpi_rcv_rtt/1000, 
                tcpi->tcpi_unacked,
                tcpi->tcpi_snd_cwnd);                    
                } //if(findestr
		findestr=1;         
              } //if(attr->rta_type
              attr=RTA_NEXT(attr,rtalen); 
              } //while(RTA_OK
    	      } //if(rtalen
                
        	//sound
        	if(r>=1)
		iRE=1;
		else
		iRE=0;
              	             	
           nlh=NLMSG_NEXT(nlh,numbytes);       	      
	   } //while(NLMSG_OK
    } //while endlos

//filterPRG leeren
free(filterPRG);		
//----------------------- recive socket mit struct.alle vergleichen und anzeigen -------------------------

} //loop()



//--------------------------------------------------------------------------------------------------------
//funktionen  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------
//aufruf +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------



int main(){

//variablen
//socket
int sock=-1;
//ncurses 
int key=0;
//fensterbreite/hoehe
int breit=0;
int hoehe=0;    
//verbindung verbieten/erlauben
char buf[3],buf2[3];
int zu=0,erlaub=0,v=0,v2=0,durch=0;

//sternchen
srand(79);   //zum initialisieren von rand
int zufall, zufall2;    
int snoop=0;
int maxrow, maxcol;
//sternchen
//sound
char soundp[33];	

    //prüfen ob programm mit root rechten ausgeführt 
    if(geteuid()!=0){			
    printf("\n\x1b[32mDie Anwendung mit sudo starten damit man auf alle Programme zugreifen "
           "kann!\x1b[0m\n\n");
    sleep(10);
    }
    
    //sock öffnen für INET_DIAG abfrage  
    if((sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_INET_DIAG)) < 0)
     		return EXIT_FAILURE;

//ncurses
//um tastatur eingaben des users abzufangen für programm optionen während es läuft
initscr();  //start curses mode
cbreak();   //line buffering aus
noecho();   //echo aus
scrollok(stdscr, TRUE); //Damit am Ende der letzten Zeile beim Einfügen einer neuen Zeile weiter 
			//nach unten gescrollt wird                             
//sternchen
scroll(stdscr); //scrolling
//sternchen
    
nodelay(stdscr, TRUE); //verhindern bei der Verwendung der Funktion getch() anhält
   			  
    //textfarbe da \x1b[31m nicht bei printw geht
    if(has_colors() == FALSE) printw("Kein farbiger Text moeglich!"); 
    else start_color(); 			//farbe starten
    init_pair(1, COLOR_GREEN, COLOR_BLACK);     //farbe auswählen
//ncurses
        

    //tasten eingabe des benutzer abfangen während das programm läuft zur steuerung
    //113 ist q für quit bzw. beenden
    while(key != 113){          
          
          //INET_DIAG abfrage anlegen und einstellen
          send_diag_msg(sock);
          
          //breite und länge ausmessen
     	  getmaxyx(stdscr, maxrow, maxcol);
          breit=maxcol;
          hoehe=maxrow;
          
 	  //loop aufrufen und damit alles starten und snoop für sternchen speichern
	  snoop=loop(sock,zu,erlaub,key,breit,hoehe);  
	  	           
          //unerwünschte verbindung schliessen
          zu=0;
          //erlauben von programmen               
          erlaub=0;

	 //sound abspielen
	 //durch und iRE damit nur einmal der sound kommt wenn eine neue verbindung aufgeht
	 if(iRE==1 && durch==0){
	 strncpy(soundp,"aplay -q labor/sounds/space3.wav",33-1);
	 soundp[33-1]='\0';
	 system(soundp);
	 durch++;
	 }
	 //durch wieder auf 0 für die nächste verbindung die aufgeht
	 if(iRE==0)
	 durch=0;

          //ncurses
          key=getch();
          
          //erhöht die geschwindigkeit - getch wird für 10 milisekunden immer wieder ausgesetzt 
          if(key == ERR) 
          napms(10);
                
          //verbindung verbieten    	      
          if(key == 's'){
          printw("Bitte die Nummer eingeben:\n");
          fgets(buf, 3, stdin);
          sscanf(buf, "%d", &v);  
            if(geteuid()==0){			          
            zu=v;
            printw("Die Verbindung %d wird verboten...\n",v);            
            }else{ 
            printw("Die Anwendung mit sudo starten um Verbindungen verbieten zu können!\n");          
            }  
          }
	  
	  //erlauben von programmen                     	  
    	  if(key == 'e'){
        printw("Bitte die Nummer eingeben:\n");
        fgets(buf2, 3, stdin);
        sscanf(buf2, "%d", &v2);  
        erlaub=v2; 
        //printw("Die Verbindung %d wurde dauerhaft erlaubt...\n",v2);            
        }
             	  
    
    	   //sternchen anzeigen
    	   if(snoop==0){
    	   scrl(1);			
    		  for(int i=0;i<5;i++){    
     		  zufall=rand () % maxcol; 
     		  mvaddch(maxrow-1, zufall, '0'); 
     		  zufall2=rand () % maxcol;      
     		  mvaddch(maxrow-2, zufall2, '1');      
    		  }
    	   } 
    } //while(key

endwin();
close(sock);
return EXIT_SUCCESS;				
} //main


//--------------------------------------------------------------------------------------------------------
//aufruf +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//--------------------------------------------------------------------------------------------------------

#include "dht.h"
#define key_space 100000                //pow(2,30)
using namespace std;



int  serv_id;
int num_of_serv;
int portno;

unsigned key_range,key_range_begin,key_range_end;
string line1,line2;
string fnf="NO CONTENTS";

ifstream ifile1,ifile2;



void init()
{

	ifile1.open("serv_id",ios::in);
	ifile2.open("num_of_serv",ios::in);
	
	ofstream ofile;
	ofile.open("load_rate");
	ofile<<"0";
	ofile.close();
	
	
	if(ifile2.is_open())
	{
		getline(ifile2,line2);
		num_of_serv=atoi(line2.c_str());
		cout<<"number of servers = "<<num_of_serv<<endl;
		key_range=key_space/num_of_serv;
		ifile2.close();
	}
	else
	{
		perror("\n ERROR OBTAINING NUMBER OF SERVERS\n");
	}
	
	if(ifile1.is_open())
	{
		getline(ifile1,line1);
		serv_id=atoi(line1.c_str());
		key_range_begin=(serv_id-1)*(unsigned long)key_range;
		key_range_end=(serv_id)*key_range;
		ifile1.close();
		cout<<"key range begin = "<<key_range_begin<<endl;
		cout<<"key range end = "<<key_range_end<<endl;
	}
	else
	{
		perror("\n ERROR OBTAINING SERVER ID\n");
	}
}



string propagate_read(string c_domain,char cbuffer[])
{
			int c_sockfd;
					
			cout<<"next server "<<c_domain<<endl;
			
			struct sockaddr_in c_serv_addr;
			struct hostent *c_server;
			//char rbuffer[buf_size];
			
			c_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			c_server = gethostbyname(c_domain.c_str());
			
			if (c_server == NULL)
			 {
				cerr<<"ERROR, no such host\n";
				exit(0);
			 }
			
             bzero((char *) &c_serv_addr, sizeof(c_serv_addr));
             
             c_serv_addr.sin_family = AF_INET;
                                                                   //intializing the host structure i.e. struct hostent server
			 bcopy((char *)c_server->h_addr, (char *)&c_serv_addr.sin_addr.s_addr,c_server->h_length);
			 
			 c_serv_addr.sin_port = htons(portno);
			 
			 if (connect(c_sockfd,(struct sockaddr *) &c_serv_addr,sizeof(c_serv_addr)) < 0) 
			 error("ERROR connecting");
         
			 c_serv_addr.sin_port = htons(portno);
			 
			 write(c_sockfd,cbuffer,buf_size);
			
			string ret_contents=read_buffer(c_sockfd);
			close(c_sockfd);
			cout<<"\n PRINTING RETURN VALUE FROM INTERMEDIATE SERVER \n";
			cout<<ret_contents<<endl; 
			
			return ret_contents;
			
}

void client_connection(int sock)
{
    int n;
    MD5 md5;
    string command;
    string resource;
    string resource_hash;
    char cbuffer[buf_size],buffer[buf_size];                     //stores the command
    string content;
    int buffer_sz=0;
    unsigned long hash_val;
    
	int num_of_servers=0; 
	
	
    bzero((char *)cbuffer,buf_size);

    n = read(sock,(char*)cbuffer,buf_size);
    bcopy(cbuffer,buffer,buf_size);
    
    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }
    cout<<"incoming data from client = "<<buffer<<endl;
    
		char *str=(char *)strpbrk(buffer," ");         //removing the command bit i.e. 0 or 1
		resource=strtok(str," ");                           //getting the key
		//content==strpbrk(str+1," ");                        //getting the number of characters to be read
		cout<<"resource name = "<<resource<<endl;
		resource_hash=(char *)md5.digestString((char *)resource.c_str());
		
		hash_val=hex_to_dec(resource_hash)%(unsigned long)key_space;         //%(int)key_space add if required for limited key-space
		cout<<"hash_val = "<<hash_val<<endl;
		int req_serv=(hash_val/key_range)+1;
		
    if(buffer[0]=='0')  //read command
    {
		
		if((hash_val>=key_range_begin)&&(hash_val<key_range_end))
		{
			
			//main server
			gettimeofday(&end, NULL);
			seconds=end.tv_sec  - start.tv_sec;
			cout<<"seconds = "<<seconds<<endl;
			
			if(seconds > 30)                                //calculating load-rate every 5 seconds
			{
				gettimeofday(&start, NULL);                //resetting start
				str_loadrate=read_line_file("load_rate");
				str_maxthreshold=read_line_file("max_load_rate");
				
				cur_loadrate=atoi((char *)str_loadrate.c_str());
				cout<<"Current load rate"<<cur_loadrate<<endl;
				max_threshold=atoi((char *)str_maxthreshold.c_str());
				
				if(cur_loadrate > max_threshold)
				balance_load=true;                         //allow load balancing if current load-rate is greater than max-threshold load-rate
				else
				balance_load=false;                        //dis-allowing load balancing if current load is lesser than max threshold load
			}
			if(balance_load==false)
			{
				string fname=convertInt(hash_val);
				cout<<"resource exists in this node = "<<fname<<endl;
			   
			   str_loadrate=read_line_file("load_rate");
			   cur_loadrate=atoi((char *)str_loadrate.c_str());
			   
			   cur_loadrate++;
			   string new_threshold=convertInt(cur_loadrate);
			   ofstream ofile;
			   ofile.open("load_rate");
			   ofile<<new_threshold;
			   ofile.close();
			   
			   cout<<"filename = "<<fname<<endl;
			   string contents = read_file(fname,"0");
			   
			   if(contents.empty())
			   {
				   write_buffer(sock,fnf);         //FILE NOT FOUND OR DOES NOT EXIST
			   }
			   else
			   {
					cout<<"\n---------------HELLO WORLD ---------\n"<<endl;
					write_buffer(sock,contents);
			   }
			   cout<<"\n END OF WRITE"<<endl;
			   
			   close(sock);
			}
			else
			{
				//sister server providing data
				
				string ret_contents;
				string dest_domain=read_line_file("secondary_server");
				ret_contents=propagate_read(dest_domain,cbuffer);
			
			
				write_buffer(sock,ret_contents);
			
				close(sock);
			}
		}
		else     //route to other server nodes and bring back data via a client connection
		{
			string dest_domain;
			if(hash_val >= key_range_begin)
			dest_domain=read_line_file("ahead_server");
			if(hash_val < key_range_end)
			dest_domain=read_line_file("behind_server");
			
			string ret_contents;
			ret_contents=propagate_read(dest_domain,cbuffer);
			
			
			write_buffer(sock,ret_contents);
			
			close(sock);
		}
	}
	else if(buffer[0]=='1')
	{
		//cout<<"In write mode"<<endl;
		
		string w_contents = read_buffer(sock);
		cout<<"In server contents are :"<<w_contents<<endl;
		if((hash_val>=key_range_begin)&&(hash_val<key_range_end))
		{
			string fname=convertInt(hash_val);
			cout<<"resource exists in this node ="<<fname<<endl;
			cout<<"Contents to be written in local server are "<<w_contents<<endl;
			
			write_file(fname,w_contents);         //check for error conditions as well
			
			//writing to sister node
			
			string sis_domain=read_line_file("secondary_server");
			int sis_sockfd;
			
			struct sockaddr_in sis_serv_addr;
			struct hostent *sis_server;
			
			sis_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			
			if (sis_sockfd < 0) 
			error("ERROR opening socket");
			
			sis_server = gethostbyname(sis_domain.c_str());
			
			if(sis_server == NULL)
			 {
				cerr<<"ERROR, no such host\n";
				exit(0);
			 }
			
					
             bzero((char *) &sis_serv_addr, sizeof(sis_serv_addr));
             
             sis_serv_addr.sin_family = AF_INET;
                                                                   //intializing the host structure i.e. struct hostent server
			 bcopy((char *)sis_server->h_addr, (char *)&sis_serv_addr.sin_addr.s_addr,sis_server->h_length);
			 
			 sis_serv_addr.sin_port = htons(portno);
			 
			 if (connect(sis_sockfd,(struct sockaddr *) &sis_serv_addr,sizeof(sis_serv_addr)) < 0) 
			 error("ERROR connecting");
         
			 sis_serv_addr.sin_port = htons(portno);
			 
			 n = write(sis_sockfd,cbuffer,buf_size);
			 
			 write_buffer(sis_sockfd,w_contents);           	  //propagating write request
			 
			 
			 
			 
			 cout<<"End of write propagate"<<endl;
			
		}
		else
		{
			//propagate
			int c_sockfd;
			
			string c_domain;
			if(hash_val >= key_range_begin)
			c_domain=read_line_file("ahead_server");
			if(hash_val < key_range_end)
			c_domain=read_line_file("behind_server");
			
			struct sockaddr_in c_serv_addr;
			struct hostent *c_server;
			char rbuffer[buf_size];
			
			c_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			c_server = gethostbyname(c_domain.c_str());
			
			if (c_server == NULL)
			 {
				cerr<<"ERROR, no such host\n";
				exit(0);
			 }
			
             bzero((char *) &c_serv_addr, sizeof(c_serv_addr));
             
             c_serv_addr.sin_family = AF_INET;
                                                                   //intializing the host structure i.e. struct hostent server
			 bcopy((char *)c_server->h_addr, (char *)&c_serv_addr.sin_addr.s_addr,c_server->h_length);
			 
			 c_serv_addr.sin_port = htons(portno);
			 
			 if (connect(c_sockfd,(struct sockaddr *) &c_serv_addr,sizeof(c_serv_addr)) < 0) 
			 error("ERROR connecting");
         
			 c_serv_addr.sin_port = htons(portno);
			 
			 n = write(c_sockfd,cbuffer,buf_size);
			 
			 write_buffer(c_sockfd,w_contents);           //propagating write request
			 
			 
			 
			 
			 cout<<"End of write propagate"<<endl;
		}
		
	}
	else
	{
		perror("\nINVALID COMMAND\n");
	}
	
    if (n < 0) 
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

int main( int argc, char *argv[] )
{
    int sockfd, client_sockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;
    
    gettimeofday(&start, NULL);
    
	init();
	
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(sockfd,5);
    clilen = (socklen_t)sizeof(cli_addr);
    while (1) 
    {
        client_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        if (client_sockfd < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }
        /* Create child process */
        int pid = fork();
        if (pid < 0)
        {
            perror("ERROR on fork");
	    exit(1);
        }
        if (pid == 0)  
        {
            /* This is the child process */
            close(sockfd);
            client_connection(client_sockfd);
            exit(0);
        }
        else
        {
            close(client_sockfd);
        }
    } /* end of while */
}

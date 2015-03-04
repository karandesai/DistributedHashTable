#include "dht.h"
using namespace std;


string gate_domain;
string resource;

string buffer;

int choice;
string contents;

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    
    /*if (argc < 3) {
       cerr<<"usage %s hostname port\n"<<argv[0]<<endl;
       exit(0);
    }*/
    
    portno = atoi(argv[1]);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);               //create a socket file into which if data is written it is transmitted over the socket connection
    
    if (sockfd < 0) 
        error("ERROR opening socket");
        
        
    cout<<"Enter the gateway domain"<<endl;
    cin>>gate_domain;
      
    server = gethostbyname(gate_domain.c_str());
    
    if (server == NULL) {
       cerr<<"ERROR, no such host\n";
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
                                                                   //intializing the host structure i.e. struct hostent server
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
         
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
        
        
    cout<<"Please enter resource key: "<<endl;
    cin>>resource;
    
    cout<<"Menu : \n0 : Read Resource\n1 : Write Resource"<<endl;
    cin>>choice;
    switch(choice)
    {
		case 0:
		buffer = "0 "+resource;
		break;
		
		case 1:
		buffer = "1 "+resource;
		break;
		
	}
    
    //bzero((char *)buffer,256);
    
    n = write(sockfd,buffer.c_str(),buffer.length());
    
    if (n < 0) 
         error("ERROR writing to socket");
    
    if(choice == 1)
    {
		cout<<"Enter the value to be written :";       //dont leave white space between : and "
		char dummy;
		cin>>dummy;                 //removing new line
		string dummy_str;
		stringstream ss;
		ss<<dummy;
		ss>>dummy_str;
		
		getline(cin,contents);     //reading entire line containing white spaces
		contents=dummy_str+contents;
		//cout<<"dummy = "<<dummy_str<<endl;
		cout<<"In client contents are "<<contents<<endl;
		write_buffer(sockfd,contents);
	}
    if(choice == 0)
    {
		cout<<"\nPRINTING RETURN VALUE FROM SERVER\n"<<endl;
		string ret_val=read_buffer(sockfd);
		cout<<ret_val<<endl;
	}
	
	close(sockfd);	
    return 0;
}

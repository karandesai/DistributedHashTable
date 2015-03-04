#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <math.h>
#include <netdb.h> 
#include "md5.h"
#define buf_size 256
using namespace std;

map<char,int> hex_dec;
string conn_err="CONNECTION ERROR";
bool balance_load=false;
string str_loadrate;
string str_maxthreshold;
int cur_loadrate;
int max_threshold;
int seconds;

struct timeval start, end;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

unsigned long hex_to_dec(string str)
{
	int i=0;
	unsigned long res=0;
	
	hex_dec['0']=0;
	hex_dec['1']=1;
	hex_dec['2']=2;
	hex_dec['3']=3;
	hex_dec['4']=4;
	hex_dec['5']=5;
	hex_dec['6']=6;
	hex_dec['7']=7;
	hex_dec['8']=8;
	hex_dec['9']=9;
	hex_dec['a']=10;
	hex_dec['b']=11;
	hex_dec['c']=12;
	hex_dec['d']=13;
	hex_dec['e']=14;
	hex_dec['f']=15;
	
	for(i=0;i<32;i++)
	{
			res=res+hex_dec[str[i]]*(pow(16,(31-i)));
	}
	return res;
}

string convertInt(long number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

void write_buffer(int serv_sock, string cont)
{
		   char wbuffer[buf_size];
		   long buf_len = cont.length();
		   string blen=convertInt(buf_len);
		   long i=0;
		   
		   int flag=0;
		   int prev_flag;
		   
		   getsockopt(serv_sock,IPPROTO_TCP,TCP_NODELAY,(void *)&prev_flag,(socklen_t *)sizeof(int));
		   setsockopt(serv_sock,IPPROTO_TCP,TCP_NODELAY,(void *)&flag,(socklen_t)sizeof(int));
		   
		   write(serv_sock,blen.c_str(),blen.length());
		   
		   setsockopt(serv_sock,IPPROTO_TCP,TCP_NODELAY,(void *)&prev_flag,(socklen_t)sizeof(int));
		   sleep(5);
		   cout<<"\nfile content length is "<<blen<<endl;
		   int write_len;
		   cout<<"\n BEGiNNING WRITE TO SOCKET\n";
		   while(i<buf_len)
		   {
			   string str=cont.substr(i,buf_size);
			   i=i+buf_size;
			   memcpy(&wbuffer,str.c_str(),buf_size);
			   cout<<wbuffer<<endl;
			   cout<<"\n--------------\n";
			   write_len=write(serv_sock,wbuffer,buf_size);
			   sleep(2);
			   cout<<write_len<<endl;
			   bzero((char *)wbuffer,buf_size);
		   }
}


string read_buffer(int r_sockfd)
{
	
		
		char r_buffer[buf_size];
		string ret_contents="";
		
		int n = read(r_sockfd,r_buffer,buf_size);             //getting the length of incoming data
		cout<<"r_buffer = "<<r_buffer<<endl;
		 if (n < 0) 
         error("ERROR reading from socket");
		
		
			long read_size=atol(r_buffer);
			cout<<"\n"<<read_size<<endl;
			long read_count=0;
			
			while(read_count<read_size)
			{
				bzero((char *) &r_buffer,buf_size);
				read(r_sockfd,r_buffer,buf_size);
				ret_contents=ret_contents+r_buffer;
				read_count=read_count+buf_size;
			}
		
		return ret_contents;

}

int write_file(string filename,string text)
{
  ofstream myfile;
  myfile.open(filename.c_str());                             //add ios::binary later
     if(myfile.is_open())
     {  myfile<<text.c_str();
     
      }

    else
     {
       return 0; // RETURN 0 if FAILED
     }

  myfile.close();
  return 1;

}


string read_file (string filename,string bytes)
{

  ifstream myfile;
  int read_bytes;
  //string bytes="35";
  read_bytes=atoi(bytes.c_str());
  char* buffer;
  //string filename="Design";

 //if 0 is given it means read whole file
 myfile.open(filename.c_str());                                     //add ios::binary later

 if(read_bytes==0)
   
  {
  if(myfile.is_open())
        {
           myfile.seekg (0, ios::end); 
           int length = myfile.tellg(); //traverse to beginning
           buffer = new char[length];
           read_bytes=length;
           myfile.seekg (0, ios::beg);  //go to beginning
          
        }
        else return NULL;
       
  }

else   //if readbytes is not 0
    {
    buffer = new char [read_bytes]; //else size obtained from parameter
    }


if(myfile.is_open())
    {
    myfile.read(buffer,read_bytes);
    }

else
    {
    return NULL;
    }

	string output(buffer);
	myfile.close();
	return output; // Return final string
	
}


string read_line_file(const char * fname)
{
	ifstream ifile;
	ifile.open(fname,ios::in);
	
	string line;
	if(ifile.is_open())
	{
		getline(ifile,line);
		ifile.close();
		return line;
	}
	else
	{
		return conn_err;
	}
}

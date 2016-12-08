#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>   
#include<sys/ioctl.h>
#include<unistd.h>  
#include<iostream>
#include<sstream>
#include<fstream>
#include<errno.h>
#include<stdlib.h>
#include<errno.h>
using namespace std;

int send_image(int socket, int sequence)
{
    cout<<"ENTERING SEND"<<endl;
    std::string path = "/home/andreas/images/image_";
	std::string jpg =".jpg";
    stringstream sstream ;
    sstream << path << sequence << jpg;
    std::string full_path = sstream.str();
    
    FILE *picture;
	int size, read_size, stat, packet_index;
	char send_buffer[10240], read_buffer[256];
	packet_index = 1;
    cout << full_path.c_str()<<endl;
	picture = fopen(full_path.c_str(), "r");
	printf("Getting Picture Size\n");   

	if(picture == NULL) {
        perror(full_path.c_str());
		printf("Error %d \n", errno); } 
    cout << "1" << endl;
	fseek(picture, 0, SEEK_END);
    cout << "2" << endl;
	size = ftell(picture);
    cout << "3" << endl;
	fseek(picture, 0, SEEK_SET);
	printf("Total Picture size: %i\n",size);

	//Send Picture Size
	printf("Sending Picture Size\n");
	write(socket, (void *)&size, sizeof(int));

	//Send Picture as Byte Array
	printf("Sending Picture as Byte Array\n");

	do 
	{ //Read while we get errors that are due to signals.
        cout<<"Reading"<< endl;
		stat=read(socket, &read_buffer , 255);
		printf("Bytes read: %i\n",stat);
	} while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %c\n", read_buffer);

    while(!feof(picture)) 
    {
        //while(packet_index = 1){
        //Read from the file into our send buffer
        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

        //Send data through our socket 
        do
        {
            stat = write(socket, send_buffer, read_size);  
        }while (stat < 0);

        printf("Packet Number: %i\n",packet_index);
        printf("Packet Size Sent: %i\n",read_size);     
        printf(" \n");
        printf(" \n");


        packet_index++;  

        //Zero out our send buffer
        bzero(send_buffer, sizeof(send_buffer));
    }
    //fflush(stdout);
    cout<<"EXITING"<<endl;
    return 0;
}
    
int main(int argc , char *argv[])
{
    int socket_desc;
    struct sockaddr_in server;
    char *parray;

    //Create socket
    /*
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    cout<<"Here"<<endl;
    if (socket_desc == -1) 
    {
        printf("Could not create socket");
    }

    memset(&server,0,sizeof(server));
    server.sin_addr.s_addr = inet_addr("192.168.2.2");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8889 );
    /*
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , 
    sizeof(server)) < 0) 
    {
        cout<<strerror(errno);
        close(socket_desc);
        puts("Connect Error");
        return 1;
    }

    puts("Connected\n");
    fflush(stdout);
    for(int i=1; i<11; i++)
    {
        send_image(socket_desc, i);
    }
    
    close(socket_desc);
    */
    int sequence=0;
    while (true)
    {
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        cout<<"Here"<<endl;
        if (socket_desc == -1) 
        {
            printf("Could not create socket");
        }

        memset(&server,0,sizeof(server));
        server.sin_addr.s_addr = inet_addr("192.168.2.2");
        server.sin_family = AF_INET;
        server.sin_port = htons( 8889 );
        if (connect(socket_desc , (struct sockaddr *)&server , 
        sizeof(server)) < 0) 
        {
            cout<<strerror(errno);
            close(socket_desc);
            puts("Connect Error");
            return 1;
        }

        puts("Connected\n");
        fflush(stdout);
        if (sequence<10)
        {
            sequence++;
            send_image(socket_desc, sequence);
            
        }
        else
        {
            return 1;
        }
        close(socket_desc);
    }
    
    close(socket_desc);
    
    return 0;
}

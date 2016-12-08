#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>   
#include<unistd.h>  
#include<iostream>
#include<fstream>
#include<sstream>
#include<errno.h>
#include<string>
#include<stdlib.h>
#include<unistd.h>
using namespace std;

int receive_image(int socket, int sequence)
{ // Start function 
	cout<<"ENTERING RECIEVE"<<endl;
	std::string path = "image_";
	std::string jpg =".jpg";
    stringstream sstream ;
    sstream << path << sequence << jpg << endl;
    std::string full_path = sstream.str();
    
	int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, 
    packet_index =1,stat;

	char imagearray[10241],verify = '1';
	FILE *image;

	//Find the size of the image
	do
	{
        printf("Stat : %d\n", stat);
		stat = read(socket, &size, sizeof(int));
	}while(stat<0);

	printf("Packet received.\n");
	printf("Packet size: %i\n",stat);
	printf("Image size: %i\n",size);
	printf(" \n");

	char buffer[] = "Got it";

	//Send our verification signal
	do
	{
		stat = write(socket, &buffer, sizeof(int));
	}while(stat<0);

	printf("Reply sent\n");
	printf(" \n");

	image = fopen(full_path.c_str(), "w");

	if( image == NULL)
	{
		printf("Error has occurred. Image file could not be opened\n");
		return -1; 
	}

	//Loop while we have not received the entire file yet


	int need_exit = 0;
	struct timeval timeout = {10,0};

	fd_set fds;
	int buffer_fd, buffer_out;

	while(recv_size < size) 
	{
	//while(packet_index < 2){
        cout << "receiving file" <<endl;
		FD_ZERO(&fds);
		FD_SET(socket,&fds);

		buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

		if (buffer_fd < 0)
		   printf("error: bad file descriptor set.\n");

		if (buffer_fd == 0)
		   printf("error: buffer read timeout expired.\n");

		if (buffer_fd > 0)
		{
			do
			{
			   read_size = read(socket,imagearray, 10241);
			}while(read_size <0);

			printf("Packet number received: %i\n",packet_index);
			printf("Packet size: %i\n",read_size);


			//Write the currently read data into our image file
			write_size = fwrite(imagearray,1,read_size, image);
			printf("Written image size: %i\n",write_size); 

			if(read_size !=write_size) 
			{
				printf("error in read write\n");    
			}

			recv_size += read_size;
			packet_index++;
			printf("Total received image size: %i\n",recv_size);
			printf(" \n");
			printf(" \n");
		}

	}
    
    cout << "EXITING RECEIVE"<<endl;
    return 0;
}

int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c, read_size,buffer = 0;
    struct sockaddr_in server , client;
    char *readin;
	int sequence = 0;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(1235);

      //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) 
     < 0)
    {
       puts("bind failed");
       return 1;
    }

    puts("bind done");

    //Listen
    //listen(socket_desc , 3);

    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    //c = sizeof(struct sockaddr_in);
    while (true)
    {
        listen(socket_desc , 3);
        puts("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);
        if((new_socket = accept(socket_desc, (struct sockaddr *)&client,
        (socklen_t*)&c)))
        {
            puts("Connection accepted");
        }

        fflush(stdout);

        if (new_socket<0)
        {
          perror("Accept Failed");
          return 1;
        }
        if(sequence < 10)
        {
            sequence++;
            receive_image(new_socket, sequence);
        }
        else
        {
            return 1;
        }
        usleep(500);
    }
    /*
    if((new_socket = accept(socket_desc, (struct sockaddr *)&client,
    (socklen_t*)&c)))
    {
		puts("Connection accepted");
    }

    fflush(stdout);

    if (new_socket<0)
    {
      perror("Accept Failed");
      return 1;
    }
    
	for(int i=1;i<11;i++)
	{   
		receive_image(new_socket, i);
        usleep(500);
	}
    */
    close(socket_desc);
    return 0;
}

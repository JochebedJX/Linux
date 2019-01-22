#include "server.h"

Server::Server(int port):_port(port){}

void Server::Init_Server(){

  _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(_sockfd < 0){
    std::cerr << " socket error" << std::endl; 
    return;
  }

  
  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_port = htons(_port);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(_sockfd, (struct sockaddr*)&local, sizeof(local)) < 0){
    std::cerr << "bind error !" << std::endl;
    return;
  }


}


void Server::recv_data(std::string &out_string){

  char buf[SIZE];
  struct sockaddr_in peer;
  socklen_t len;
  ssize_t s = recvfrom(_sockfd, buf, SIZE, 0, (struct sockaddr*)&peer, &len);
  if(s > 0){
    buf[s] = 0;
    out_string = buf;

  }else {
    //
  }
}

void Server::send_data(const std::string &in_string, const struct sockaddr_in &peer, const socklen_t &len){

  sendto(_sockfd, in_string.c_str(), in_string.size(), 0, (struct sockaddr*)&peer, len); 

}

void Server::broadcast(){

}

Server::~Server(){
  close(_sockfd);
}



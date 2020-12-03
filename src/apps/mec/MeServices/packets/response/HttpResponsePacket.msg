// @author Alessandro Noferi


packet HTTPResponsePacket
{
    @customize(true);
  	string status;
  	string connection;
  	string contentType;
  	string body;
  	string payload;

}
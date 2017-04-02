/*
 * =====================================================================================
 *
 *       Filename:  Handler.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年09月22日 16时53分45秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "handler.h"

#include <string>

#include "Log.h"
#include "util_glue.h"
#include "client.h"

#include "PacketDef.pb.h"
#include "protocol_c2s.pb.h"
#include "protocol_s2c.pb.h"     

#define BIG_PACKET_HEADER

struct ServerPktHeader      
{                           
#ifdef BIG_PACKET_HEADER    
	uint32_t size;            
	uint32_t cmd;             
	char content[0];
#else                       
	uint16_t size;            
	uint16_t cmd;             
	char content[0];
#endif                      
} __attribute__ ((packed)); 

using namespace TgsProtocol;

bool HandlerInit(uint8_t process)
{
	//to do implementation
//	IME_LOG("handler init %u", thread);
	return true;
}

bool HandlerFinally(uint8_t process)
{
	//to do implementation
//	IME_LOG("handler finally %u", thread);
	return true;
}

template<typename TPacket>      
void SendPacketProtocol( TPacket* packet, ClientNetwork &client)
{
	char *pkg;   
	ServerPktHeader hdr;

	if(packet == NULL) 
		return;        

	hdr.size = packet->ByteSize() + sizeof(ServerPktHeader);
	hdr.cmd = packet->cmdid();

	pkg = (char *)malloc(hdr.size);
	memcpy(pkg, &hdr, sizeof(hdr)); 
	if (false == packet->SerializeToArray((void *)(pkg + sizeof(hdr)), packet->ByteSize()))
	{
		assert(0);
	}                                                                            
	if (!packet->IsInitialized())         
	{
		IME_ERROR("send message of type %s error content %s", packet->GetTypeName().c_str(), packet->InitializationErrorString().c_str());
		free(pkg);
		return;
	}

	if (client.SendSync(pkg, hdr.size))  
		IME_ERROR("send data fail");
	free(pkg);
}

int GetProtocolID(ClientNetwork &client, ServerPktHeader *phdr)
{
	if (client.RecvSync((char *)phdr, sizeof(ServerPktHeader)))
	{
		IME_ERROR("recv header data fail");
		return -1;
	}
	return phdr->cmd;
}

void DiscardPacket(ClientNetwork &client, const ServerPktHeader &hdr)
{
	char *pbuffer;
	pbuffer = (char *)malloc(hdr.size - sizeof(hdr));
	if (client.RecvSync(pbuffer, hdr.size - sizeof(hdr)))
	{
		IME_ERROR("recv content data fail");
		return;
	}
	free(pbuffer);
}

template<typename TPacket>      
void RecvPacketProtocol( TPacket* packet, ClientNetwork &client, const ServerPktHeader &hdr)
{
	char *pbuffer;
	pbuffer = (char *)malloc(hdr.size - sizeof(hdr));
	if (client.RecvSync(pbuffer, hdr.size - sizeof(hdr)))
	{
		IME_ERROR("recv content data fail");
		return;
	}

	if (packet->ParseFromArray(pbuffer, hdr.size - sizeof(hdr)) == false)
	{   
		IME_ERROR("parse data fail type %s error content %s", packet->GetTypeName().c_str(), packet->InitializationErrorString().c_str());
		free(pbuffer);
		return;
	}   
	if (!packet->IsInitialized())
		IME_ERROR("recv message of type %s error content %s", packet->GetTypeName().c_str(), packet->InitializationErrorString().c_str());
	free(pbuffer);
}

void HandlerWork()
{
	//to do implementation
	IME_LOG("handler work");

	ClientNetwork client;
	if (client.Connect("172.16.1.158", "6001", 5))
	{
		IME_ERROR("connect fail");
		return;
	}
	
	pid_t id = getpid();
	IME_LOG("process id %u handle", id);
	// login
	CLIENT2LOBBY_AuthReq req;
	req.set_charid(id);
	req.set_devicename("linux_test");
	req.set_devicetype("linux");
	req.set_deviceuid("none");
	req.set_platform(0);
	req.set_language(0);

	SendPacketProtocol(&req, client);
	ServerPktHeader hdr;
	//int i = 0;
	while (true)
	{
		GetProtocolID(client, &hdr);
		IME_LOG("get header cmd %u size %u", hdr.cmd, hdr.size);
		if (hdr.cmd == LOBBY2CLIENT_LOGIN_FAILED)
		{
			LOBBY2CLIENT_LoginFailed pkg_ret;
			RecvPacketProtocol(&pkg_ret, client, hdr);
			std::string strPacket = pkg_ret.Utf8DebugString();
			IME_LOG("get login ret %s", strPacket.c_str());
			if (pkg_ret.error() != L2C_SUCCESS)
			{
				IME_ERROR("login fail");
				exit(1);
			}
			if (pkg_ret.bcreatechar())
			{
				CLIENT2LOBBY_CreateCharReq create_char;
				create_char.set_gender(1);
				create_char.set_charname(Uint64ToString(id * 10));
				SendPacketProtocol(&create_char, client);
			}
			else
			{
				break;
			}
		}
		else if (hdr.cmd == LOBBY2CLIENT_CREATE_CHAR_FAILED)
		{
			LOBBY2CLIENT_CreateCharFailed pkg_ret;
			RecvPacketProtocol(&pkg_ret, client, hdr);
			std::string strPacket = pkg_ret.Utf8DebugString();
			IME_LOG("create char ret %s", strPacket.c_str());
			if (pkg_ret.error() != L2C_SUCCESS)
			{
				IME_ERROR("create char fail %d", pkg_ret.error());
				exit(1);
			}
			break;
		}
		else
		{
			DiscardPacket(client, hdr);
		}
		//if (i++ > 10)
		//	break;
	}

	IME_LOG("user %u have logan", id);

	int number = 5;	
	CLIENT2LOBBY_GetOpenBoxInfo_Req box_req;
	SendPacketProtocol(&box_req, client);

	for (int i = 0; i < number; i++)
	{
		GetProtocolID(client, &hdr);
		if (hdr.cmd == LOBBY2CLIENT_GRTOPENBOXINFO_RET)
		{
			LOBBY2CLIENT_GetOpenBoxInfo_Ret pkg_ret;
			RecvPacketProtocol(&pkg_ret, client, hdr);
			std::string strPacket = pkg_ret.Utf8DebugString();
			IME_LOG("get openbox info ret %s", strPacket.c_str());

			sleep(3);
			CLIENT2LOBBY_GetOpenBoxInfo_Req box;
			SendPacketProtocol(&box, client);
		}
		else if (hdr.cmd == LOBBY2CLIENT_HEART_NTF)
		{
			LOBBY2CLIENTHeartNtf pkg_ret;
			RecvPacketProtocol(&pkg_ret, client, hdr);
			std::string strPacket = pkg_ret.Utf8DebugString();
			IME_LOG("get heart ntf ret %s", strPacket.c_str());

			CLIENT2LOBBYHeartRep pkg_req;
			pkg_req.set_beattime(pkg_ret.beattime());
			SendPacketProtocol(&pkg_req, client);
		}
		else
		{
			DiscardPacket(client, hdr);
		}
	}
}

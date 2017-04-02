/*
 * =====================================================================================
 *
 *       Filename:  Client.cpp
 *
 *    Description:  client network
 *
 *        Version:  1.0
 *        Created:  03/20/2017 05:38:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "client.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include "Log.h"
#include "SocketBuffer.h"
#include "util_glue.h"

ClientNetwork::ClientNetwork()
{
	input_buffer_ = new CSocketBuffer();
	output_buffer_ = new CSocketBuffer();
	input_buffer_->Init();
	output_buffer_->Init();
	return;
}

ClientNetwork::~ClientNetwork()
{
	delete input_buffer_;
	delete output_buffer_;
	close(fd_);
	return;
}

int ClientNetwork::Connect(const char *host, const char *serv, int timeout)
{
	int n;
	struct addrinfo	hints, *res, *ressave;

	// get ip and port from host and serv
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
		IME_ERROR("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
		return -1;
	}
	ressave = res;

	fd_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd_ < 0)
	{
		IME_ERROR("socket create fail for %s:%s %s", host, serv, strerror(errno));
		return -1;
	}

	int flags, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = Fcntl(fd_, F_GETFL, 0);
	Fcntl(fd_, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	if ( (n = connect(fd_, res->ai_addr, res->ai_addrlen)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			close(fd_);
			IME_ERROR("connect fail for %s:%s %s", host, serv, strerror(errno));
			return(-1);
		}
	}

	FD_ZERO(&rset);
	FD_SET(fd_, &rset);
	wset = rset;
	tval.tv_sec = timeout;
	tval.tv_usec = 0;

	n = select(fd_+1, &rset, &wset, NULL, timeout ? &tval : NULL);
	if ( n <= 0) {
		close(fd_);		/* timeout */
		if (n == 0)
		{
			errno = ETIMEDOUT;
		}
		IME_ERROR("connect fail timeout or fail %s:%s %s", host, serv, strerror(errno));
		return(-1);
	}

	if (FD_ISSET(fd_, &rset) || FD_ISSET(fd_, &wset)) {
		len = sizeof(error);
		if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return(-1);			/* Solaris pending error */
	} 
	else
	{
		IME_ERROR("select error: fd_ not set");
		return -1;
	}

	if (error) {
		close(fd_);		/* just in case */
		IME_ERROR("connect fail for %s:%s reason %s", host, serv, strerror(errno));
		return -1;
	}

	freeaddrinfo(ressave);

	return 0;
}

int ClientNetwork::SendSync(const char *pbuffer, int len)
{
	int n;
	int already = 0;
	if (len < 0)
	{
		IME_ERROR("len is unvalid %d", len);
		return -1;
	}
	while (already < len)
	{
		n = send(fd_, pbuffer + already, len - already, 0);
		if (-1 == n)
		{
			if (EAGAIN == errno)
			{
				usleep(500);
				continue;
			}
			else if (EINTR == errno)
			{
				continue;
			}
			IME_ERROR("send fail reason %s", strerror(errno));
			return -1;
		}
		already += n;
	}
	return 0;
}

int ClientNetwork::RecvSync(char *pBuffer, int len)
{
	int n;
	int already = 0;
	if (len < 0)
	{
		IME_ERROR("len is unvalid %d", len);
		return -1;
	}
	if (len == 0)
		return 0;
	while (already < len)
	{
		n = recv(fd_, pBuffer + already, len - already, 0);	
		if (-1 == n)
		{
			if (EAGAIN == errno)
			{
				usleep(500);
				continue;
			}
			else if (EINTR == errno)
			{
				continue;
			}
			IME_ERROR("send fail reason %s errno %u", strerror(errno), errno);
			return -1;
		}
		already += n;
	}
	return 0;
}

int ClientNetwork::Send(const char *pbuffer, int len)
{
	output_buffer_->Append(pbuffer, len);
	if (output_buffer_->Sends(fd_) < 0)
	{
		IME_ERROR("send data fail");
		return -1;
	}
	return 0;
}

int ClientNetwork::Run()
{
	int n = input_buffer_->Reads(fd_);
	if (n)
	{
		IME_ERROR("read data fail");
		return -1;
	}
	if (output_buffer_->GetSize() > 0)
	{
		n = output_buffer_->Sends(fd_);
		if (n)
		{
			IME_ERROR("send data fail");
			return -1;
		}
	}
	return 0;
}

int ClientNetwork::GetRecvBufferLen()
{
	return input_buffer_->GetSize();
}

int ClientNetwork::GetRecvBuffer(char *pbuffer, int len)
{
	return input_buffer_->BufferRead(pbuffer, len);
}

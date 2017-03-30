#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#include "SocketBuffer.h"
#include "util_glue.h"
#include "Log.h"

CSocketBuffer::CSocketBuffer()
{
	m_stcBuffer.buffer = NULL;
	m_stcBuffer.orig_buffer = NULL;
	m_stcBuffer.totallen = 0;
	m_stcBuffer.misalign = 0;
	m_stcBuffer.off = 0;
	m_nBufferInitLen = BUFFER_INIT_LEN;
}

CSocketBuffer::~CSocketBuffer()
{
	//free the memory
	if (m_stcBuffer.orig_buffer != NULL)
		free(m_stcBuffer.orig_buffer);
}

void CSocketBuffer::Clear()
{

	m_stcBuffer.misalign = 0;
	m_stcBuffer.off = 0;
	m_stcBuffer.buffer = m_stcBuffer.orig_buffer;
}

void CSocketBuffer::Init(char* szBuf, size_t uLen)
{
	m_stcBuffer.buffer = szBuf;
	m_stcBuffer.orig_buffer = szBuf;
	m_stcBuffer.totallen = uLen;
}

void CSocketBuffer::Init()
{
	//use config buffer len
	m_nBufferInitLen = CSocketBuffer::BUFFER_INIT_LEN;
	//char *pTmp = new char [m_nBufferInitLen];
	char *pTmp = (char*)malloc(m_nBufferInitLen);
	Init(pTmp,m_nBufferInitLen);
}

int CSocketBuffer::Append(const void * pBuffer, size_t uLen)
{
	if (!uLen)
		return 0;

	size_t need = m_stcBuffer.misalign + m_stcBuffer.off + uLen;

	if (m_stcBuffer.totallen < need)
	{
		if (m_stcBuffer.misalign >= uLen)
		{
			Align();
		}
		else if (Expand(uLen) == -1)
			return (-1);
	}

	memcpy(m_stcBuffer.buffer + m_stcBuffer.off, pBuffer, uLen);
	m_stcBuffer.off += uLen;

	return 0;
}

int CSocketBuffer::Appends(const CSocketBuffer * pBuffer)
{
	if (pBuffer->GetSize() > 0)
	{
		return Append(pBuffer->GetBuffer(), pBuffer->GetSize());
	}
	else
	{
		return 0;
	}
}

void CSocketBuffer::Align()
{
	memmove(m_stcBuffer.orig_buffer, m_stcBuffer.buffer, m_stcBuffer.off);
	m_stcBuffer.buffer = m_stcBuffer.orig_buffer;
	m_stcBuffer.misalign = 0;
}

int CSocketBuffer::Expand(size_t uLen)
{
	size_t need = m_stcBuffer.misalign + m_stcBuffer.off + uLen;

	/* If we can fit all the data, then we don't have to do anything */
	if (m_stcBuffer.totallen >= need)
		return (0);

	/*
	 * If the misalignment fulfills our data needs, we just force an
	 * alignment to happen.  Afterwards, we have enough space.
	 */
	if (m_stcBuffer.misalign >= uLen) {
		Align();
	} else {
		void *newbuf;
		size_t length = m_stcBuffer.totallen;

		if (length < 256)
			length = 256;
		while (length < need)
			length <<= 1;

		if (m_stcBuffer.orig_buffer != m_stcBuffer.buffer)
			Align();
		if ((newbuf = realloc(m_stcBuffer.buffer, length)) == NULL)
			return (-1);

		m_stcBuffer.orig_buffer = m_stcBuffer.buffer = (char *)newbuf;
		m_stcBuffer.totallen = length;
	}

	return 0;
}

void CSocketBuffer::Erase(size_t uLen)
{
	if (uLen >= m_stcBuffer.off)
	{
		m_stcBuffer.off = 0;
		m_stcBuffer.buffer = m_stcBuffer.orig_buffer;
		m_stcBuffer.misalign = 0;
		return;
	}

	m_stcBuffer.buffer += uLen;
	m_stcBuffer.misalign += uLen;

	m_stcBuffer.off -= uLen;
}

void CSocketBuffer::Reset()
{
	Erase(GetSize());
}

const char * CSocketBuffer::GetBuffer() const
{
	return m_stcBuffer.buffer;
}

const char * CSocketBuffer::GetBuffer(size_t & size) const
{
	size = m_stcBuffer.off;
	return m_stcBuffer.buffer;
}

size_t CSocketBuffer::GetSize() const
{
	return m_stcBuffer.off;
}

int CSocketBuffer::BufferRead(char * szBuffer, size_t uLen)
{
	size_t nread = uLen;
	if (nread >= m_stcBuffer.off)
		nread = m_stcBuffer.off;

	memcpy(szBuffer, m_stcBuffer.buffer, nread);
	Erase(nread);

	return nread;
}


int CSocketBuffer::Reads(int fd, int nHowmuch)
{
	char *p;
	int n = BUFFER_MAX_READ;
	char buf[1024];

#if defined(FIONREAD)
#ifdef WIN32
	long lng = n;
	if (ioctlsocket(fd, FIONREAD, &lng) == -1 || (n=lng) == 0) {
#else
	if (ioctl(fd, FIONREAD, &n) == -1 || n == 0) {
#endif
		n = BUFFER_MAX_READ;
	} else if (n > BUFFER_MAX_READ && n > howmuch) {
		/*
		 * It's possible that a lot of data is available for
		 * reading.  We do not want to exhaust resources
		 * before the reader has a chance to do something
		 * about it.  If the reader does not tell us how much
		 * data we should read, we artifically limit it.
		 */
		if (n > m_stcBuffer.totallen << 2)
			n = m_stcBuffer.totallen << 2;
		if (n < BUFFER_MAX_READ)
			n = BUFFER_MAX_READ;
	}
#endif	
	if (nHowmuch < 0 || nHowmuch > n)
		nHowmuch = n;

	
	size_t need = m_stcBuffer.misalign + m_stcBuffer.off + nHowmuch;
	if (m_stcBuffer.totallen < need)
	{
		if (m_stcBuffer.misalign >= (size_t)nHowmuch)
		{
			Align();
		}
		else if (Expand(nHowmuch) == -1)
		{
			Reset();
			//realloc ʧ�ܣ�
			//need log
			return -1;
		}
	}

	/* We can append new data at this point */
	p = m_stcBuffer.buffer + m_stcBuffer.off;

	n = recv(fd,p,nHowmuch,0);	

	if (n == EAGAIN || n == EINTR)
		return 0;
	else if (n < 0)
	{
		strerror_r(errno, buf, 1024);
		IME_ERROR("send fail reason %s", buf);
		return -1;
	}
	if (n == 0)
		return 0;

	m_stcBuffer.off += n;

	return n;
}

int CSocketBuffer::Sends(int fd)
{
	int n;
	char buf[1024];

	n = send(fd,m_stcBuffer.buffer,m_stcBuffer.off,0);

	if (n == EAGAIN || n == EINTR)
		return 0;
	else if (n < 0)
	{
		strerror_r(errno, buf, 1024);
		IME_ERROR("send fail reason %s", buf);
		return -1;
	}
	if (n == 0)
		return 0;
	Erase(n);

	return n;
}


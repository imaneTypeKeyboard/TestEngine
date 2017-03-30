

#ifndef _SOCKET_BUFFER_H_
#define _SOCKET_BUFFER_H_

#include <stdio.h>

class CSocketBuffer
{
public:
	CSocketBuffer();
	~CSocketBuffer();

	typedef struct _STC_BUFFER
	{
		char *buffer; // 缓存地址
		char *orig_buffer; // 容器地址

		size_t misalign; // 容器已被读取的空间长度
		size_t totallen; // 总长度  toallen = misalign + off + 剩余缓存空间长度
		size_t off; // 缓存的长度
	} STC_BUFFER;
	static const int STC_BUFFER_SIZE = sizeof(STC_BUFFER);

	//每次read的字节数
	static const int BUFFER_MAX_READ = 4096;
	//默认初始化大小
	static const int BUFFER_INIT_LEN = 4096 * 2;


	//初始化
	//外部new个空间
	void Init(char* szBuf, size_t uLen);
	//默认 推荐采用
	void Init();

	//append一个buffer class
	int Appends(const CSocketBuffer * pPwBuffer);

	int Append(const void * pBuffer, size_t uLen);

	//删除buff中uLen个字节
	void Erase(size_t uLen);

	//重置buff
	void Reset();

	//获取buff指针
	const char * GetBuffer() const;

	//获取buff指针
	const char * GetBuffer(size_t & size) const;

	//获取buff的size
	size_t GetSize() const;

	//从buff中读 返回读到的字节数 参数为user的缓冲区和size
	int BufferRead(char * szBuffer, size_t uLen);

	//读取数据buff，nHowmuch表示一次读到字节数。-1 表示采用默认 4096字节
	int Reads(int fd, int nHowmuch = -1);

	//向curl 发送buff  返回发送成功到字节数
	int Sends(int fd);

	void Clear();

private:
	void Align();
	int Expand(size_t uLen);


private:
	STC_BUFFER m_stcBuffer;
	int m_nBufferInitLen;
};


#endif


/*
 * =====================================================================================
 *
 *       Filename:  util_glue.cpp
 *
 *    Description:  杂类函数
 *
 *        Version:  1.0
 *        Created:  03/23/2017 11:48:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "util_glue.h"

#include "Log.h"

int Fcntl(int fd, int cmd, int arg)
{
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
	{
		IME_ERROR("fcntl error");
		return -1;
	}
	return(n);
}

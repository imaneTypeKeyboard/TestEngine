/*
 * =====================================================================================
 *
 *       Filename:  Client.h
 *
 *    Description:  client network
 *
 *        Version:  1.0
 *        Created:  03/20/2017 07:42:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <string>

class CSocketBuffer;

/*
 * =====================================================================================
 *        Class:  ClientNetwork
 *  Description:   
 * =====================================================================================
 */
class ClientNetwork
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		ClientNetwork();                             /* constructor */
		~ClientNetwork();

		/* ====================  ACCESSORS     ======================================= */

		/* ====================  MUTATORS      ======================================= */

		/* ====================  OPERATORS     ======================================= */
		int Connect(const char *host, int port, int timeout);
		int SendSync(char *pbuffer, int len);
		int RecvSync(char *pbuffer, int len);

		void Send(char *pbuffer, int len);
		void Run();


	protected:
		/* ====================  METHODS       ======================================= */

		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  METHODS       ======================================= */

		/* ====================  DATA MEMBERS  ======================================= */
		int fd_;
		
		CSocketBuffer *input_buffer_;
		CSocketBuffer *output_buffer_;

}; /* -----  end of class ClientNetwork  ----- */


#endif

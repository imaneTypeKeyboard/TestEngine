/*
 * =====================================================================================
 *
 *       Filename:  Handler.h
 *
 *    Description:  logical operations module.
 *        Version:  1.0
 *        Created:  2013年09月22日 15时05分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef _HANDLER_H_
#define _HANDLER_H_
#include <stdint.h>

/* 
 * ===  FUNCTION  ======================================================================
 *  Name:  HandlerInit
 *  Description:  world process(logical process) or main process first call this function
 *  proc: 0 main process 1 world process 
 *  return: true is success, or is failure
 * =====================================================================================
 */
bool HandlerInit(uint8_t process);


/* 
 * ===  FUNCTION  ======================================================================
 *  Name:  HandlerFinally
 *  Description:  world process(logical process) or main final call this function
 *  proc: 0 main process 1 world process 
 *  return: true is success, or is failure
 * =====================================================================================
 */
bool HandlerFinally(uint8_t process);


/* 
 * ===  FUNCTION  ======================================================================
 *  Name:  HandlerWork
 *  Description:  world process(logical process) funciton. Notice every timer is 
 *  independent. For example now time is 0. When time arrives 3, this function will be 
 *  called twice(1s and 3s). But para is 1 or 3.
 * =====================================================================================
 */
void HandlerWork();

#endif

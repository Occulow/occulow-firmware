/*
 * logging.h
 *
 * Created: 4/28/2017 3:34:56 PM
 *  Author: tsun
 */


#ifndef LOGGING_H_
#define LOGGING_H_


#define DEBUG_PRINT 1

#ifdef DEBUG_PRINT
	#define LOG(fmt, args...) printf(fmt, ##args);
	#define LOG_LINE(fmt, args...) printf(fmt "\r\n", ##args);
#else
	#define LOG(fmt, args...) while (0);
	#define LOG_LINE(fmt, args...) while (0);
#endif


#endif /* LOGGING_H_ */
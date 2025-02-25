#ifndef __HTTP_CONN_H__
#define __HTTP_CONN_H__

#include <stdio.h>

//负责控制 server和client之间的HTTP连接；
//	也负责解析client发来的HTTP请求报文；
//	并让server针对其发来的请求进行正确的返回

class HTTPConn
{
//常量
public:
	static const int READ_BUFFER_SIZE = 2048;
	enum METHOD	//HTTP请求的行为
	{
		GET = 0,
		POST,
		HEAD,
		PUT,
		DELETE,
		TRACE,
		OPTIONS,
		CONNECT,
		PATH
	};
	enum CHECK_STATE
	{
		CHECK_STATE_REQUESTLINE = 0,
		CHECK_STATE_HEADER,
		CHECK_STATE_CONTENT
	};
	enum HTTP_CODE	//状态码
	{
		NO_REQUEST,
		GET_REQUEST,
		BAD_REQUEST,
		NO_RESOURCE,
		FORBIDDEN_REQUEST,
		FILE_REQUEST,
		INTERNAL_ERROR,
		CLOSED_CONNECTION
	};
	enum LINE_STATUS	//行读取状态
	{
		LINE_OK = 0,
		LINE_BAD,
		LINE_OPEN
	};

public:
	HTTPConn() {}
	~HTTPConn() {}

//成员函数
public:	
	void init();
	void close_conn();
	void process();	//报文行为处理总封装

private:
	
	/* 状态机解析报文用 */
	LINE_STATUS parse_line();
	HTTP_CODE parse_requestline(char *text);
	HTTP_CODE parse_headers(char *text);
	HTTP_CODE parse_content(char *text);

	/* 具体处理报文读写逻辑 */
	HTTP_CODE process_read();
	bool process_write(HTTP_CODE ret);


//成员变量
public:

private:
	char m_read_buf[READ_BUFFER_SIZE];
	long m_read_idx;
	long m_checked_idx;
	int m_start_line;
	CHECK_STATE m_check_state;

	char *m_url;	//从请求行中读到的URL
	METHOD m_method;	//从请求行读取到的报文请求方式（GET啊之类的）
	char *m_version;	//当前HTTP的版本
	long m_content_length;
	char *m_host;

	int m_start_line;
};

#endif

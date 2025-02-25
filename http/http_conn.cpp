#include "http_conn.h"











void HTTPConn::init()
{
	m_check_state = CHECK_STATE_REQUESTLINE;
	m_method = GET;	//默认GET吗？
	m_url = nullptr;
	m_version = nullptr;
	m_host = nullptr;
	m_content_length = 0;
	m_check_idx = 0;
	m_read_idx = 0;

	memset(m_read_buf, '\0', READ_BUFFER_SIZE);
}

void HTTPConn::close_conn()
{

}

//从状态机
HTTPConn::LINE_STATUS HTTPConn::parse_line()
{
	m_read_idx;
	m_checked_idx;
	char tmp;
	for(; m_checked_idx < m_read_idx; m_checked_idx++)
	{
		tmp = m_read_buf[m_checked_idx];
		if(tmp == '\r')
		{
			// get xx'\r'，差了个'\n'
			if((m_checked_idx + 1) == m_read_idx)
			{
				return LINE_OPEN;
			}
			else if(m_read_buf[m_checked_idx + 1] == '\n')
			{
				m_read_buf[m_checked_idx++] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
		else if(tmp == '\n')
		{
			if(m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
			{
				m_read_buf[m_checked_idx - 1] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

HTTPConn::HTTP_CODE HTTPConn::parse_requestline(char *text)
{
	m_url = strpbrk(text, " \t");
	if(!m_url)
	{
		return BAD_REQUEST;
	}
	*m_url++ = '\0';	//截断

	char *method = text;	//字符串形式的method
	if(strcasecmp(method, "GET") == 0)
	{
		m_method = GET;
	}
	else if(strcasecmp(method, "POST") == 0)
	{
		m_method = POST;
	}
	else
	{
		return BAD_REQUEST;
	}
	m_url += strspn(m_url, " \t");	//
	
	m_version = strpbrk(m_url, " \t");
	if(!m_version)
	{
		return BAD_REQUEST;
	}
	*m_version++ = '\0';
	m_version += strspn(m_url, " \t");
	if(strcasecmp(m_version, "HTTP/1.1") != 0)	//只支持HTTP/1.1版本
	{
		return BAD_REQUEST;
	}
	//http协议辨认
	if(strncasecmp(m_version, "http://", 7) == 0)
	{
		m_url += 7;
		m_url = strchr(m_url, '/');
	}
	if(strncasecmp(m_version, "https://", 8) == 0)
	{
		m_url += 8;
		m_url = strchr(m_url, '/');
	}
	if(!m_url || m_url[0] != '/')
	{
		return BAD_REQUEST;
	}

	//当上边一段测到的末尾字符是/时，则显示 自己写的判断界面
	if(strlen(m_url) == 1)
	{
		strcat(m_url, "judge.html");
	}
	m_check_state =	CHECK_STATE_HEADER; 
	return NO_REQUEST;
}

//判断header是什么
HTTPConn::HTTP_CODE HTTPConn::parse_headers(char *text)
{
	char tmp;
	if(text[0] == '\0')	//表示header的读取理论上结束了
	{
		if(m_content_length != 0)
		{
			//进入这个分支意味着什么？
			m_check_state = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		return GET_REQUEST;
	}
	else if(strncasecmp(text, "Connection:", 11) == 0)
	{
		text += 11;
		text += strspn(text, " \t");
		if(strcasecmp(text, "keep-alive") == 0)
		{
			//m_linger = true;	//给谁用的？
		}
	}
	else if(strncasecmp(text, "Context-length:", 15) == 0)
	{
		text += 15;
		text += strspn(text, " \t");
		m_content_length = atol(text);	//原来是报文里有给!
	}
	else if(strncasecmp(text, "Host:", 5) == 0)
	{
		text += 5;
		text += strspn(text, " \t");
		m_host = text;
	}
	else
	{
		//不认识的Header
	}
	return NO_REQUEST;
}

HTTPConn::HTTP_CODE HTTPConn::parse_content(char *text)
{
	//已经读完了并解析了整个HTTP请求报文
	if(m_read_idx >= (m_content_length + m_checked_idx))
	{
		text[m_content_length] = '\0';	//定个终点

		return GET_REQUEST;
	}
	return NO_REQUEST;
}











char *HTTPConn::get_line()
{
	return m_read_buf + m_start_line;
}


//用于 执行client发来的请求的对应任务
HTTPConn::HTTP_CODE HTTPConn::do_request()
{
	//TODO
}

HTTPConn::HTTP_CODE HTTPConn::process_read()
{
	HTTPConn::LINE_STATUS line_status = LINE_OK;
	HTTPConn::HTTP_CODE ret = NO_REQUEST;
	char *text = nullptr;

	//下面这个循环就是 真实的读取报文时的主状态机循环
	//所以CHECK_STATE_CONTENT是给逻辑处理用的
	while((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || (line_status = parse_line()) == LINE_OK)
	{
		text = get_line();	//第一次get_line有何作用？
		m_start_line = m_checked_idx;

		switch(m_check_state)
		{
		case CHECK_STATE_REQUESTLINE:
		{
			ret = parse_requestline(text);	//该解析的都存在成员变量里了
			if(ret == BAD_REQUEST)
			{
				return BAD_REQUEST;
			}
			break;
		}
		case CHECK_STATE_HEADER:
		{
			ret = parse_headers(text);	//同上，该解析的都存在成员变量里了
			if(ret == BAD_REQUEST)
			{
				return BAD_REQUEST;
			}
			else if(ret == GET_REQUEST)
			{
				return do_request();
			}
			break;
		}
		case CHECK_STATE_CONTENT:
		{
			ret = parse_content(text);	//同上
			if(ret == GET_REQUEST)
			{
				return do_request();
			}
			line_status = LINE_OPEN;	//只有read_buf读完之后还没能LINE_OK才会触发这个并跳出，并让下一次调用process_read的时候再接着读取
			break;
		}
		default:
			return INTERNAL_ERROR;
		}
	}
	return NO_REQUEST;
}


//TODO
bool HTTPConn::process_write(HTTPConn::HTTP_CODE ret)
{
	
}

void HTTPConn::process()
{
	//先读到client发来的请求报文并解析请求
	HTTPConn::HTTP_CODE read_ret = process_read();
	if(read_ret == NO_REQUEST)
	{
		//不知道这行干了啥
		return;
	}

	//然后根据client请求报文的HTTP_CODE，让server向client写指定的资源
	bool write_ret = process_write(read_ret);
	if(!write_ret)
	{
		close_conn();	//没写成功则直接关闭连接
	}
	//不知道这行干了啥
}








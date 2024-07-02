#include "csapp.h"
#include <time.h>


int read_ini(char* filename,const char* key,void* value);
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
char add_log[1024];
int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    char ini[1024];
    if (read_ini("www.ini","port",&ini)==-1){
        printf("read_ini error\n");
        return 0;
    }
    port=atoi((const char*)ini);
    printf("\n%d\n",port);

    listenfd = Open_listenfd(port);// 打开一个监听套接字
    while (1) {// 执行典型的无限服务器循环，不断接受连接请求
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 

        // 添加访问记录
        char ipv4_address[1024];
        // 获取客户端 IPv4 地址
        inet_ntop(AF_INET,&clientaddr.sin_addr,ipv4_address,INET_ADDRSTRLEN);
        time_t nowt=time(0); // 获取当前时间
        struct tm* nowlt=localtime(&nowt); // 转成本地时间
        // 格式化日志记录字符串  存储在 add_log 
        sprintf(add_log,"%d/%d/%d %d:%d:%d IP:%s ",\
            nowlt->tm_year+1900,nowlt->tm_mon,nowlt->tm_mday,nowlt->tm_hour,\
            nowlt->tm_min,nowlt->tm_sec,ipv4_address);


        doit(connfd);   // 执行                                       
        Close(connfd);                                            
    }
}

// 简单的读取配置
int read_ini(char* filename, const char* key, void* value){
    FILE* fl;
    if ((fl=fopen(filename,"r"))==NULL){
        perror("fopen");
        return -1;
    }
    char sLine[1024];
    while ((fgets(sLine,1024,fl))!=NULL){
        if (strncmp(sLine,"//",2)==0) 
            continue;
        if ('#'==sLine[0]) 
            continue;
        char* eq=strchr(sLine,'=');
        if (NULL!=eq && strncmp(key,sLine,strlen(key))==0){
            sLine[strlen(sLine)-1]='\0';
            fclose(fl);
	        printf("\n%siswhat\n",(char*)eq);
            while (*(eq+1)==' ' && *(eq+1)!='.') eq++;
            strcpy(value,eq+1);
            return 0;
        }
    }
    fclose(fl);
    return -1;
}


// 处理请求
// 这段 doit 函数是一个处理客户端请求的核心函数，主要负责解析 HTTP 请求、
// 确定请求类型（静态内容或动态内容）、执行相应的处理函数，并记录请求日志。
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
  
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);                      // 读取浏览器的请求
    sscanf(buf, "%s %s %s", method, uri, version);          // 格式化读取请求
    if (strcasecmp(method, "GET")) {                        // 只能处理GET
        clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
        return;
    }
    sprintf(add_log,"%s%s%s\n",add_log,method,uri);
    
    // 添加写日志
    // 将字符串 add_log 的内容追加到文件末尾，并清空 add_log 数组
    FILE* fl;
    if ((fl=fopen("webserver.log","a+"))!=NULL){
        fputs((const char*)add_log,fl);
        add_log[0]='\0';
        fclose(fl);
    }
    
    read_requesthdrs(&rio);                                 // 读并忽略请求报头 

    // Parse URI from GET request
    is_static = parse_uri(uri, filename, cgiargs);
    
    if (stat(filename, &sbuf) < 0) {
	    clienterror(fd, filename, "404", "Not found",
		    "Tiny couldn't find this file");
	    return;
    }

    if (is_static) { // Serve static content        
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            // S_ISREG 此文件是否是普通文件  S_IRUSR & sbuf.st_mode 用户是否具有读权限
            clienterror(fd, filename, "403", "Forbidden",
                "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);        //servestatic
    }
    else { // Serve dynamic content
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //executable
	    clienterror(fd, filename, "403", "Forbidden",
			"Tiny couldn't run the CGI program");
	    return;
	}
	serve_dynamic(fd, filename, cgiargs);            //servedynamic
    }
}

// 从输入流中读取 HTTP 请求的头部信息，
// 并将每一行头部信息打印到标准输出，直到遇到空行为止。
// read_requesthdrs - read and parse HTTP request headers
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}

// 去除首尾的空格
void trim(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) {  // All spaces?
        return;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator
    *(end + 1) = '\0';
}

// 解析 URI（Uniform Resource Identifier，统一资源标识符）的函数 parse_uri。它根据 URI 
//的内容判断请求的是静态内容还是动态内容，并根据情况设置相应的 filename 和 cgiargs 参数
// parse_uri - parse URI into filename and CGI args
// return 0 if dynamic content, 1 if static
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    // uri :/index.html
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  // Static content 
        strcpy(cgiargs, "");  
        // 获取root     
        if (read_ini("www.ini","root",filename)==-1){
            printf("read_ini error\n");
            return -1;
        }
        // 检查 filename 是否以 '/' 开头，如果不是，则将 filename 设置为当前目录 "."
        if (filename[0]!='/') 
        strcpy(filename, "."); 
        printf("\n\n\n%sis root\n\n\n",filename);
        // strcpy(filename, "."); 
        trim(filename);                         
        strcat(filename, uri);  
        //  如果 uri 的最后一个字符是 '/'，
        // 则追加 "index.html" 到 filename 的末尾，以处理默认文件的情况                    
        if (uri[strlen(uri)-1] == '/')           
            strcat(filename, "index.html");         
        return 1;
    }
    else {  // Dynamic content                   
	ptr = index(uri, '?');                         
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else 
	    strcpy(cgiargs, "");
	strcpy(filename, ".");
	strcat(filename, uri);
	return 0;
    }
}
// serve_static - copy a file back to the client 
// 向客户端发送静态文件的内容
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    // Send response headers to client
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    // Send response body to client
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

// get_filetype - derive file type from file name根据文件名获取文件类型
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}

// serve_dynamic - run a CGI program on behalf of the client
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    // Return first part of HTTP response
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) {
	// Real server would set all CGI vars here
	setenv("QUERY_STRING", cgiargs, 1);
	Dup2(fd, STDOUT_FILENO);         // Redirect stdout to client 
	Execve(filename, emptylist, environ); // Run CGI program 
    }
    Wait(NULL); // Parent waits for and reaps child
}

// clienterror - returns an error message to the client
// 向客户端发送一个简单的 HTML 格式的错误页面作为 HTTP 响应。
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    // Build the HTTP response body
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    // Print the HTTP response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

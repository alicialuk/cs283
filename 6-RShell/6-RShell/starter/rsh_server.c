#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include "rshlib.h"
#include "dshlib.h"

static int g_server_socket = -1;

void handle_signal(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\nReceived signal %d, shutting down server...\n", signum);
        if (g_server_socket >= 0) {
            close(g_server_socket);
            g_server_socket = -1;
        }
        exit(0);
    }
}

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    signal(SIGPIPE, SIG_IGN);

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        int err_code = svr_socket;
        return err_code;
    }
    
    g_server_socket = svr_socket;

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);
    
    g_server_socket = -1;

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */

int stop_server(int svr_socket) {
    int result = OK;
    
    g_server_socket = -1;
    
    if (svr_socket >= 0) {
        result = close(svr_socket);
        if (result < 0) {
            perror("Error closing server socket");
            return ERR_RDSH_SERVER;
        }
    }
    
    printf("Server stopped successfully\n");
    return result;
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */

int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in server_addr;
    int enable = 1;
    
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket creation failed");
        return ERR_RDSH_COMMUNICATION;
    }
    
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt failed");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ifaces, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    if (bind(svr_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    if (listen(svr_socket, 5) < 0) {
        perror("listen failed");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    printf("Server started on %s:%d\n", ifaces, port);
    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket) {
    int client_sock, rc;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (1) {
        client_sock = accept(svr_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            if (errno == EINTR) {
                if (g_server_socket < 0) {
                    return OK;
                }
                continue;
            }
            
            perror("accept failed");
            return ERR_RDSH_COMMUNICATION;
        }
        
        printf("Client connected from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        rc = exec_client_requests(client_sock);
        
        close(client_sock);
        
        if (rc == OK_EXIT) {
            printf("%s", RCMD_MSG_SVR_STOP_REQ);
            break;
        } else {
            printf("%s", RCMD_MSG_CLIENT_EXITED);
        }
    }
    
    return OK;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    char *buffer;
    int recv_size;
    int result = OK;
    command_list_t cmd_list;
    
    buffer = malloc(RDSH_COMM_BUFF_SZ);
    if (!buffer) {
        perror("malloc failed");
        return ERR_RDSH_COMMUNICATION;
    }
    
    while (1) {
        char cmd_buffer[RDSH_COMM_BUFF_SZ] = {0};
        int cmd_len = 0;
        int is_last_chunk = 0;
        
        while (!is_last_chunk) {
            recv_size = recv(cli_socket, buffer, RDSH_COMM_BUFF_SZ, 0);
            
            if (recv_size <= 0) {
                if (recv_size < 0) {
                    perror("recv failed");
                    free(buffer);
                    return ERR_RDSH_COMMUNICATION;
                } else {
                    free(buffer);
                    return OK;
                }
            }
            
            for (int i = 0; i < recv_size; i++) {
                if (buffer[i] == '\0') {
                    is_last_chunk = 1;
                    if (cmd_len + i + 1 < RDSH_COMM_BUFF_SZ) {
                        memcpy(cmd_buffer + cmd_len, buffer, i + 1);
                        cmd_len += i + 1;
                    }
                    break;
                }
            }
            
            if (!is_last_chunk) {
                if (cmd_len + recv_size < RDSH_COMM_BUFF_SZ) {
                    memcpy(cmd_buffer + cmd_len, buffer, recv_size);
                    cmd_len += recv_size;
                }
            }
        }
        
        printf(RCMD_MSG_SVR_EXEC_REQ, cmd_buffer);
        
        if (strlen(cmd_buffer) == 0) {
            send_message_eof(cli_socket);
            continue;
        }
        
        if (strcmp(cmd_buffer, "exit") == 0) {
            send_message_string(cli_socket, "Exiting client session\n");
            result = OK;
            break;
        } else if (strcmp(cmd_buffer, "stop-server") == 0) {
            send_message_string(cli_socket, "Stopping server\n");
            result = OK_EXIT;
            break;
        }
        
        int parse_result = build_cmd_list(cmd_buffer, &cmd_list);
        
        if (parse_result == WARN_NO_CMDS) {
            send_message_string(cli_socket, CMD_WARN_NO_CMD);
            continue;
        } else if (parse_result == ERR_TOO_MANY_COMMANDS) {
            char error_msg[RDSH_COMM_BUFF_SZ];
            snprintf(error_msg, RDSH_COMM_BUFF_SZ, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            send_message_string(cli_socket, error_msg);
            continue;
        } else if (parse_result != OK) {
            send_message_string(cli_socket, "Error parsing command\n");
            continue;
        }
        
        if (cmd_list.num > 0) {
            Built_In_Cmds cmd_type = rsh_match_command(cmd_list.commands[0].argv[0]);
            
            if (cmd_type == BI_CMD_CD) {
                if (cmd_list.commands[0].argc > 1) {
                    if (chdir(cmd_list.commands[0].argv[1]) == 0) {
                        char msg[RDSH_COMM_BUFF_SZ];
                        snprintf(msg, RDSH_COMM_BUFF_SZ, "Changed directory to %s\n", 
                                cmd_list.commands[0].argv[1]);
                        send_message_string(cli_socket, msg);
                    } else {
                        char msg[RDSH_COMM_BUFF_SZ];
                        snprintf(msg, RDSH_COMM_BUFF_SZ, "Failed to change directory to %s\n", 
                                cmd_list.commands[0].argv[1]);
                        send_message_string(cli_socket, msg);
                    }
                } else {
                    const char *home = getenv("HOME");
                    if (home && chdir(home) == 0) {
                        send_message_string(cli_socket, "Changed to home directory\n");
                    } else {
                        send_message_string(cli_socket, "Failed to change to home directory\n");
                    }
                }
            } else if (cmd_type == BI_CMD_DRAGON) {
                send_message_string(cli_socket, "Roarrr! The dragon appears!\n");
            } else {
                int exit_code = rsh_execute_pipeline(cli_socket, &cmd_list);
                printf(RCMD_MSG_SVR_RC_CMD, exit_code);
            }
            
            free_cmd_list(&cmd_list);
        } else {
            send_message_eof(cli_socket);
        }
    }
    
    free(buffer);
    return result;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int socket) {
    int bytes_sent;
    
    bytes_sent = send(socket, &RDSH_EOF_CHAR, 1, 0);
    if (bytes_sent == 1) {
        return OK;
    }
    
    return ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */

int send_message_string(int cli_socket, char *buff) {
    int bytes_sent;
    int msg_len = strlen(buff);
    
    bytes_sent = send(cli_socket, buff, msg_len, 0);
    if (bytes_sent != msg_len) {
        printf(CMD_ERR_RDSH_SEND, bytes_sent, msg_len);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return send_message_eof(cli_socket);
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int i, status;
    pid_t pid, last_pid;
    int pipefds[2*CMD_MAX];
    
    for (i = 0; i < clist->num - 1; i++) {
        if (pipe(pipefds + 2*i) < 0) {
            perror("pipe creation failed");
            return ERR_RDSH_CMD_EXEC;
        }
    }
    
    for (i = 0; i < clist->num; i++) {
        pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            return ERR_RDSH_CMD_EXEC;
        } else if (pid == 0) {
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            } else {
                dup2(pipefds[2*(i-1)], STDIN_FILENO);
            }
            
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                dup2(pipefds[2*i + 1], STDOUT_FILENO);
            }
            
            for (int j = 0; j < 2*(clist->num-1); j++) {
                close(pipefds[j]);
            }
            
            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) < 0) {
                fprintf(stderr, "exec failed: %s\n", clist->commands[i].argv[0]);
                exit(1);
            }
        }
        
        if (i == clist->num - 1) {
            last_pid = pid;
        }
    }
    
    for (i = 0; i < 2*(clist->num-1); i++) {
        close(pipefds[i]);
    }
    
    waitpid(last_pid, &status, 0);
    
    while (wait(NULL) > 0);
    
    send_message_eof(cli_sock);
    
    return WEXITSTATUS(status);
}

Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "stop-server") == 0) {
        return BI_CMD_STOP_SVR;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    }
    
    return BI_NOT_BI;
}
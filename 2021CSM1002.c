#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <ctype.h>
#include <curl/curl.h>
#include <time.h>

#define PORT 7000
#define BUFFER_SIZE 1024
#define MAX 1000
#define NORMAL_MAX 100
#define DELUXE_MAX 50
#define PREMIUM_MAX 30
#define MAX_THREAD_COUNT 10
#define BACKLOGS 20
#define NORMAL "NORMAL"
#define DELUXE "DELUXE"
#define PREMIUM "PREMIUM"
#define FROM "anuragjais111@gmail.com"


int thread_count = 0;
int normal_ticket_count = 0;
int deluxe_ticket_count = 0;
int premium_ticket_count = 0;
int ticket_count = 0;
sem_t mutex;
sem_t normal_mutex;
sem_t deluxe_mutex;
sem_t premium_mutex;

struct node
{
    char *email;
    char *name;
    int quantity;
    char *ticket_type;
    struct node *next;
};

struct node *normal_tickets[MAX];
struct node *deluxe_tickets[MAX];
struct node *premium_tickets[MAX];

char *email_body[100000];
int body_index = 0;

int send_message(char *To, FILE *f)
{
    CURL *curl;
    CURLcode res = CURLE_OK;
    char errbuf[CURL_ERROR_SIZE];
    struct curl_slist *recipients = NULL;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

        recipients = curl_slist_append(recipients, To);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READDATA, f);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        curl_easy_setopt(curl, CURLOPT_USERNAME, "anuragjais111@gmail.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "kibcvaruyhoecdwh");

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %d - %s\n%s\n",
                    res, curl_easy_strerror(res), errbuf);

        curl_easy_cleanup(curl);
    }

    return (int)res;
}
int send_email(char *email, char *emailFileName)
{
    // struct curl_slist *recipients = NULL;
    int ret;
    FILE *f;
    // read in the email contents and send the message
    f = fopen(emailFileName, "rb");
    ret = send_message(email, f);
    fclose(f);
    /* Free the list of recipients */
    // curl_slist_free_all(email);

    return ret;
}

void print_ticket_details(char *name,char *email,int curr_count,int ticket_type_count,int quantity,char *ticket_type){
    printf("print Ticket Details on server side\n");
    printf("%s-", name);
    printf("The ticket booking is successfull.\n");
    printf("Ticket numbers: ");
    for (int i = curr_count + 1; i <= ticket_type_count; i++)
    {
        printf("%d ", i);
    }
    printf("Total tickets booked %d\n", quantity);
    printf("Ticket Type = %s\n", ticket_type);
    printf("sent to %s ", email);
}


void write_email_message(char *email,char *name,int curr_count,int ticket_type_count,int quantity,char *ticket_type){
    FILE *fp;
    fp = fopen("email_message.txt", "w");
    fprintf(fp, "TO: %s\r\n", email);
    fprintf(fp, "FROM: %s\r\n", FROM);
    fprintf(fp, "Subject: Ticket Booking Confirmation\r\n");
    fprintf(fp, "\r\n");
    fprintf(fp, "%s- The ticket booking is successfull\r\n", name);
    fprintf(fp, "Ticket Numbers: \r\n");
    for (int i = curr_count + 1; i <= ticket_type_count; i++)
    {
        fprintf(fp, "%d ", i);
    }
    fprintf(fp, "\r\n");
    fprintf(fp, "Total tickets booked %d\r\n", quantity);
    fprintf(fp,"Ticket type %s\r\n",ticket_type);
    fprintf(fp,"\r\n");
    fprintf(fp,"Thanks for booking ticket\r\n");
    fprintf(fp, "Have a great day.\r\n");
    fclose(fp);
}

void book_normal_ticket(int socket, char *email, char *name, int quantity)
{
    if ((normal_ticket_count + quantity) < NORMAL_MAX)
    {

        int temp = quantity;
        int curr_count = normal_ticket_count;
        while (temp > 0)
        {
            struct node *ticket;
            ticket = (struct node *)(malloc(sizeof(struct node)));
            ticket->email = email;
            ticket->name = name;
            ticket->ticket_type = "NORMAL\0";
            ticket->quantity = quantity;
            normal_tickets[normal_ticket_count++] = ticket;
            temp--;
        }
        print_ticket_details(name,email,curr_count,normal_ticket_count,quantity,NORMAL);
        write_email_message(email,name,curr_count,normal_ticket_count,quantity,NORMAL);
        send_email(email, "email_message.txt");
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "Your Ticket is booked. Please check your email\n";
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
    else
    {
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "No sufficient tickets are available in this category.";
        printf("%s ",name);
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
}

void book_deluxe_ticket(int socket, char *email, char *name, int quantity)
{
    if ((deluxe_ticket_count + quantity) < DELUXE_MAX)
    {

        int temp = quantity;
        int curr_count = deluxe_ticket_count;
        while (temp > 0)
        {
            struct node *ticket;
            ticket = (struct node *)(malloc(sizeof(struct node)));
            ticket->email = email;
            ticket->name = name;
            ticket->ticket_type = DELUXE;
            ticket->quantity = quantity;
            deluxe_tickets[deluxe_ticket_count++] = ticket;
            temp--;
        }
        print_ticket_details(name,email,curr_count,deluxe_ticket_count,quantity,DELUXE);
        write_email_message(email,name,curr_count,deluxe_ticket_count,quantity,DELUXE);
        send_email(email, "email_message.txt");
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "Your Ticket is booked. Please check your email\n";
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
    else
    {
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "No sufficient tickets are available in this category.";
        printf("%s ",name);
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
}

void book_premium_ticket(int socket, char *email, char *name, int quantity)
{
    if ((premium_ticket_count + quantity) < PREMIUM_MAX)
    {
        int temp = quantity;
        int curr_count = premium_ticket_count;
        while (temp > 0)
        {
            struct node *ticket;
            ticket = (struct node *)(malloc(sizeof(struct node)));
            ticket->email = email;
            ticket->name = name;
            ticket->ticket_type = PREMIUM;
            ticket->quantity = quantity;
            premium_tickets[premium_ticket_count++] = ticket;
            temp--;
        }
        print_ticket_details(name,email,curr_count,premium_ticket_count,quantity,PREMIUM);
        write_email_message(email,name,curr_count,premium_ticket_count,quantity,PREMIUM);
        send_email(email, "email_message.txt");
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "Your Ticket is booked. Please check your email\n";
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
    else
    {
        send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
        char *reply = "No sufficient tickets are available in this category.";
        printf("%s ",name);
        printf("%s\n", reply);
        write(socket, reply, strlen(reply));
    }
}
// void serve_dynamic(char *query_string,){
int is_character(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int is_email_valid(char *email)
{
    if (!is_character(email[0]))
    {
        return 0;
    }
    int at_the_rate = -1, dot_sign = -1;
    for (int i = 0;
         i < strlen(email); i++)
    {
        if (email[i] == '@')
        {
            at_the_rate = i;
            if (email[i + 6] != '\0' && email[i + 6] == '.')
            {
                dot_sign = i;
                break;
            }
        }
    }
    if (at_the_rate == -1 && dot_sign == -1)
        return 0;
    if (at_the_rate > dot_sign)
        return 0;
    return !(dot_sign >= (strlen(email) - 1));
}
void *connection_handler(void *socket_desc)
{

    int request;
    char client_reply[BUFFER_SIZE], *request_lines[5];
    char *file_name;
    char *extension;

    // Get the socket descriptor.
    int sock = *((int *)socket_desc);

    // Get the request.
    request = recv(sock, client_reply, BUFFER_SIZE, 0);

    sem_wait(&mutex);
    thread_count++;

    if (thread_count > MAX_THREAD_COUNT) // If there is 15 request at the same time, other request will be refused.
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>Server is busy.</body></html>";
        write(sock, message, strlen(message));
        thread_count--;
        sem_post(&mutex);
        free(socket_desc);
        shutdown(sock, SHUT_RDWR);
        close(sock);
        sock = -1;
        pthread_exit(NULL);
    }
    sem_post(&mutex);

    if (request < 0) // Receive failed.
    {
        puts("Recv failed");
    }
    else if (request == 0) // receive socket closed. Client disconnected upexpectedly.
    {
        puts("Client disconnected upexpectedly.");
    }
    else
    {
        request_lines[0] = strtok(client_reply, " \t\n");
        if (strncmp(request_lines[0], "GET\0", 4) == 0)
        {

            request_lines[1] = strtok(NULL, " \t/?");
            // request_lines[2] = strtok(NULL, " \t\n");
            // printf("%s\n", request_lines[1]);
            char *email, *name, *ticket_type, *ticket_count;
            int quantity = -1;
            char *tokens = request_lines[1];

            // printf("tokens\n");
            // printf("%s\n", tokens);
            // printf("%s\n", request_lines[1]);
            char *p = request_lines[1];

            int is_invalid = 0;
            int count = 0;
            char errMsg[1000];

            while ((p = strsep(&tokens, "&\n")))
            {
                char *var = strtok(p, "="),
                     *val = NULL;
                if (var && (val = strtok(NULL, "=")))
                {
                    // printf("%-8s    %s\n", var, val);
                    if (strcmp(var, "Guest_Email") == 0)
                    {
                        email = val;
                        // printf("Guest_Email = %s\n", email);

                        if (!is_email_valid(email))
                        {
                            is_invalid = 1;
                            strcpy(errMsg, "Invalid email address");
                            break;
                        }

                    }
                    else if (strcmp(var, "Guest_Name") == 0)
                    {
                        name = val;
                        // printf("Guest Name = %s\n", name);

                        for (int i = 0; i < strlen(name); i++)
                        {
                            if (!isalpha(name[i]))
                            {
                                is_invalid = 1;
                                strcpy(errMsg, "Guest Name contains only characters");
                                break;
                            }
                        }

                        if (is_invalid)
                            break;

                    }
                    else if (strcmp(var, "Ticket_Type") == 0)
                    {
                        ticket_type = val;
                        // printf("ticket_type = %s\n", ticket_type);

                        if (strcmp(ticket_type, NORMAL) == 0)
                        {
                            is_invalid = 0;
                        }
                        else if (strcmp(ticket_type, DELUXE) == 0)
                        {
                            is_invalid = 0;
                        }
                        else if (strcmp(ticket_type, PREMIUM) == 0)
                        {
                            is_invalid = 0;
                        }
                        else
                        {
                            is_invalid = 1;
                            strcpy(errMsg, "Ticket Type Key error only NORMAL,DELUXE,PREMIUM valid all in caps");
                            // break;
                        }

                        if( is_invalid ){
                            break;
                        }
                    }
                    else if (strcmp(var, "Quantity") == 0)
                    {
                        ticket_count = val;
                        // printf("ticket_count = %s\n", ticket_count);
                        if (ticket_count)
                        {
                            quantity = atoi(ticket_count);

                            // printf("quantity = %d\n", quantity);
                            if (quantity <= 0)
                            {
                                is_invalid = 1;

                                strcpy(errMsg, "Invalid quantity requested");
                                break;
                            }
                            // else
                            // {
                                // if (strcmp(ticket_type, NORMAL) == 0)
                                // {
                                //     if (quantity > NORMAL_MAX)
                                //     {
                                //         is_invalid = 1;
                                //         strcpy(errMsg, "NORMAL Ticket quantity is out of range");
                                //         break;
                                //     }
                                // }
                                // else if (strcmp(ticket_type, DELUXE) == 0)
                                // {
                                //     if (quantity > DELUXE_MAX)
                                //     {
                                //         is_invalid = 1;
                                //         strcpy(errMsg, "DELUXE Ticket quantity is out of range");
                                //         break;
                                //     }
                                // }
                                // else if (strcmp(ticket_type, PREMIUM) == 0)
                                // {
                                //     if (quantity > PREMIUM_MAX)
                                //     {
                                //         is_invalid = 1;
                                //         strcpy(errMsg, "PREMIUM Ticket quantity is out of range");
                                //         break;
                                //     }
                                // }
                            //}
                        }
                        else
                        {
                            is_invalid = 1;
                            strcpy(errMsg, "Quantity can not be null");
                            break;
                        }
                    }
                }

                else
                {
                    printf("Field Error\n");
                    fputs("<empty field>\n", stderr);
                    is_invalid = 1;
                    strcpy(errMsg, "Empty Field Error");
                    break;
                }
            }

            if (is_invalid)
            {

                if( name ){
                    printf("%s-", name);
                }
                printf("%s\n", errMsg);

                send(sock, "HTTP/1.0 400 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
                // char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
                write(sock, errMsg, strlen(errMsg));
            }
            else if( name == NULL ){
                printf("Guest_Name is missing\n");
                strcpy(errMsg, "Guest_Name is missing");
                send(sock, "HTTP/1.0 400 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
                // char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
                write(sock, errMsg, strlen(errMsg));
            }
            else if( email == NULL ){
                printf("Guest_Email is missing\n");
                strcpy(errMsg, "Guest_Email is missing");
                send(sock, "HTTP/1.0 400 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
                // char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
                write(sock, errMsg, strlen(errMsg));
            }
            else if( ticket_type == NULL){
                printf("Ticket_Type field is missing");
                strcpy(errMsg, "Ticket_Type field is missing");
                send(sock, "HTTP/1.0 400 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
                // char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
                write(sock, errMsg, strlen(errMsg));
            }
            else if( quantity == -1){
                printf("Quantity field is missing\n");
                strcpy(errMsg, "Quantity field is missing");
                send(sock, "HTTP/1.0 400 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
                // char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
                write(sock, errMsg, strlen(errMsg));

            }
            else
            {
                if (strcmp(ticket_type, NORMAL) == 0)
                {

                    sem_wait(&normal_mutex);
                    book_normal_ticket(sock, email, name, quantity);
                    sem_post(&normal_mutex);
                }
                else if (strcmp(ticket_type, DELUXE) == 0)
                {
                    sem_wait(&deluxe_mutex);
                    book_deluxe_ticket(sock, email, name, quantity);
                    sem_post(&deluxe_mutex);
                }
                else if (strcmp(ticket_type, PREMIUM) == 0)
                {
                    sem_wait(&premium_mutex);
                    book_premium_ticket(sock, email, name, quantity);
                    sem_post(&premium_mutex);
                }
            }
        }
    }

    free(socket_desc);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;
    sem_wait(&mutex);
    thread_count--;
    sem_post(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char *argv)
{

    sem_init(&mutex, 0, 1);
    sem_init(&normal_mutex, 0, 1);
    sem_init(&deluxe_mutex, 0, 1);
    sem_init(&premium_mutex, 0, 1);
    int socket_desc, c, new_socket, *new_sock;
    struct sockaddr_in server, client;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Socket not created\n");
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Socket binding failed\n");
        return 1;
    }

    listen(socket_desc, BACKLOGS);

    printf("Server started at %d port number\n", PORT);

    printf("Waiting for incoming connection requests\n");

    c = sizeof(struct sockaddr_in);

    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        printf("Client connected \n");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
        {
            puts("Not able to create thread\n");
            return 1;
        }
    }
    return 0;
}
/* 
 * frmt.c
 *
 * Various utilties used for formatting parts of an http response
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "inc/frmt.h"
#define LINE_SIZE 256

/* frmt_set_resline: formats the resline (code, msg, version) for a response 
 * @param: buf a ptr to a buffer that will be filled
 * @param: size the max number of bytes buf can be filled with
 * @param: ver a string representing the version of the server 
 * @param: code an int that is used to get the status message 
 * @returns: bytes written to buf, -1 on err */
int frmt_set_resline(char *buf, int size, char *ver, int code) {
    char *msg; 
    switch (code) {
        case 200:
            msg = "OK";
            break;
        case 400:
            msg = "Bad Request";
            break;
        case 403:
            msg = "Forbidden";
            break;
        case 404:
            msg = "Not Found";
            break;
        default:
            printf("not found...\n");
            return -1;
    }
    return snprintf(buf, size, "%s %d %s\r\n", ver, code, msg); 
}

/* frmt_set_servline: formats the server line for a response 
 * @param: buf a ptr to a buffer that will be filled
 * @param: size the max number of bytes buf can be filled with
 * @param: serv a string representing the name of the server
 * @returns: bytes written to buf, -1 on err*/
int frmt_set_servline(char *buf, int size, char *serv) {
    return snprintf(buf, size, "Server: %s\r\n", serv);
}

/* frmt_set_content_len: formats content-length line for a response 
 * @param: buf a ptr to a buffer to be filled 
 * @param: size max number of bytes buf can be filled with
 * @param: len the size in bytes to be formatted
 * @returns: bytes buf filled with, -1 on err */
int frmt_set_cont_len(char *buf, int size, int len) {
    return snprintf(buf, size, "Content-Length: %d\r\n", len);
}

/* frmt_set_content_type: formats content-type line for a response 
 * @param: buf a ptr to a buffer to be filled 
 * @param: size max number of bytes buf can be filled with
 * @param: len the size in bytes to be formatted
 * @returns: bytes buf filled with, -1 on err */
int frmt_set_cont_type(char *buf, int size, char *type) {
    return snprintf(buf, size, "Content-Type: %s\r\n\r\n", type);
}

/* frmt_set_date: formats a date line with a given title
 * @param: buf a ptr to a buffer that will be filled
 * @param: size max number of bytes buf can be filled with
 * @param: date the time to be formatted
 * @param: title the string to use for the prefix of the header
 * @returns: bytes written to buf, -1 on err */
int frmt_set_date(char *buf, int size, time_t *date, char *title) {
    char time_buf[LINE_SIZE]; 
    struct tm tm = *gmtime(date);
    strftime(time_buf, LINE_SIZE, "%a, %d %b %Y %H:%M:%S GMT\r\n", &tm);
    return snprintf(buf, size, "%s: %s", title, time_buf);
}




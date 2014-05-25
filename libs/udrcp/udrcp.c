/*
     Copyright 2012-2014 Infinitycoding all rights reserved
     This file is part of the UDRCP-library.

     The UDRCP-library is free software: you can redistribute it and/or modify
     it under the terms of the GNU Lesser General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     any later version.

     The UDRCP-library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public License
     along with the UDRCP-library.  If not, see <http://www.gnu.org/licenses/>.
 */



/**
 *  @author Simon Diepold aka. Tdotu <simon.diepold@infinitycoding.de>
 */

#include <udrcp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


/**
 * @brief Creates an ner Package manager.
 * @param in the input pipe
 * @param out the putput pipe
 * @param err the error-log pipe/file
 * @return a new package manager
 */
pckmgr *new_pckmgr(int in, int out, int err)
{
    pckmgr *mgr = malloc(sizeof(pckmgr));
    mgr->counter = 0;
    mgr->used_ids = list_create();
    mgr->in = in;
    mgr->out = out;
    mgr->err = err;
    mgr->recieved_pcks = list_create();
    return mgr;
}


/**
 * @brief generates a new package-ID.
 * @param mgr the package manager which shuld send the message
 * @return the package ID
 */
pckid_t gen_pckid(pckmgr *mgr)
{
    list_set_first(mgr->used_ids);
    while(!list_is_empty(mgr->used_ids) && !list_is_last(mgr->used_ids))
    {
        if((pckid_t)list_get_current(mgr->used_ids) == mgr->counter )
        {
            if(mgr->counter == MAX_ID)
                mgr->counter = 0;
            else
                mgr->counter++;

            list_set_first(mgr->used_ids);
        }
        else
            list_next(mgr->used_ids);
    }
    list_push_front(mgr->used_ids,(void*)mgr->counter);
    return mgr->counter++;
}


/**
 * @brief frees a generated package-ID.
 * @param mgr the package manager
 * @param id the ID to free
 * @return true if success, false if id is not generated by this manager
 */
int free_pckid(pckmgr *mgr, pckid_t id)
{
    list_set_first(mgr->used_ids);
    while(!list_is_empty(mgr->used_ids) && !list_is_last(mgr->used_ids))
    {
        if((pckid_t)list_get_current(mgr->used_ids) == id )
        {
            list_remove(mgr->used_ids);
            return true;
        }
        else
            list_next(mgr->used_ids);
    }

    return false;
}


/**
 * @brief Frees a package.
 * @param package the package to be freed
 */
void free_pck(pck_t *package)
{
    if(package->data > 0)
        free(package->data);
    free(package);
}


/**
 * @brief Sends a Package via mgr.
 * @param mgr the manger which should send the message
 * @param type the type of the package
 * @param size the size of the data which should be appended to the package
 * @param data pointer to the data which should be appended
 * @return the package ID of the sended package
 */
pckid_t send_package(pckmgr *mgr, pcktype_t type, size_t size, void *data)
{
    pckhead_t *header = malloc(sizeof(pckhead_t));
    pckid_t id = gen_pckid(mgr);
    header->id = id;
    header->size = size+12;
    header->type = type;
    write(mgr->out,header,sizeof(pckhead_t));
    write(mgr->out,data,size);
    free(header);
    return id;
}


/**
 * @brief Responds to a request package.
 * @param mgr the manger which should send the response
 * @param id the id of the request package
 * @param size the size of the data which should be appended to the response
 * @param data pointer to the data which should be appended
 */
void respond(pckmgr *mgr, pckid_t id, pcktype_t type, size_t size, void *data)
{
    pckhead_t *header = malloc(sizeof(pckhead_t));
    header->id = id;
    header->size = size+12;
    header->type = type;
    write(mgr->out,header,sizeof(pckhead_t));
    write(mgr->out,data,size);
    free(header);
}


/**
 * @brief Conect to another udrcp-system partner
 * @param mgr the manger which should connect
 * @param protocol_version the protocoll version string
 * @return true if sucess, false if the connection failed
 */
int subsystem_connect(pckmgr *mgr, char *protocol_version)
{
    send_package(mgr, RESET_CON, 0, NULL);
    pckid_t ping_id = send_package(mgr, PING, strlen(protocol_version), protocol_version);
    pckhead_t *pck_header = malloc(sizeof(pckhead_t));
    read(mgr->in,pck_header,sizeof(pckhead_t));
    if(pck_header->id == ping_id && pck_header->type == PONG)
    {
        mgr->version = malloc(pck_header->size);
        read(mgr->in,mgr->version,pck_header->size-sizeof(pckhead_t));
    }
    else
    {
        udrcp_error(mgr,"invalind ping response from host ID:%d TYPE:%d Size:%d\n",pck_header->id,pck_header->type,pck_header->size);
        free(pck_header);
        return false;
    }
    //todo: check udrcp version
    free(pck_header);
    return true;
}


/**
 * @breif Waits for the next package and retuns it.
 * @param mgr the manger which should poll for the package
 * @return the next package
 */
pck_t *poll_next(pckmgr *mgr)
{
    pck_t *pck = malloc(sizeof(pck_t));
    read(mgr->in,pck,sizeof(pckhead_t));
    if(pck->size > 12)
    {
        pck->size -= 12;
        pck->data = malloc(pck->size);
        read(mgr->in,pck->data,pck->size);
    }
    else
        pck->data = NULL;
    return pck;
}


/**
 * @brief Waits for the next package and puts it into the queue.
 * @param mgr the manger which should poll for the package
 */
void poll_queue(pckmgr *mgr)
{
    list_push_front(mgr->recieved_pcks,poll_next(mgr));
}


/**
 * @brief Fetches a package by ID from the queue.
 * @param mgr the manager
 * @param id the ID of the package
 * @return NULL if the package is not in the queue else the requested package
 */
pck_t *fetch_queue(pckmgr *mgr,pckid_t id)
{
    list_set_first(mgr->recieved_pcks);
    while(!list_is_last(mgr->recieved_pcks) && !list_is_empty(mgr->recieved_pcks))
    {
        pck_t *current = list_get_current(mgr->recieved_pcks);
        if(current->id == id)
        {
            list_remove(mgr->recieved_pcks);
            return current;
        }
        list_next(mgr->recieved_pcks);
    }
    return NULL;
}


/**
 * @brief Waits for a specific package and puts all other incoming packages into the queue.
 * @param mgr the manager
 * @param id the id of the specific package
 * @return the package
 */
pck_t *pck_poll(pckmgr *mgr, pckid_t id)
{
    pck_t *pck = fetch_queue(mgr,id);
    if(pck)
        return pck;

    pck = poll_next(mgr);
    while(pck->id != id)
    {
        list_push_front(mgr->recieved_pcks,pck);
        pck = poll_next(mgr);
    }
    return pck;
}


/**
 * @brief Resets the UDRCP-Connection and all UDRCP-package-IDs
 * @param mgr the manager of the connection to be reseted
 */
void reset_conn(pckmgr *mgr)
{
    mgr->counter = 0;
    while(!list_is_empty(mgr->used_ids))
        free_pck(list_pop_front(mgr->used_ids));
    while(!list_is_empty(mgr->recieved_pcks))
        free_pck(list_pop_front(mgr->recieved_pcks));
}


/**
 * @breif Request a interrupt singal for each occuring num interrupt
 * @param mgr the manager which should request the signals
 * @param num the interrupt number
 * @return true if success, false if failure
 */
int req_intsig(pckmgr *mgr, unsigned int num)
{
    pckid_t id = send_package(mgr, INT_REQ, sizeof(unsigned int), &num);
    int errorcode = true;
    while(errorcode)
    {
        pck_t *resp = pck_poll(mgr,id);
        switch(resp->type)
        {
        case ERROR:
            if(resp->size >= 4)
                errorcode = *((unsigned int*)resp->data);
            else
                errorcode = -1;
            break;

        case CONFIRM:
            break;

        case SUCCESS:
            free_pck(resp);
            return true;
            break;

        default:
            udrcp_error(mgr,"unknown response type. [ID: %d, size: %d, type: %d]\n",resp->id,resp->size,resp->type);
            errorcode = -2;
            break;

        }
        //free_pck(resp);
    }
    return errorcode;
}


/**
 * @breif Stops recieving interrupt signals for interrupt num.
 * @param mgr the manger 
 * @param num the interrupt singal number
 * @return true if success, false if failure
 */
int free_intsig(pckmgr *mgr, unsigned int num)
{
    pckid_t id = send_package(mgr, INT_FREE, sizeof(unsigned int), &num);
    int errorcode = true;
    while(errorcode)
    {
        pck_t *resp = pck_poll(mgr,id);
        switch(resp->type)
        {
        case ERROR:
            if(resp->size >= 4)
                errorcode = *((unsigned int*)resp->data);
            else
                errorcode = -1;
            break;

        case CONFIRM:
            break;

        case SUCCESS:
            free_pck(resp);
            return true;
            break;

        default:
            udrcp_error(mgr,"unknown response type. [ID: %d, size: %d, type: %d]\n",resp->id,resp->size,resp->type);
            errorcode = -2;
            break;

        }
        free_pck(resp);
    }
    return errorcode;
}


/**
 * @brief Writes a error message in the UDRCP-error Pipe/file
 * @param mgr the connection manager which should report
 * @param format a printf like format string
 * @return the number of written characters
 */
int udrcp_error(pckmgr *mgr,const char *format,...)
{
    char buffer[512]; //todo: should be dynamic
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);
    return write(mgr->err,buffer,strlen(buffer));
}



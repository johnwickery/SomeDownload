#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    DBusError err;	
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessageIter arg;
    DBusPendingCall *pending;
    char *value = "METHOD_CALL";
    char *replyvalue = NULL;
    int ret;

    /* Create bus connection */
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "dbus_bus_get: %s\n", err.message);
        dbus_error_free(&err);
        return -1;
    }

    /* Request well-known name for connection */
    ret = dbus_bus_request_name(conn, "org.wuhan.client", 
               DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "dbus_bus_request_name: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return -2;
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        fprintf(stderr, "Err: DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER\n");
        dbus_connection_unref(conn);
        return -3;
    }

    /* Create Method Call */
    msg = dbus_message_new_method_call("org.wuhan.server",
                 "/org/wuhan/Object", "org.wuhan.interface", "method_test");
    if (NULL == msg) {
        fprintf(stderr, "dbus_message_new_method_call Error\n");
        dbus_connection_unref(conn);
        return -4;
    }
    /* Append argument to Method Call*/
    //strcpy(value, "SIGTEST");
#if 1
    dbus_message_iter_init_append(msg, &arg);
    if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_STRING, &value)) {
        fprintf(stderr, "dbus_message_iter_append_basic: Out of Memory\n");
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -5;
    }
#else
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "dbus_message_append_args: Out of Memory\n");
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -5;
    }
#endif

#if 0
    /* Send message and flush the connection */
    if (!dbus_connection_send(conn, msg, NULL)) {
        fprintf(stderr, "dbus_connection_send: Out of Memory\n");
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -6;
    }
    dbus_connection_flush(conn);
    fprintf(stdout, "Method Call with value %s OK\n", value);
#else
    /* Send message and get a hand for reply */
	/* We can also use dbus_connection_send_with_reply_and_block() for blocking */
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
        fprintf(stderr, "dbus_connection_send: Out of Memory\n");
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -6;
    }
    if (NULL == pending) { 
        fprintf(stderr, "Pending Call Null\n"); 
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -7;
    }
    dbus_connection_flush(conn);
    fprintf(stdout, "Method Call with value %s OK\n", value);
    dbus_message_unref(msg);
   
    /* block until we recieve a reply */
    dbus_pending_call_block(pending);

    /* get the reply message */
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        dbus_connection_unref(conn);
        return -8; 
    }
    /* free the pending message handle */
    dbus_pending_call_unref(pending);

    /* read the parameters */
    if (!dbus_message_iter_init(msg, &arg))
        fprintf(stderr, "Message has no arguments!\n"); 
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&arg)) 
        fprintf(stderr, "Argument is not string!\n"); 
    else
        dbus_message_iter_get_basic(&arg, &replyvalue);

#if 0
    /* Only if we have more than one arg... */
    if (!dbus_message_iter_next(&arg))
        fprintf(stderr, "Message has too few arguments!/n"); 
    else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&arg)) 
        fprintf(stderr, "Argument is not int!\n"); 
    else
        dbus_message_iter_get_basic(&arg, &value_int);
#endif
    printf("Got Method Call Reply: %s\n", replyvalue);
   
#endif
    /* free message and connection */
    if (msg)
        dbus_message_unref(msg);
    dbus_connection_unref(conn);
}

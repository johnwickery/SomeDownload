#include <dbus/dbus.h>
#include <stdio.h>

void reply_to_method_call(DBusMessage* msg, DBusConnection* conn)
{
   DBusMessage* reply;
   DBusMessageIter args;
   char *value = "method_test_value";
   char *param = "";

   /* read the arguments */
   if (!dbus_message_iter_init(msg, &args))
      fprintf(stderr, "Message has no arguments!\n"); 
   else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) 
      fprintf(stderr, "Argument is not string!\n"); 
   else 
      dbus_message_iter_get_basic(&args, &param);

   fprintf(stdout, "Method called with arg %s\n", param);

   /* create a reply from the message */
   reply = dbus_message_new_method_return(msg);

   /* add the arguments to the reply */
   dbus_message_iter_init_append(reply, &args);
   if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &value)) { 
      fprintf(stderr, "Out Of Memory!\n"); 
      return;
   }

   /* send the reply && flush the connection */
   if (!dbus_connection_send(conn, reply, NULL)) {
      fprintf(stderr, "Out Of Memory!\n"); 
      return;
   }
   dbus_connection_flush(conn);

   // free the reply
   dbus_message_unref(reply);
}

int main(int argc, char **argv)
{
    DBusError err;	
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessageIter arg;
    char *value = NULL;
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
    ret = dbus_bus_request_name(conn, "org.wuhan.server", 
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

    /* Waiting for method call */
    while(1) {
        /* Block(-1) until connection is readable or writable */
        /* Seems that dbus_connection_read_write() works bad*/
        dbus_connection_read_write_dispatch(conn, -1);
        msg = dbus_connection_pop_message(conn);
        if (msg == NULL) {
            fprintf(stderr, "Receive MethodCall Error\n");
            continue;
        }

        /* Check whether its the signal we are waiting for */
        if (dbus_message_is_method_call(msg, "org.wuhan.interface", "method_test")) {
            if(strcmp(dbus_message_get_path(msg), "/org/wuhan/Object") == NULL)
                reply_to_method_call(msg, conn);
        }
        if (msg)
            dbus_message_unref(msg);
    }

    /* free message and connection */
    if (msg)
        dbus_message_unref(msg);
    dbus_connection_unref(conn);
}


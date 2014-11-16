#include <dbus/dbus.h>
#include <stdio.h>

static DBusHandlerResult filter_func (DBusConnection *connection, 
            DBusMessage *msg, void *user_data)  
{  
    DBusHandlerResult ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;  
    char *value = NULL;  
  
    /* Check whether its the signal we are waiting for */
    if (dbus_message_is_signal(msg, "org.wuhan.interface", "signal_test")){
        DBusError err;  
        dbus_error_init (&err);  
        dbus_message_get_args (msg, &err, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID);  
        if (dbus_error_is_set (&err)) {  
            fprintf(stderr, "Error getting msg args: %s", err.message);  
            dbus_error_free (&err);  
        } else {  
            fprintf(stdout, "Received signal with value %s\n", value);
            ret = DBUS_HANDLER_RESULT_HANDLED;  
        }
    }

    return ret;  
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
    ret = dbus_bus_request_name(conn, "org.wuhan.receiver", 
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

    /* Add filter function */
    if (!dbus_connection_add_filter(conn, filter_func, NULL, NULL))  
        return -5;

    /* Add rule for receive signal */
    dbus_bus_add_match(conn, "type='signal', interface='org.wuhan.interface'", &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "dbus_bus_add_match: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return -4;
    }

    /* Loop */
    while (dbus_connection_read_write_dispatch(conn, -1))  
        ;

    /* free message and connection */
    dbus_message_unref(msg);
    dbus_connection_unref(conn);
}

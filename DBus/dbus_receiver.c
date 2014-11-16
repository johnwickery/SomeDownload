#include <dbus/dbus.h>
#include <stdio.h>

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

    /* Add rule for receive signal */
    dbus_bus_add_match(conn, "type='signal', interface='org.wuhan.interface'", &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "dbus_bus_add_match: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return -4;
    }

    /* Receive signale */
    while(1) {
        /* Block(-1) until connection is readable or writable */
        /* Seems that dbus_connection_read_write() works bad*/
        dbus_connection_read_write_dispatch(conn, -1);
        msg = dbus_connection_pop_message(conn);
        if (msg == NULL) {
            fprintf(stderr, "Receive signal Error\n");
            continue;
        }

        /* Check whether its the signal we are waiting for */
        if (dbus_message_is_signal(msg, "org.wuhan.interface", "signal_test")){
            if (!dbus_message_iter_init(msg, &arg)) {
                fprintf(stderr, "No arg for message\n");
            }
            else if (dbus_message_iter_get_arg_type(&arg) != DBUS_TYPE_STRING) {
                fprintf(stderr, "Error arg type\n");
            } else {
                dbus_message_iter_get_basic(&arg, &value);
                fprintf(stdout, "Receive signal with value %s\n", value);
            }
        }
        dbus_message_unref(msg);
    }

    /* free connection */
    //dbus_message_unref(msg);
    dbus_connection_unref(conn);
}

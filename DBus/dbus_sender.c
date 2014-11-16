#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    DBusError err;
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessageIter arg;
    //char value[64] = {0};
    char *value = "SIGTEST";
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
    ret = dbus_bus_request_name(conn, "org.wuhan.sender", 
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

    /* Create Signal */
    msg = dbus_message_new_signal("/org/wuhan/sender", 
                "org.wuhan.interface", "signal_test");
    if (NULL == msg) {
        fprintf(stderr, "dbus_message_new_signal Error\n");
        dbus_connection_unref(conn);
        return -4;
    }
    /* Append argument to signal*/
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

    /* Send message and flush the connection */
    if (!dbus_connection_send(conn, msg, NULL)) {
        fprintf(stderr, "dbus_connection_send: Out of Memory\n");
        dbus_message_unref(msg);
        dbus_connection_unref(conn);
        return -6;
    }
    dbus_connection_flush(conn);
    fprintf(stdout, "Send signal with value %s OK\n", value);

    /* free message and connection */
    dbus_message_unref(msg);
    dbus_connection_unref(conn);
}

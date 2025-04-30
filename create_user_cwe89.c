#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <postgresql/libpq-fe.h>  // PostgreSQL
#include <sqlite3.h>    // SQLite
#include <sql.h>        // ODBC
#include <sqlext.h>     // ODBC
#include <mysql.h>      // MySQL

#define PORT 8080
#define BUFFER_SIZE 100

// MySQL 
void mysql_query_unsafe(const char* userInput) {
    MYSQL *conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, "localhost", "testuser", "Password90!", "testdb", 0, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", userInput);
    
    
    if (mysql_query(conn, query) != 0) {
        fprintf(stderr, "mysql_query failed: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != NULL) {
                printf("ID: %s, Username: %s, Email: %s\n", row[0], row[1], row[2]);
            }
            mysql_free_result(result);
        }
    }
    
    mysql_close(conn);
}

// PostgreSQL 
void postgres_query_unsafe(const char* userInput) {
    PGconn *conn = PQconnectdb("dbname=testdb user=testuser password=Password90! host=localhost");
    
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", userInput);
    
    PGresult *result = PQexec(conn, query);
    
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            printf("ID: %s, Username: %s, Email: %s\n",
                   PQgetvalue(result, i, 0),
                   PQgetvalue(result, i, 1),
                   PQgetvalue(result, i, 2));
        }
    }
    
    PQclear(result);
    PQfinish(conn);
}

// SQLite 
void sqlite_query_unsafe(const char* userInput) {
    sqlite3 *db;
    char *errMsg = 0;
    
    if (sqlite3_open("test.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", userInput);
    
    if (sqlite3_exec(db, query, 0, 0, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

// ODBC 
void odbc_query_unsafe(const char* userInput) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;
    
    // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    
    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    
    // Connect to database
    SQLCHAR connStr[] = "DSN=TestDB;UID=testuser;PWD=Password90!";
    ret = SQLDriverConnect(dbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    
    if (!SQL_SUCCEEDED(ret)) {
        fprintf(stderr, "Failed to connect to database\n");
        goto cleanup;
    }
    
    // Allocate statement handle
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", userInput);
    
    ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    
    if (SQL_SUCCEEDED(ret)) {
        SQLCHAR id[50], username[50], email[50];
        SQLLEN idLen, usernameLen, emailLen;
        
        while (SQL_SUCCEEDED(SQLFetch(stmt))) {
            SQLGetData(stmt, 1, SQL_C_CHAR, id, sizeof(id), &idLen);
            SQLGetData(stmt, 2, SQL_C_CHAR, username, sizeof(username), &usernameLen);
            SQLGetData(stmt, 3, SQL_C_CHAR, email, sizeof(email), &emailLen);
            
            printf("ID: %s, Username: %s, Email: %s\n", id, username, email);
        }
    }
    
cleanup:
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[BUFFER_SIZE] = {0};

    // Setup the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) exit(EXIT_FAILURE);

    // Read input from the socket
    ssize_t n = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (n < 0) exit(EXIT_FAILURE);
    buffer[n] = '\0';

    close(client_fd);
    close(server_fd);

    // Test all database implementations
    mysql_query_unsafe(buffer);
    postgres_query_unsafe(buffer);
    sqlite_query_unsafe(buffer);
    odbc_query_unsafe(buffer);

    return 0;
}

/*
Compilation command:
gcc create_user_cwe89.c -o create_user_cwe89 -I/usr/include/postgresql -I/usr/include/mysql -lmysqlclient -lpq -lsqlite3 -lodbc
*/
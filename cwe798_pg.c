#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

int main() {
    const char* conninfo = "host=127.0.0.1 dbname=postgres user=admin password=uW61<l2BSnky!";

    printf("Connecting to PostgreSQL...\n");
    // SINK CWE 798
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to PostgreSQL failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    printf("Connected to PostgreSQL!\n");

    const char* query = "SELECT version();";
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // Print the result
    int nrows = PQntuples(res);
    for (int i = 0; i < nrows; i++) {
        printf("PostgreSQL version: %s\n", PQgetvalue(res, i, 0));
    }

    // Cleanup
    PQclear(res);
    PQfinish(conn);

    return 0;
}

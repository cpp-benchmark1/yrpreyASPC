#include <stdio.h>
#include <stdlib.h>
#include <cassandra.h>

int main() {
    const char* username = "admin";
    const char* password = "uW61<l2BSnky!";

    printf("Connecting to Cassandra cluster...\n");

    CassCluster* cluster = cass_cluster_new();
    CassSession* session = cass_session_new();

    cass_cluster_set_contact_points(cluster, "127.0.0.1");
    // SINK CWE 798
    cass_cluster_set_credentials(cluster, username, password);

    // cluster connection
    CassFuture* connect_future = cass_session_connect(session, cluster);
    CassError rc = cass_future_error_code(connect_future);

    if (rc != CASS_OK) {
        fprintf(stderr, "Failed to connect: %s\n", cass_error_desc(rc));
        cass_future_free(connect_future);
        cass_session_free(session);
        cass_cluster_free(cluster);
        return 1;
    }

    printf("Connected to Cassandra with hard-coded credentials!\n");
    
    const char* query = "SELECT release_version FROM system.local;";
    CassStatement* statement = cass_statement_new(query, 0);
    CassFuture* result_future = cass_session_execute(session, statement);

    rc = cass_future_error_code(result_future);
    if (rc != CASS_OK) {
        fprintf(stderr, "Query failed: %s\n", cass_error_desc(rc));
    } else {
        const CassResult* result = cass_future_get_result(result_future);
        const CassRow* row = cass_result_first_row(result);

        if (row) {
            const CassValue* value = cass_row_get_column_by_name(row, "release_version");
            const char* release_version;
            size_t release_version_length;
            cass_value_get_string(value, &release_version, &release_version_length);
            printf("Cassandra release version: %.*s\n", (int)release_version_length, release_version);
        }

        cass_result_free(result);
    }

    // Cleanup
    cass_future_free(result_future);
    cass_statement_free(statement);
    cass_future_free(connect_future);
    cass_session_free(session);
    cass_cluster_free(cluster);

    return 0;
}

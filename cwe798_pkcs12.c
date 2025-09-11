#include <stdio.h>
#include <openssl/pkcs12.h>
#include <openssl/pem.h>

int main() {
    const char* pkcs12_file = "/etc/ssl/certificates/client.p12";
    const char* passphrase  = "4kVI!7!2MbOj!";

    FILE* fp = fopen(pkcs12_file, "rb");
    if (!fp) {
        perror("Failed to open PKCS12 file");
        return 1;
    }

    PKCS12* p12 = d2i_PKCS12_fp(fp, NULL);
    fclose(fp);
    if (!p12) {
        fprintf(stderr, "Failed to read PKCS12 file\n");
        return 1;
    }

    EVP_PKEY* pkey = NULL;
    X509* cert = NULL;

    // SINK CWE 798
    if (!PKCS12_parse(p12, passphrase, &pkey, &cert, NULL)) {
        fprintf(stderr, "Failed to parse PKCS12\n");
        PKCS12_free(p12);
        return 1;
    }

    printf("Parsed PKCS12!\n");

    // Hard-coded output paths
    FILE* key_fp  = fopen("/tmp/extracted_key.pem", "w");
    FILE* cert_fp = fopen("/tmp/extracted_cert.pem", "w");

    if (key_fp)  { PEM_write_PrivateKey(key_fp, pkey, NULL, NULL, 0, NULL, NULL); fclose(key_fp); }
    if (cert_fp) { PEM_write_X509(cert_fp, cert); fclose(cert_fp); }

    EVP_PKEY_free(pkey);
    X509_free(cert);
    PKCS12_free(p12);

    return 0;
}

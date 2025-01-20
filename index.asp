<% 
    If Request.Form("submit") <> "" Then 
        Dim username, email, telephone
        username = Request.Form("username")
        email = Request.Form("email")
        telephone = Request.Form("telephone")
        
        ' Chama o programa.exe com o username de usuário como argumento
        Dim shell
        Set shell = Server.CreateObject("WScript.Shell")
        shell.Run "programa.exe " & username, 0, True
        
        ' Exibe a mensagem de sucesso
        Response.Write "username de usuário " & username & " criado com sucesso!"
    Else 
%>
<html>
<head>
    <title>Adicionar Usuário</title>
</head>
<body>
    <h1>Adicionar Usuário</h1>
    <form method="POST" action="formulario.asp">
        <label for="username">username:</label><br>
        <input type="text" id="username" name="username" required><br>
        <label for="email">Email:</label><br>
        <input type="email" id="email" name="email" required><br>
        <label for="telephone">telephone:</label><br>
        <input type="text" id="telephone" name="telephone" required><br><br>
        <input type="submit" name="submit" value="Adicionar">
    </form>
</body>
</html>
<%
    End If
%>

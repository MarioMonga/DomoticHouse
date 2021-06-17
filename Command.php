<?php
$servername = "localhost";
$username = "software";
$password = "pi";
$dbname = "HomeData";

//launch py
//exec('../Dispathcer.py')

//
//get the command 
$command = filter_input(INPUT_GET, 'command', FILTER_SANITIZE_SPECIAL_CHARS);

//skip i valori vuoti
if (empty($command))
{
	exit;	
}
// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
}

//$sql = "SELECT * FROM Command";
$sql = "INSERT INTO `Command` (`Command`) VALUES (\"".$command."\"); ";

if ($conn->query($sql) !== TRUE) {
  echo "Errore: " . $sql . ", " . $conn->error;
}


$conn->close();

//return value
print($command);
exit;
?> 

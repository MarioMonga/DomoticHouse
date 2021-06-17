<?php
	/*
	ini_set('display_errors', 1);
	ini_set('display_startup_errors', 1);
	error_reporting(E_ALL);
	/**/

	//credenziali db
	$servername = "localhost";
	$username = "software";
	$password = "pi";
	$dbname = "HomeData";
	
	//get the command 
	$subject = filter_input(INPUT_GET, 'subject', FILTER_SANITIZE_SPECIAL_CHARS);
	//skip i valori vuoti
	if (empty($subject))
	{
		echo "0";
		exit;	
	}	
	//connessione al db
	$conn = new mysqli($servername, $username, $password, $dbname);
	
	//verifica la connessione
	if ($conn->connect_error) {
		die("Connection failed: " . $conn->connect_error);
	}
	
	//prende l'ultimo valore (decrescente) con subject definito dalla richiesta
	$sql = "SELECT `id`, `Status` FROM `Status` WHERE `Subject` = \"".$subject."\" ORDER BY `Updated` DESC LIMIT 1;";
	$result = $conn->query($sql);
	if($result)
	{
		//printf("Select returned %d rows.\n", $result->num_rows);
		$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
		$value=$row["Status"];
	}
	else
	{
		$value = 0;
	}
	//return value
	echo $value;
	
	//
	$conn->close();
	exit;
?> 

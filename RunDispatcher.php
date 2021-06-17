<?php
	ini_set('display_errors', 1);
	ini_set('display_startup_errors', 1);
	error_reporting(E_ALL);
	
	//echo $_GET['command'];
	if($_GET['command']== 'run'){
		$command = escapeshellcmd('sudo /home/pi/Esame/script/Dispatcher.py > /home/pi/Esame/script/log.txt');
		$output = shell_exec($command);
		echo "in esecuzione" . $output;
		
	}
	
	else if ($_GET['command']=='kill'){
		$command = escapeshellcmd('sudo pkill -f /home/pi/Esame/script/Dispatcher.py');
		$output = shell_exec($command);
		echo "Terminato" . $output;
		exit;


	}


?>

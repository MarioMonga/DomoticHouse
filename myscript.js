
//
// presentazione
//	
Reveal.initialize({
	hash: true,
	// The "normal" size of the presentation, aspect ratio will be preserved
	// when the presentation is scaled to fit different resolutions
	width: 960,
	height: 700,

	// Factor of the display size that should remain empty around the content
	margin: 0.01,
	pdfSeparateFragments: false 
	
});

//
// carosello
//

//carosello a singola foto	
var owl = $('.carousel1');
var  OpenDoorText = "La porta è aperta";
var  CloseDoorText = "La porta è chiusa";

owl.owlCarousel({
	items:1,
	loop:true,
	margin:10,
	autoplay:true,
	autoplayTimeout:1500,
	autoplayHoverPause:true
});

//carosello a 3 foto
$('.carousel3').owlCarousel({
	items:2,
	loop:true,
	margin:0,
	center:true,
	autoWidth:false,
	dots:0,
	autoplay:true,
	autoplayTimeout:1000,
	autoplayHoverPause:true
});

//invia un comando ad arduino
function SendCommand(command) {
	console.log("commando: "+command);
	$.ajax({
		method: "GET",
		url: "./api/Command.php",
		data: { command: command}
		//dataType: "json"
	})
	.done(function(msg) {
		//alert( "Data Saved: " + msg );
	})
	.fail(function() {
		alert( "Errore durante l'esecuzione del comando!" );
	});
}

//controlla tutti gli stati
function CheckAllStatus() {
	//clearInterval(g_TimerStatus);
	CheckStatus('L');
	CheckStatus('D');
	CheckStatus('T');
	CheckStatus('R');
	CheckStatus('W');
	//g_TimerStatus=setInterval(CheckStatus('L'), 2000);
} 

//controlla tutti gli stati
function CheckStatus(subject) {
	//clearInterval(g_TimerStatus);
	$.ajax({
		method: "GET",
		url: "./api/Read.php",
		data: { subject: subject}
		//dataType: "json"
	})
	.done(function(value) {
		console.log( "Data read: " + value + " subject "+subject);
		if (subject=='L'){
			SetStateLight(value);
		}
		else if (subject=='D'){
			SetStateDoor(value);
		}
		else if (subject=='T'){
			SetTemperature(value);
		}
		else if (subject=='R'){
			SetStateRadiator(value);
		}
		else if (subject=='W'){
			WhoOpenDoor(value);
		}

	})
	.fail(function() {
		console.log( "Errore durante l'esecuzione del comando di lettura" );
	});
	//g_TimerStatus=setInterval(CheckStatus('L'), 2000);
} 
//esegtue l'aggiornamento dello stato della luce
function SetStateLight(value) {
	if(value==1)
	{
		//$('#ImageLight').attr('src', 'assets/img/L_on.png');
		$('#ImageLight').attr('src',function(i,e){
			return e.replace("_off.","_on.");
		})
		$('#TextLight').html("accesa");
	}
	else if(value==0)
	{
		$('#ImageLight').attr('src',function(i,e){
			return e.replace("_on.","_off.");
		})
		$('#TextLight').html("spenta");
	}
} 

//esegue l'aggiornamento dello stato della porta
function SetStateDoor(value) {
	if(value==1)
	{
		//$('#ImageLight').attr('src', 'assets/img/L_on.png');
		$('#ImageDoor').attr('src',function(i,e){
			return e.replace("_off.","_on.");
		})
		$('#TextDoor').html(OpenDoorText);
	}
	else if(value==0)
	{
		$('#ImageDoor').attr('src',function(i,e){
			return e.replace("_on.","_off.");
		})
		$('#TextDoor').html(CloseDoorText);
	}
} 

//esegue l'aggiornamento dello stato della porta
function SetStateRadiator(value) {
	if(value==1)
	{
		$('#ImageTemperature').attr('src', 'assets/img/R_on.png');

		$('#TextTemperature').html("La stufa è accesa");
	}
	else if(value==0)
	{
		$('#ImageTemperature').attr('src', 'assets/img/R_off.png');

		$('#TextTemperature').html("La stufa è spenta");
	}

} 


//aggiorna la temperatura visualizzata
function SetTemperature(value) {
	$('#ValueTemperature').html(value);
} 
function WhoOpenDoor(value){
	if(value == 2){
		OpenDoorText = "Nora ha aperto la porta";
		CloseDoorText = "Nora ha chiuso la porta";
	}
	else if(value == 1){
		OpenDoorText = "Mario ha aperto la porta";
		CloseDoorText = "Mario ha chiuso la porta";
	}
	else if(value == 9){
		CloseDoorText = "<span style=\"color:red\">Qualcuno ha cercato di aprire la porta!</span>";
		OpenDoorText = "<span style=\"color:red\">Qualcuno ha cercato di chiudere la porta!</span>";
	}
	else{
		OpenDoorText = "La porta è aperta";
		CloseDoorText = "La porta è chiusa";

	}
	
}
	
		
//jquery avviato		 
$(document).ready(function() {

	//imposta timer: ogni secondo controlla gli stati
	g_TimerStatus= setInterval(function () {CheckAllStatus()}, 1000);
	
	
	/*==================== Send command: luce,porta,panic ==============================*/
    $(".Command").click(function() {
		var command= $(this).data("command");
		SendCommand(command);
	});

	//evento selettore cambio temperature 
	//$('input[name=myInput]').change(function() 	{ 
	$('#SetTemperature').change(function() 	{ 
		var SetTemp= $(this).val();
		//alert('Changed!' + SetTemp);
		$('#SetTemperatureValue').html(SetTemp+"°C");
		SendCommand("TS"+ SetTemp);
		
	});	
//end ready	
});


/*
    window.addEventListener("load", function() {
      var svgObject = document.getElementById('svg-object').contentDocument;
      var svg = svgObject.getElementById('svg8');
      console.log(svg);
    });
/**/

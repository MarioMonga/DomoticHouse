//==============================================
// Title: Domotic house
// Author: Mario Monga - mario.monga@icparmacentro.it
// Version: 8.1.0
// Created: 1 May 2021
// Updated: 9 June 2021
//===============================================



//
// librerie
//
#include <SPI.h>
#include <RFID.h>
#include <Servo.h> 
#include <math.h>

//
// definizione I/O pin
//
#define LIGHT_DO 		4
#define FAN_DO 			5
#define RADIATOR_DO             6   //PWM~ per effetto fiamma
#define SPEAKER_DO 		7
#define RESET_DIO 		9   //Riservati libreria RFID
#define SDA_DIO   		10  //Riservati libreria RFID
#define BUTTON                  19 //pulsante per check

//
// definizione variabili globali
//
boolean g_bIsRadiatorOn;
boolean g_bIsFanOn;
boolean g_bIsDoorOpen;
boolean g_bIsLightOn;
int 	g_nDegreesLimit;
boolean g_bDebug = false;
int     g_nTimerClock=0;
int     g_nTimerSent=0;
int     g_nWhoOpenDoor;
 
Servo ServoPorta;
RFID RC522(SDA_DIO, RESET_DIO);



//----------------------------------------------
// setup
//
void setup()
{ 
  Serial.begin(9600);
  SPI.begin(); 
  RC522.init();
  //Init DIO
  pinMode(LIGHT_DO, OUTPUT);
  pinMode(FAN_DO, OUTPUT);
  pinMode(SPEAKER_DO, OUTPUT);
  pinMode(RADIATOR_DO, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(6, OUTPUT); //???
  pinMode(A0,INPUT);

  LightOff();
  InitDoor();
  RadiatorOff();
}

// ---------------------------------------------------------
// main loop infinito
//
void loop(){
  //local vars
  String RX_command;
  int nTimer;
  
  //check if TEST system is active
  CheckSystem();
  //controlla messaggi dal server
  RX_command = ReadSerial();
  //if(RX_command.length()>0){Serial.println(RX_command)};
  //
  //gestione della porta
  Door(RX_command);
  //
  //gestione delle luci
  CtrlLight(RX_command);
  //
  //gestione climatizzazione
  CtrlTemperature(RX_command);
  //
  //clock loop
  delay(100);
  //
  g_nTimerClock++;
  //Timer to send
  if(g_nTimerClock > 10){
     //each second send the status
     SendMessage();
     g_nTimerClock=0;
  }

}
// //////////////////////////////////////// funzioni comuni ///////////////////////////////////////////////////////////////7
// ---------------------------------------------------------
//  Function: CheckSystem
//  Brief: esegue un check di tutto il sistema
//  return: void
// 
void CheckSystem(){
  if(digitalRead(BUTTON)== LOW)
  {
    //wait the button restore to HIGH
    while (digitalRead(BUTTON)== LOW){}
    //debug
    if(g_bDebug == true){Serial.println("pulsante premuto");}
    OpenDoor();
    delay(1000);
    CloseDoor();
    LightOn();
    delay(500);
    LightOff();
    CtrlTemperature("TS50");
    delay(3000);
    g_nDegreesLimit = 0;
    OkSound();
  }
  return;
}

// ---------------------------------------------------------
//  Function: SendMessage
//  Brief: compone e invia il messaggio alla seriale
//  return: void
// 
void SendMessage(){
    //stato luce
    String string = "L:";
    if(g_bIsLightOn == true){
      string +="1;";
    }
    else{
      string += "0;";
    }
    
    //stato porta
    string += "D:";
    if(g_bIsDoorOpen==true){
      string +="1;";
    }
    else{
      string += "0;";
    }
    //chi ha comandato la porta con la key
    string += "W:";
    string += g_nWhoOpenDoor;
    string += ";";
    if(g_nTimerSent>0)
    {
      g_nTimerSent--;
      if(g_nTimerSent <=0)
      {
        //reset the sender
        g_nWhoOpenDoor=0;
      }
    }
    
    //stato calorifero
    string += "R:";
    if(g_bIsRadiatorOn==true){
      string +="1;";
    }
    else{
      string += "0;";
    }

    //
    //temperatura attuale
    string += "T:";
    string += ReadTemperature();
    string += ";";
    Serial.println(string);
}

// ---------------------------------------------------------
//  Function: ReadSerial
//  Brief: legge una stringa ricevuta via seriale
//  return: String letta
// 
String ReadSerial(){
  String rx = "";

  while(Serial.available()) {
    rx= Serial.readString();// read the incoming data as string
    //Serial.println(rx);
  }
  Serial.flush();
  /*
  if (Serial.available() >0){
   rx = Serial.read();
   
   }
  */
  return rx;
}

// ---------------------------------------------------------
//  Function: Trace
//  Brief: scrive un tracciamento 
//  
//
void Trace(String sLogString, boolean bNewLine = true)
{
  if(g_bDebug == true)
  {
    if(bNewLine)
      Serial.println(sLogString);
    else
      Serial.print(sLogString);
  }
}


// //////////////////////////////////////// GESTIONE PORTA ///////////////////////////////////////////////////////////////7

// ---------------------------------------------------------
//  Function: InitDoor
//  Brief:  inizializza il ServoPorta
//  return: none
//
void InitDoor(){
  ServoPorta.attach(3);
  //chiude la porta all'avvio
  CloseDoor();
}

// ---------------------------------------------------------
//  Function: OpenDoor
//  Brief:  apre la porta
//  return: none
//
void OpenDoor(){
  ServoPorta.write(177);
  g_bIsDoorOpen = true;
}

// ---------------------------------------------------------
//  Function: CloseDoor
//  Brief:  chiude la porta
//  return: none
//
void CloseDoor(){
  ServoPorta.write(66);
  g_bIsDoorOpen = false;
}

// ---------------------------------------------------------
//  Function: Door
//  Brief:  CHE APRE AL PORTA ALLA LETTURA DI UN TAG
//  return: none
//
void Door(String command)
{
  //Serial.println("entra nella funzione Door");
  //local vars
  String CodeRead;

  //check if the card is present
  if(RC522.isCard()) 
  {
    CodeRead = GetCardCode();
    if( CheckCode(CodeRead) == true )
    {
      //Serial.println("CORRETTO " + CodeRead);
      OkSound();
      //se è aperta...
      if (g_bIsDoorOpen == true){
        CloseDoor();
      }
      else {
        OpenDoor();
      }
    }
    else
    {
      //Serial.println("ERRATO " + CodeRead);
      ErrorSound();
    }
  }
  //apre tramite comandi
  if(command[0] == 'D')
  {
    switch(command[1])
    {
    //spegni
    case '0':
      CloseDoor();
      break;
      //accendi
    case '1':
      OpenDoor();
      break;
      //flip/flop
    case '2':
      if(g_bIsDoorOpen == true)
      {
        CloseDoor();
      }
      else
      {
        OpenDoor();
      }
      break;
    }

  }

}

// ---------------------------------------------------------
//  Function: GetCardCode
//  Brief:  legge il serial number della crate
//  return: String serial number letto
//
String GetCardCode(){

  byte i;
  String CodeRead;
  //if (RC522.isCard())
  {
    // Viene letto il suo codice 
    RC522.readCardSerial();
    String CodeRead ="";
    //Serial.println("Codice delle tessera letto:");

    // Viene caricato il codice della tessera, all'interno di una Stringa
    for(i = 0; i <= 4; i++)
    {
      CodeRead+= String (RC522.serNum[i],HEX);
      CodeRead.toUpperCase();
    }
    return CodeRead;
    //Serial.println(CodeRead);
  }
}
// ---------------------------------------------------------
//  Function: CheckCode
//  Brief: 
//  param: CodeRead: codice letto da esaminare 
//  return: TRUE se identficato altrimenti FALSE  
//
boolean CheckCode(String CodeRead){

  //carica il timer per far persistere lo stato di WHO a 3 secondi (invii)
  g_nTimerSent = 3;
  if( CodeRead == "E49FB2A3C"){
    //Serial.println("codice di nora");
    g_nWhoOpenDoor = 2;
    return true;

    //carta di nora
  }
  else if(CodeRead == "638A213EF6"){
    //Serial.println("codice di mario");
    g_nWhoOpenDoor = 1;
    return true;
  }
  else{
    //Serial.println("codice non accettato");
    g_nWhoOpenDoor = 9;
    return false;
  }
}


//  Function: GetStateDoor
//   return: lo stato della porta 
//
boolean GetStateDoor(){
  return g_bIsDoorOpen;
}


// //////////////////////////////////////////SUONI////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------
//  Function: Error Sound
//   riproduce un suono d'errore
//
void ErrorSound(){
  digitalWrite(SPEAKER_DO, HIGH);
  delay(800);
  digitalWrite(SPEAKER_DO, LOW);
  return;
}

//--------------------------------------------------------
// Function: OkSound
// riproduce un suono di conferma
//
//stato
void OkSound(){
  digitalWrite(SPEAKER_DO, HIGH);
  delay(100);
  digitalWrite(SPEAKER_DO, LOW);
  delay(100);
  digitalWrite(SPEAKER_DO, HIGH);
  delay(150);
  digitalWrite(SPEAKER_DO, LOW);
  return;
}

// //////////////////////////////////////// GESTIONE LUCI ///////////////////////////////////////////////////////////////7



//-----------------------------------------------------
// function: LightOn
// accende le luci
//
void LightOn() {
  digitalWrite(LIGHT_DO, HIGH);
  g_bIsLightOn = true;
  return;
}

//-------------------------------------------------------
// function: LightOn
// accende le luci
//
void LightOff() {
  digitalWrite(LIGHT_DO, LOW);
  g_bIsLightOn = false;
  return;

}

// ---------------------------------------------------------
//  Function: Light
//  Brief:  accende le luci attraverso un comando
//  return: none
//
void CtrlLight(String command){
  
  //controlla se il comando e' relativo alle luci
  if(command[0] == 'L')
  {
    switch(command[1])
    {
      //spegni
    case '0':
      LightOff();
      break;
      //accendi
    case '1':
      LightOn();
      break;
      //flip/flop
    case '2':
      if(g_bIsLightOn == true){
        LightOff();
      }
      else{
        LightOn();
      }      
      break;
    }
  }

}

// ////////////////////////////////////CLIMATIZZAZIONE/////////////////////////////////

//-----------------------------------------------
//Function: RadiatorON
// accende la stufa con effetto fiamma
//
void RadiatorOn(){
  //digitalWrite(RADIATOR_DO,HIGH);
  int nValue = random(100,255);//random(150)+140; 
  analogWrite(RADIATOR_DO, nValue ); 
  //Trace("Fiamma valore:" + nValue);
  g_bIsRadiatorOn = true;
}
//-----------------------------------------------
//Function: RadiatorOff
// Spegne la stufa
//
void RadiatorOff(){
  digitalWrite(RADIATOR_DO,LOW); 
  g_bIsRadiatorOn = false;
}

//-----------------------------------------------
//Function: RadiatorON
// accende la stufa con effetto fiamma
//
void FanOn(){
  digitalWrite(FAN_DO,HIGH); 
  g_bIsFanOn = true;

}

//-----------------------------------------------
//Function: RadiatorON
// accende la stufa con effetto fiamma
//
void FanOff(){
  digitalWrite(FAN_DO,LOW); 
  g_bIsFanOn = false;

}

//----------------------------
//Function: ReadTemperature
//legge la temperatura dell'ambiente e la imposta a una variabile

int ReadTemperature (){
  double Temp;
  Temp = log(10000.0*((1024.0/analogRead(A0)-1)));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  Temp = Temp- 273.15;
  return Temp;
// risultato in gradi:
//int(CalcTemperature(analogRead(A0)))
}

//----------------------------
//Function CtrlTemperature
// Imposta i Gradi da seriale
void CtrlTemperature(String command){
    //local vars
    String Value = "";
    
    if(g_bDebug == true){Serial.println("--------------------------------------------------");}
    if(g_bDebug == true){Serial.print("TemperaturaAmbiente: ");}
    if(g_bDebug == true){Serial.println(int(ReadTemperature()));}
    if(g_bDebug == true){Serial.print("Temperatura impostata: ");}
    if(g_bDebug == true){Serial.println(g_nDegreesLimit);}
    //String sLog;
    //sLog= "Temperatura impostata: " + g_nDegreesLimit;
    //if(g_bDebug == true){Serial.println(sLog);}
    //Trace("Temperatura impostata: " + g_nDegreesLimit);

    
    //verifica se il comando e' per questo destinatario "temperature" (primo byte)
    if (command[0] == 'T'){
      //local vars
      //byte aa = command[2];
      //byte bb = command[3];
      //verifica l'azione da svolgere (secondo byte)
      switch(command[1]){
        case 'S': //S => soglia temperatura
          //leggo la temperatura scritta su due char
          Value += command[2] ; 
          Value += command[3] ; 
          //Serial.println("2 CHAR: " + Value);
          //converto la stringa in un valore decimale
          g_nDegreesLimit = Value.toInt();
          //Serial.println(g_nDegreesLimit, DEC);
          break;
        //case '0':
          }
    }
    
    if (int(ReadTemperature()) > g_nDegreesLimit){
      FanOn();
      RadiatorOff();
      if(g_bDebug == true){Serial.println("si accende il ventilatore");}
    }
    else if (int(ReadTemperature()) < g_nDegreesLimit){
      RadiatorOn();
      FanOff();
      if(g_bDebug == true) {Serial.println("si accende la stufa");}

    }
    else{
      RadiatorOff();
      FanOff();
      if(g_bDebug == true){Serial.println("non si accende ne la stufa ne il ventilatore");}

    }
    if(g_bDebug == true){Serial.println("--------------------------------------------------");}

}

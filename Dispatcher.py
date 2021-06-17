#!/usr/bin/env python
# -*- coding: utf-8 -*-

#File name: Dispatcher.py
#Author: Mario Monga - mario.superlativo@gmail.com
#Create: 29-5-2021
#LastModified: 06-06-2021
#Brief: riceve i messagi da arduino e li carica nel db, legge i messaggi del db e li inoltra ad arduino
#Example:
#License: GPL3



import serial
import sys
import glob
import mysql.connector
import StringIO
import time

"""

"""
#('/dev/tty[A-Za-z]*') per trovare tutte le porte non solo quelle USB
def trovaPorta():
  ports = glob.glob('/dev/tty*')
  result = []
  for port in ports:
   try:
      ser=serial.Serial(port)
      ser.close()
      result.append(port)
   except (serial.SerialException):
      pass
   return port
   #print(port)
     

#funzione per inviare il comando via seriale
def SendCommand(Command):
    ser = serial.Serial('/dev/ttyACM0',9600)
    #print("Send command: " +Command)
    ser.write(Command)
    ser.close()
    
#funzione che decodifica il messaggio e ritorna l'sql per l'inserimento nel DB
def DecodeRxMessage(StringReceived):
	sql = ""
	sqlValues = ""
	#print("Messaggio intero da decodificare: " +StringReceived)
	Messages = StringReceived.split(";")
	for Message in Messages:
		#print("Messaggio da decodificare: " +Message)
		#controllo validita': il messaggio non sia vuoto e che la prima lettera sia tra A e Z
		if Message and Message[0] >= 'A' and Message[0] <= 'Z':
			Status=Message.split(":",2)
			if Status and len(Status)==2 and Status[0][0] >= 'A' and Status[0][0] <= 'Z':
				if sqlValues:
					sqlValues+= ","
				sqlValues+="('"+Status[0]+"',"+Status[1]+")"
				#print("values: " +sqlValues)
	if sqlValues:
		sql="INSERT INTO Status (Subject, Status) VALUES "+sqlValues+";"
		#print("Sql composto: " +sql)
	return sql
	

#def main():
if __name__ == "__main__":
	#print(trovaPorta())
	ser = serial.Serial('/dev/ttyACM0',9600)
	conn = mysql.connector.connect(user='software',password='pi',host='localhost',database='HomeData',charset='utf8',autocommit=True)
	while True:
		try:
			#leggoi i messagge da srduino
			RX_message= ser.readline()
			if RX_message:
				cursor = conn.cursor(buffered=True)
				#print(RX_message)
				#print("ricerca...")
				sql= DecodeRxMessage(RX_message)
				if sql:
					#print("SQL status: " + sql)
					cursor.execute(sql)
					print("sql:"+sql)
				# legge il comando più vecchio da inviare a HOME
				cursor.execute("SELECT `id`,`Command` FROM `Command` WHERE `Receiver` = 'HOME' ORDER BY `id` LIMIT 1;")
				if cursor.rowcount > 0:
					#prendo record
					record=cursor.fetchone()
					data_id = record[0]
					data_command = record[1]
					#print("command found! id: " + str(data_id) + "  command: " + str(data_command))
					#invia comando
					SendCommand(data_command.encode('ascii'))
					print("Send command:" + str(data_command))
					#cancella il record usato
					cursor.execute("DELETE FROM `Command` WHERE `id` = '"+str(data_id)+"';")
					#conn.commit();

		except Exception: # Catch exception which will be raise in connection loss
			print("Eccezione...")
			conn = mysql.connector.connect(user='software',password='pi',host='localhost',database='HomeData',charset='utf8',autocommit=True)
			cursor = conn.cursor(buffered=True)
			ser.close() 
			ser = serial.Serial('/dev/ttyACM0',9600)
		finally:
			cursor.close()
		#ritardo
		time.sleep(0.2)
		#print("sleep")
	# Close the connection
	conn.close()
	ser.close()   
	
	




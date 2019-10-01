
/**
*  B R A C C I O    R O B O T I C O 
*
*  Creato da : Alessandro Manfredi 
*
*  T E S I N A   E S A M E   D I   M A T U R I T A
**/

#include "Arduino.h"
#include <Wire.h>
#include <SoftwareSerial.h>

const byte SPALLA_AVANTI = 'c';
const byte SPALLA_INDIETRO = 'd';
const byte GOMITO_AVANTI = 'e';
const byte GOMITO_INDIETRO = 'p';
const byte POLSO_AVANTI = 'h';
const byte POLSO_INDIETRO = 'l';
const byte MANO_APRI = 'm';
const byte MANO_CHIUDI = 'n';
const byte MOVIMENTO_AUTOMATICO = 'X';
const byte RX_PIN_JOISTIC = 2;
const byte TX_PIN_JOISTIC = 13;
const byte RX_PIN_TELECAMERA = 10;
const byte TX_PIN_TELECAMERA = 5;
const byte BUZZER = A2;

SoftwareSerial Connessione_Joistic(RX_PIN_JOISTIC, TX_PIN_JOISTIC);
SoftwareSerial Connessione_Telecamera(RX_PIN_TELECAMERA, TX_PIN_TELECAMERA);

// CONSTANTI PER LO STATO DEL FUNZIONAMENTO
const byte AUTOMATICO = 'A';
const byte MANUALE = 'M';
byte Funzionamento, Stato;
// COSTANTI PER PORTE MOTORE CANALE A
const int DIREZIONE_A = 12;
const int BRAKE_A = 9;
const int VELOCITA_A = 3;
// CONSTENTE PER LA CONNESSIONE CON LA SCHEDA ARDUINO-SERVO-JOISTIC
const int VELOCITA_MAX = 255;
const int VELOCITA_MIN = 0;
const int VELOCITA_MOVIMENTO = 100;
const int VELOCITA_MOVIMENTO_AUTOMATICO = 60;
const byte MOTORE_AVANTI = 'a';
const byte MOTORE_INDIETRO = 'i';
const byte MOTORE_FERMO = 'f';
const byte CONN_I2C_S_JOISTIC = 4;
const byte THIS_ADRESS = 5;
const byte OTHER_ADRESS = 6;
byte Valore_Connessione;
void Motore_Fermo();
void Motore_Avanti();
void Motore_Indietro();
void Motore_Fermo_Automatico();
void Motore_Avanti_Automatico();
void Motore_Indietro_Automatico();

void setup()
{
  //Settaggi Canale A
  pinMode(DIREZIONE_A, OUTPUT); //Initiates Motor Channel A pin
  pinMode(BRAKE_A, OUTPUT);     //Initiates Brake Channel A pin
  pinMode(VELOCITA_A, OUTPUT);
  //Settaggio PORTA CONNESSIONE CON SCHEDA ARDUINO-SERVO-JOISTCI
  Serial.begin(19200);
  pinMode(RX_PIN_JOISTIC, INPUT);
  pinMode(TX_PIN_JOISTIC, OUTPUT);
  Connessione_Joistic.begin(9600);
  pinMode(RX_PIN_TELECAMERA, INPUT);
  pinMode(TX_PIN_TELECAMERA, OUTPUT);
  Connessione_Telecamera.begin(9600);
  Wire.begin(CONN_I2C_S_JOISTIC);
  Wire.onReceive(receiveEvent);
  Funzionamento = MANUALE;
  Stato = MANUALE;
  pinMode(BUZZER, OUTPUT);
}

// DIREZIONE : HIGH = AVANTI   &&  LOW == INDIETR
void loop()
{
  if (Funzionamento == MANUALE);
  else if (Funzionamento == AUTOMATICO)
  {
    if (Connessione_Telecamera.available())
    {
      Valore_Connessione = Connessione_Telecamera.read();
      Serial.println(Valore_Connessione);

      switch (Valore_Connessione)
      {

        case SPALLA_AVANTI:
        {
          Connessione_Joistic.write(SPALLA_AVANTI);
          break;
        }
        case SPALLA_INDIETRO:
        {
          Connessione_Joistic.write(SPALLA_INDIETRO);
          break;
        }

        case GOMITO_AVANTI:
        {
          Connessione_Joistic.write(GOMITO_AVANTI);
          Serial.println(Valore_Connessione);
          break;
        }
        case GOMITO_INDIETRO:
        {
          Connessione_Joistic.write(GOMITO_INDIETRO);
          Serial.println(Valore_Connessione);
          break;
        }

        case POLSO_AVANTI:
        {
          Connessione_Joistic.write(POLSO_AVANTI);
          break;
        }
        case POLSO_INDIETRO:
        {
          Connessione_Joistic.write(POLSO_INDIETRO);
          break;
        }

        case MANO_APRI:
        {
          Connessione_Joistic.write(MANO_APRI);
          break;
        }
        case MANO_CHIUDI:
        {
          Connessione_Joistic.write(MANO_CHIUDI);
          break;
        }

        case MOTORE_AVANTI:
        {
          Motore_Avanti_Automatico();
          Serial.println(Valore_Connessione);
          break;
        }
        case MOTORE_INDIETRO:
        {
          Motore_Indietro_Automatico();
          break;
        }
        case MOTORE_FERMO:
        {
          Motore_Fermo_Automatico();
          break;
        }

        case MOVIMENTO_AUTOMATICO:
        {
          byte servo_mov = Connessione_Telecamera.read();
          int gradi = Connessione_Telecamera.read();

          Connessione_Joistic.write(MOVIMENTO_AUTOMATICO);
          Connessione_Joistic.write(servo_mov);
          Connessione_Joistic.write(gradi);
        }
      }
    }
    Valore_Connessione = 0;
  }
}

// funzioni per la ricezione e l'invio dei dati tramite protocoloo i2c
void receiveEvent(int data)
{
  // funzione che funziona come un interrupt...ovvero quando vede un carattere in arrivo si richiama automaticamente
  Valore_Connessione = Wire.read();
  if (Valore_Connessione == MANUALE)
  {

    Stato = MANUALE;
    Funzionamento = MANUALE;
    Connessione_Telecamera.write(MANUALE);
    Serial.println("MANUALE");
  }
  if (Valore_Connessione == AUTOMATICO)
  {
    Stato = AUTOMATICO;
    Funzionamento = AUTOMATICO;
    Connessione_Telecamera.write(AUTOMATICO);
    Serial.println("AUTOMATICO");
  }
  if (Valore_Connessione == MOTORE_AVANTI)
    Motore_Avanti();
  else if (Valore_Connessione == MOTORE_INDIETRO)
    Motore_Indietro();
  else if (Valore_Connessione == MOTORE_FERMO)
    Motore_Fermo();
  Serial.println(Valore_Connessione);
}

void Motore_Avanti()
{
  digitalWrite(DIREZIONE_A, HIGH);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MOVIMENTO);
  return;
}

void Motore_Indietro()
{
  digitalWrite(DIREZIONE_A, LOW);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MOVIMENTO);
  return;
}

void Motore_Fermo()
{
  digitalWrite(DIREZIONE_A, LOW);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MIN);
  return;
}

void Motore_Avanti_Automatico()
{
  digitalWrite(DIREZIONE_A, HIGH);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MOVIMENTO_AUTOMATICO);
  return;
}

void Motore_Indietro_Automatico()
{
  digitalWrite(DIREZIONE_A, LOW);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MOVIMENTO_AUTOMATICO);
  return;
}

void Motore_Fermo_Automatico()
{
  digitalWrite(DIREZIONE_A, LOW);
  digitalWrite(BRAKE_A, LOW);
  analogWrite(VELOCITA_A, VELOCITA_MIN);
  return;
}

void Suono()
{
  for (int i = 0; i < 2; i++)
  {
    analogWrite(BUZZER, 255);
    delay(200);
    analogWrite(BUZZER, 0);
    delay(200);
  }

  return;
}

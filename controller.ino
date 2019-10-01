
/**
*  B R A C C I O    R O B O T I C O 
*
*  Creato da : Alessandro Manfredi 
*
*  T E S I N A   E S A M E   D I   M A T U R I T A
**/

#include "Arduino.h"
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Wire.h>

const byte SPALLA_AVANTI = 'c';
const byte SPALLA_INDIETRO = 'd';
const byte GOMITO_AVANTI = 'e';
const byte GOMITO_INDIETRO = 'p';
const byte POLSO_AVANTI = 'h';
const byte POLSO_INDIETRO = 'l';
const byte MANO_APRI = 'm';
const byte MANO_CHIUDI = 'n';
const byte MOVIMENTO_AUTOMATICO = 'X';

const byte RX_PIN = 13;
const byte TX_PIN = 8;
SoftwareSerial Connessione_Motori(RX_PIN, TX_PIN);
Servo Spalla, Gomito, Polso, Mano;

const int X_SOGLIA_ALTA = 550;
const int X_SOGLIA_BASSA = 450;
const int Y_SOGLIA_ALTA = 550;
const int Y_SOGLIA_BASSA = 450;
const byte ANALOGICO_X = A0;
const byte ANALOGICO_Y = A1;
const byte BTN_SU = 4;
const byte BTN_DESTRA = 3;
const byte BTN_SINISTRA = 6;
const byte BTN_GIU = 5;
const byte BTN_ANALOGICO = 2;
const byte BUZZER = A3;
const int PREMUTO = 0;
const int AVANTI = 1;
const int INDIETRO = -1;
const int FERMO = 0;
const int DESTRA = 1;
const int SINISTRA = -1;
const int S_CENTRO = 90;
const int S_DESTRA = 0;
const int S_SINISTRA = 180;
const int MOTORE_AVANTI = 254;
const int MOTORE_INDIETRO = 255;
const byte I2C_MOTORE_AVANTI = 'a';
const byte I2C_MOTORE_INDIETRO = 'i';
const byte I2C_MOTORE_FERMO = 'f';
// COSTANTE PER LA CONNESSIONE I2C
const byte CONN_I2C_S_MOTORI = 4;
const byte THIS_ADRESS = 6;
const byte OTHER_ADRESS = 5;
const byte AUTOMATICO = 'A';
const byte MANUALE = 'M';
byte Funzionamento, Stato;
int x_direzione, y_direzione;
int x_posizione, y_posizione;
int gradi_spalla, gradi_gomito, gradi_polso, gradi_mano;
byte Valore_Connessione;

int regola_gradi(int gradi);
void Suono_Accensione();
void Polso_Avanti();
void Polso_Indietro();
void Spalla_Avanti();
void Spalla_Indietro();
void Gomito_Avanti();
void Gomito_Indetro();
void Mano_Apri();
void Mano_Chiudi();
void Motore_Avanti();
void Motore_Indietro();
void Motore_Fermo();
void Manuale();
void Automatico();
void Suono_Tutto_OK();
void Invia_Stato(byte Stato);
void Resetta_Gradi();
void Muovi_Servi(byte servo_mov, int gradi);
void Muovi_Spalla_Automatico(int gradi);
void Muovi_Gomito_Automatico(int gradi);

void setup()
{
  pinMode(ANALOGICO_X, INPUT);
  pinMode(ANALOGICO_Y, INPUT);
  pinMode(BTN_ANALOGICO, INPUT);
  digitalWrite(BTN_ANALOGICO, HIGH);
  pinMode(BTN_SU, INPUT);
  digitalWrite(BTN_SU, HIGH);
  pinMode(BTN_GIU, INPUT);
  digitalWrite(BTN_GIU, HIGH);
  pinMode(BTN_DESTRA, INPUT);
  digitalWrite(BTN_DESTRA, HIGH);
  pinMode(BTN_SINISTRA, INPUT);
  digitalWrite(BTN_SINISTRA, HIGH);
  // NORMALMENTE NEI PULSANTI è RICHIESTA UNA RESISTENZA PER FARE IN MODO CHE QUANDO NON è PREMUTO IL PULSANTE VENGA RILEVATA TENSIONE ALTA
  //AL CHE QUANDO è PREMUTO BASTA RILEVARE IL LIVELLO BASSO ( 0 ) E ARDUINO QUESTI RESITORI CE LI HA INTEGRATI QUINDI
  //PER ABILITARE LA RESISTENZA DI PULL-UP DI UN PIN SI IMPOSTA IL PIN COME INPUT E POI SI ATTIVA IMPOSTANDO LO STESSO PIN A LIVELLO ALTO ( HIGH )
  // settaggio porte per i servo : OUTPUT TUTTE
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  // impostazioni dei servo: setto le porte ed imposto
  gradi_spalla = gradi_gomito = gradi_polso = S_CENTRO;
  // non metto la mano al centro perchè la mano va chiusa!
  gradi_mano = S_DESTRA;
  Spalla.attach(9);
  Spalla.write(gradi_spalla);
  Gomito.attach(10);
  Gomito.write(gradi_gomito);
  Polso.attach(11);
  Polso.write(gradi_polso);
  Mano.attach(12);
  Mano.write(gradi_mano);
  //settaggio del buzzer sulla porta 7
  pinMode(BUZZER, OUTPUT);
  Serial.begin(19200);
  // inizializzazione de protocollo I2C
  Wire.begin();
  Funzionamento = MANUALE;
  Stato = MANUALE;
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  Connessione_Motori.begin(9600);
  Suono_Accensione();
}

void loop()
{
  if (digitalRead(BTN_ANALOGICO) == LOW)
  {
    if (Stato == AUTOMATICO)
    {
      Funzionamento = MANUALE;
      Stato = MANUALE;
      Invia_Stato(MANUALE);
      Resetta_Gradi();
      Suono_Tutto_OK();
    }

    else if (Stato == MANUALE)
    {
      Funzionamento = AUTOMATICO;
      Stato = AUTOMATICO;
      // INVIO TRAITE WIRE IL SEGNALE DI STATO DEL FUNZIONAMENTO PER ACCENDERE O SPEGNERE LA YELECAMERE ED EVITARE INTERFERENZE
      Invia_Stato(AUTOMATICO);
      Resetta_Gradi();
      Suono_Tutto_OK();
    }
  }

  if (Funzionamento == MANUALE)
  {
    Manuale();
  }
  else if (Funzionamento == AUTOMATICO)
  {
    Automatico();
  }
  Valore_Connessione = 0;
}

void Automatico()
{
  if (Connessione_Motori.available())
  {

    Valore_Connessione = Connessione_Motori.read();

    Serial.println(Valore_Connessione);

    switch (Valore_Connessione)
    {
    case SPALLA_AVANTI:
    {
      Spalla_Avanti();
      break;
    }
    case SPALLA_INDIETRO:
    {
      Spalla_Indietro();
      break;
    }

    case GOMITO_AVANTI:
    {
      Gomito_Avanti();
      break;
    } 
    case GOMITO_INDIETRO:
    {
      Gomito_Indietro();
      break;
    } 
    case POLSO_AVANTI:
    {
      Polso_Avanti();
      break;
    }
    case POLSO_INDIETRO:
    {
      Polso_Indietro();
      break;
    }

    case MANO_APRI:
    {
      Mano_Apri();
      break;
    }
    case MANO_CHIUDI:
    {
      Mano_Chiudi();
      break;
    }

    case MOVIMENTO_AUTOMATICO:
    {
      byte servo_mov = Connessione_Motori.read();
      int gradi = Connessione_Motori.read();

      Muovi_Servi(servo_mov, gradi);
    }
    }
  }
  Valore_Connessione = 0;
}

void Manuale()
{
  // C O N TR O L L O   D E I   P U L S A N T I   P E R   M O D O A   A U T O M A T I C O
  x_direzione = FERMO;
  y_direzione = FERMO;
  x_posizione = analogRead(ANALOGICO_X);
  y_posizione = analogRead(ANALOGICO_Y);

  //CONTROLLO POSIZIONE
  if (x_posizione > X_SOGLIA_ALTA)
    x_direzione = DESTRA;

  else if (x_posizione < X_SOGLIA_BASSA)
    x_direzione = SINISTRA;

  if (y_posizione > Y_SOGLIA_ALTA)
    y_direzione = AVANTI;

  else if (y_posizione < Y_SOGLIA_BASSA)
    y_direzione = INDIETRO;

  
  // DA QUI GUARDO ANCORA LO STATO DELLE DIREZIONI LE QUALI MI SERVIRANNO ABBINATE ALLA PRESSIONE DEI PULSANTI PER MUOVERE I PULSANTI APPROPIATI
  // UTILILLO UNA PORTA ANALOGICA PER STABILRE UN COLEGAMENTO CON LA SCHEDA ARDUINO-MOTORI
  //  123 MOTORE AVANTI
  //  124 MOTORE INDIETRO

  // C O M A N D I   P E R   I L   P O L S O
  if (x_direzione == DESTRA && y_direzione == AVANTI && digitalRead(BTN_SU) == LOW)
  {
    // POLSO AVANTI DESTRA SU
    Serial.println("DESTRA SU POLSO AVANTI");
    Polso_Avanti();
  }
  if (x_direzione == SINISTRA && y_direzione == AVANTI && digitalRead(BTN_SU) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA SU POLSO AVANTI");
    Polso_Avanti();
  }
  if (x_direzione == DESTRA && y_direzione == INDIETRO && digitalRead(BTN_SU) == LOW)
  {
    // POLSO AVANTI DESTRA GIU
    Serial.println("DESTRA GIU POLSO AVANTI");
    Polso_Indietro();
  }
  if (x_direzione == SINISTRA && y_direzione == INDIETRO && digitalRead(BTN_SU) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA GIU POLSO AVANTI");
    Polso_Indietro();
  }

  if (y_direzione == AVANTI && digitalRead(BTN_SU) == LOW)
  {
    Serial.println("  POLSO AVANTI ");
    Polso_Avanti();
  }

  if (y_direzione == INDIETRO && digitalRead(BTN_SU) == LOW)
  {
    Serial.println("  POLSO INDIETRO ");
    Polso_Indietro();
  }

  // C O M A N D I   P E R   I L   G O M I T O
  if (x_direzione == DESTRA && y_direzione == AVANTI && digitalRead(BTN_DESTRA) == LOW)
  {
    Serial.println("DESTRA SU GOMITO AVANTI");
    Gomito_Avanti();
  }

  if (x_direzione == SINISTRA && y_direzione == AVANTI && digitalRead(BTN_DESTRA) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA SU GOMITO AVANTI");
    Gomito_Avanti();
  }

  if (x_direzione == DESTRA && y_direzione == INDIETRO && digitalRead(BTN_DESTRA) == LOW)
  {
    // POLSO AVANTI DESTRA GIU
    Serial.println("DESTRA GIU GOMITO AVANTI");
    Gomito_Indietro();
  }

  if (x_direzione == SINISTRA && y_direzione == INDIETRO && digitalRead(BTN_DESTRA) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA GIU GOMITO AVANTI");
    Gomito_Indietro();
  }

  if (y_direzione == AVANTI && digitalRead(BTN_DESTRA) == LOW)
  {
    Serial.println("GOMITO AVANTI");
    Gomito_Avanti();
  }

  if (y_direzione == INDIETRO && digitalRead(BTN_DESTRA) == LOW)
  {
    Serial.println("GOMITO INDIETRO");
    Gomito_Indietro();
  }

  // C O M A N D I   P E R   L A   S P A L L A
  if (x_direzione == DESTRA && y_direzione == AVANTI && digitalRead(BTN_GIU) == LOW)
  {
    // POLSO AVANTI DESTRA SU
    Serial.println("DESTRA SU SPALLA AVANTI");
    Spalla_Avanti();
  }

  if (x_direzione == SINISTRA && y_direzione == AVANTI && digitalRead(BTN_GIU) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA SU SPALLA AVANTI");
    Spalla_Avanti();
  }

  if (x_direzione == DESTRA && y_direzione == INDIETRO && digitalRead(BTN_GIU) == LOW)
  {
    // POLSO AVANTI DESTRA GIU
    Serial.println("DESTRA GIU SPALLA AVANTI");
    Spalla_Indietro();
  }

  if (x_direzione == SINISTRA && y_direzione == INDIETRO && digitalRead(BTN_GIU) == LOW)
  {
    // POLSO AVANTI SINISTRA SU
    Serial.println("SINISTRA GIU SPALLA AVANTI");
    Spalla_Indietro();
  }

  if (y_direzione == AVANTI && digitalRead(BTN_GIU) == LOW)
  {
    Serial.println("SPALLA AVANTI");
    Spalla_Avanti();
  }

  if (y_direzione == INDIETRO && digitalRead(BTN_GIU) == LOW)
  {
    Serial.println("SPALLA INDIETRO");
    Spalla_Indietro();
  }

  // C O M A N D I   P E R   L A   M A N O  ( A P E R T U R A   D E L L A   P I N Z A )
  if (x_direzione == DESTRA && digitalRead(BTN_SINISTRA) == LOW)
  {
    Serial.println("MANO APRI");
    Mano_Apri();
    return;
  }
  if (x_direzione == SINISTRA && digitalRead(BTN_SINISTRA) == LOW)
  {
    Serial.println("MANO CHIUDI");
    Mano_Chiudi();
    return;
  }

  // C O M A N D I   P E R   I L   M O T O R E
  if (x_direzione == AVANTI)
  {
    // MOTORE AVANTI
    Serial.println("MOTORE AVANTI");
    Motore_Avanti();
  }

  if (x_direzione == INDIETRO)
  {
    //MOTORE INDIETRO
    Serial.println("MOTORE INDIETRO");
    Motore_Indietro();
  }
  if (x_direzione == FERMO)
  {
    Serial.println("MOTORE FERMO");
    Motore_Fermo();
  }
  return;
}

int regola_gradi(int gradi)
{
  if (gradi > S_SINISTRA)
  {
    gradi = S_SINISTRA;
    // Faccio suonoare il buzzer per avvertire della posizione massima raggiunta
    analogWrite(BUZZER, 255);
    delay(50);
    analogWrite(BUZZER, 0);
  }
  if (gradi < S_DESTRA)
  {
    gradi = S_DESTRA;
    // Faccio suonoare il buzzer per avvertire della posizione minima raggiunta
    analogWrite(BUZZER, 255);
    delay(50);
    analogWrite(BUZZER, 0);
  }
  return gradi;
}

void Suono_Accensione()
{
  for (int i = 0; i < 4; i++)
  {
    analogWrite(BUZZER, 255);
    delay(200);
    analogWrite(BUZZER, 0);
    delay(200);
  }
  return;
}

void Suono_Tutto_OK()
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

/* queste funzioni ti tipo void ( senza valore di ritorno ) servono per fare muovere il singolo servo abbinato alla parte del braccio
   al che per le varie combinazioni di pulsanti e movimenti dell'analogico mi basta richiamare le seguenti funzioni.Inoltre queste funzioni contengono
   gia al loro interno il controlla-regola gradi.
   In queste funzioni non ho voluto usare dei parametri di passaggio per complicarmi meno la vita infatti tutte le variabili le ho dichiarate globali per evitare problemi vari vista 
   la compessita ma sopprattutto la lunghezza del programma
   */

void Polso_Avanti()
{
  gradi_polso += 1;
  gradi_polso = regola_gradi(gradi_polso);
  Polso.write(gradi_polso);
  return;
}

void Polso_Indietro()
{
  gradi_polso -= 1;
  gradi_polso = regola_gradi(gradi_polso);
  Polso.write(gradi_polso);
  return;
}

void Spalla_Avanti()
{
  gradi_spalla += 1;
  gradi_spalla = regola_gradi(gradi_spalla);
  Spalla.write(gradi_spalla);
  return;
}

void Spalla_Indietro()
{
  gradi_spalla -= 1;
  gradi_spalla = regola_gradi(gradi_spalla);
  Spalla.write(gradi_spalla);
  return;
}

void Gomito_Avanti()
{
  gradi_gomito += 1;
  gradi_gomito = regola_gradi(gradi_gomito);
  Gomito.write(gradi_gomito);
  return;
}

void Gomito_Indietro()
{
  gradi_gomito -= 1;
  gradi_gomito = regola_gradi(gradi_gomito);
  Gomito.write(gradi_gomito);
  return;
}

void Mano_Chiudi()
{
  gradi_mano -= 5;
  gradi_mano = regola_gradi(gradi_mano);
  Mano.write(gradi_mano);
  return;
}

void Mano_Apri()
{
  gradi_mano += 5;
  gradi_mano = regola_gradi(gradi_mano);
  Mano.write(gradi_mano);

  return;
}

void Motore_Avanti()
{
  Wire.beginTransmission(CONN_I2C_S_MOTORI);
  Wire.write(I2C_MOTORE_AVANTI);
  Wire.endTransmission();
  return;
}

void Motore_Indietro()
{
  Wire.beginTransmission(CONN_I2C_S_MOTORI);
  Wire.write(I2C_MOTORE_INDIETRO);
  Wire.endTransmission();
  return;
}

void Motore_Fermo()
{
  Wire.beginTransmission(CONN_I2C_S_MOTORI);
  Wire.write(I2C_MOTORE_FERMO);
  Wire.endTransmission();
  return;
}

void Invia_Stato(byte Stato)
{
  if (Stato == AUTOMATICO)
  {
    Wire.beginTransmission(CONN_I2C_S_MOTORI);
    Wire.write(AUTOMATICO);
    Wire.endTransmission();
  }
  else if (Stato == MANUALE)
  {
    Wire.beginTransmission(CONN_I2C_S_MOTORI);
    Wire.write(MANUALE);
    Wire.endTransmission();
  }
  return;
}

void Resetta_Gradi()
{
  gradi_spalla = gradi_gomito = gradi_polso = S_CENTRO;
  // non metto la mano al centro perchè la mano va chiusa!
  gradi_mano = 100;
  Gomito.write(gradi_gomito);
  Spalla.write(gradi_spalla);
  Polso.write(gradi_polso);
  Mano.write(gradi_mano);
  return;
}

void Muovi_Servi(byte servo_mov, int gradi)
{
  switch (servo_mov)
  {
  case GOMITO_AVANTI:
    Muovi_Gomito_Automatico(gradi);
  case GOMITO_INDIETRO:
    Muovi_Gomito_Automatico(gradi);
  case SPALLA_AVANTI:
    Muovi_Spalla_Automatico(gradi);
  case SPALLA_INDIETRO:
    Muovi_Spalla_Automatico(gradi);
  }
}

void Muovi_Spalla_Automatico(int gradi)
{
  if (gradi > 180)
    gradi = 180;
  if (gradi < 0)
    gradi = 0;
  Spalla.write(gradi);
}

void Muovi_Gomito_Automatico(int gradi)
{
  if (gradi > 180)
    gradi = 180;
  if (gradi < 0)
    gradi = 0;
  Gomito.write(gradi);
}

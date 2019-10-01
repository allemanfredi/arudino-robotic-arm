/**
*  B R A C C I O    R O B O T I C O 
*
*  Creato da : Alessandro Manfredi 
*
*  T E S I N A   E S A M E   D I   M A T U R I T A
**/

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <CMUcam4.h>
#include <CMUcom4.h>
#include <PID_v1.h>


#define TIME_T 200
#define SUCCESSO 0
#define YUV_MODE true
#define RED_MIN 155
#define RED_MAX 255
#define GREEN_MIN 0
#define GREEN_MAX 255
#define BLUE_MIN 55
#define BLUE_MAX 155
#define AREA 50
#define LED_BLINK 5 // 5 Hz
#define X_SOGLIA_MIN 55
#define X_SOGLIA_MAX 105
#define Y_SOGLIA_MIN 40 //55
#define Y_SOGLIA_MAX 80 // 65
#define SOGLIA_MIN_SENSORE 9
#define SOGLIA_MAX_SENSORE 15
#define TEMPO_SETTAGGIO 5000
#define FILTRO 17

const long int LATO_X_SOGLIA = X_SOGLIA_MAX - X_SOGLIA_MIN;
const long int LATO_Y_SOGLIA = Y_SOGLIA_MAX - Y_SOGLIA_MIN;
const long int AREA_SOGLIA = LATO_X_SOGLIA * LATO_Y_SOGLIA;
const byte GRADI_MOVIMENTO_ANALISI = 2;
const int TEMPO_PAUSA_ANALISI = 500;
const int SOGLIA_GRADI_MOVIMENTO = 70;
boolean Esci;
int gradi_gomito, gradi_spalla;
int timer;
boolean Verifica_Analisi, Verifica_Stato_Analisi;
int conta_gradi;
int Stato_Analisi_PID;
const byte SPALLA_AVANTI = 'c';
const byte SPALLA_INDIETRO = 'd';
const byte GOMITO_AVANTI = 'e';
const byte GOMITO_INDIETRO = 'p';
const byte POLSO_AVANTI = 'h';
const byte POLSO_INDIETRO = 'l';
const byte MANO_APRI = 'm';
const byte MANO_CHIUDI = 'n';
const byte MOVIMENTO_AUTOMATICO = 'X';
const byte RX_PIN_MOTORI = 11;
const byte TX_PIN_MOTORI = 10;

// C M U C A M   4
CMUcam4 Camera(CMUCOM4_SERIAL);
CMUcam4_tracking_data_t data;

SoftwareSerial Connessione_Motori(RX_PIN_MOTORI, TX_PIN_MOTORI);

const byte AUTOMATICO = 'A';
const byte MANUALE = 'M';
byte Funzionamento, Stato;
byte Valore_Connessione;
const byte MOTORE_AVANTI = 'a';
const byte MOTORE_INDIETRO = 'i';
const byte MOTORE_FERMO = 'f';
const byte BUZZER = A3;
const byte SENSORE = A4;
byte Valore_Sensore;
int Lato_x, Lato_y, Area_Analisi;

void Suono();
void Muovi_Motore_x_Analisi(byte Movimento);
void Muovi_Gomito_x_Analisi(byte Movimento, int gradi);
boolean Analisi();
void Automatico();
void Muovi_Servo_Automatico(byte servo_mov, int gradi);
double Mappa_Gradi(double output);
void Muovi_Gomito_x_Analisi_2(byte Movimento);

void setup()
{
  Valore_Sensore = 0;
  pinMode(SENSORE, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RX_PIN_MOTORI, INPUT);
  pinMode(TX_PIN_MOTORI, OUTPUT);
  Connessione_Motori.begin(9600);
  Funzionamento = MANUALE;
  Camera.begin();
  Camera.LEDOn(LED_BLINK);
  delay(TEMPO_SETTAGGIO);
  Camera.autoGainControl(false);
  Camera.autoWhiteBalance(false);
  Camera.colorTracking(YUV_MODE);
  Camera.noiseFilter(FILTRO);
  conta_gradi = 0;
  Camera.LEDOn(CMUCAM4_LED_ON);
}

int x;
void loop()
{
  if (Connessione_Motori.available())
  {
    Valore_Connessione = Connessione_Motori.read();
    //  Serial.println ( Valore_Connessione );

    if (Valore_Connessione == MANUALE)
    {
      Stato = MANUALE;
      Funzionamento = MANUALE;
      Suono();
    }
    else if (Valore_Connessione == AUTOMATICO)
    {
      Stato = AUTOMATICO;
      Funzionamento = AUTOMATICO;
      Suono();
    }
  }

  if (Funzionamento == AUTOMATICO)
  {
    Automatico();
  }

  if (Funzionamento == MANUALE)
    ; // NON SUCCEDE NIENTE..LA TELECAMERA E L'ARDUINO STARANNO IN ATTESA DEL CAMBIO DI STATO

  Valore_Connessione = 0;
}

void Automatico()
{
  Analisi();
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

void Muovi_Motore_x_Analisi(byte Movimento)
{
  Connessione_Motori.write(Movimento);
  return;
}

void Muovi_Gomito_x_Analisi(byte Movimento, int gradi)
{
  for (int i = 0; i < gradi; i++)
  {
    Connessione_Motori.write(Movimento);
    delay(50);
  }
  return;
}

void Muovi_Gomito_x_Analisi_2(byte Movimento)
{
  Connessione_Motori.write(Movimento);
  return;
}

boolean Analisi()
{
  for (;;)
  {

    int Successo = Camera.trackColor(RED_MIN, RED_MAX, GREEN_MIN, GREEN_MAX, BLUE_MIN, BLUE_MAX);
    if (Successo != SUCCESSO)
      return false;

    Camera.getTypeTDataPacket(&data);
    int distanza1, distanza2, distanza3, distanza4;
    if ((data.mx == 0) || (data.my == 0))
      return false;

    if (data.mx < X_SOGLIA_MIN)
    {
      for (; (data.mx < X_SOGLIA_MIN);)
      {
        distanza1 = abs(data.mx - distanza1);
        if (distanza1 > 50)
        {
          Muovi_Motore_x_Analisi(MOTORE_INDIETRO);
          delay(120);
          Muovi_Motore_x_Analisi(MOTORE_FERMO);
        }
        else
        {
          Muovi_Motore_x_Analisi(MOTORE_INDIETRO);
          delay(80);
          Muovi_Motore_x_Analisi(MOTORE_FERMO);
        }
        distanza1 = data.mx;
        Camera.getTypeTDataPacket(&data);
      }
    }

    if (data.mx > X_SOGLIA_MAX)
    {
      for (; (data.mx > X_SOGLIA_MAX);)
      {
        distanza2 = abs(data.mx - distanza2);
        if (distanza2 > 50)
        {
          Muovi_Motore_x_Analisi(MOTORE_AVANTI);
          delay(120);
          Muovi_Motore_x_Analisi(MOTORE_FERMO);
        }
        else
        {
          Muovi_Motore_x_Analisi(MOTORE_AVANTI);
          delay(80);
          Muovi_Motore_x_Analisi(MOTORE_FERMO);
        }
        distanza2 = data.mx;
        Camera.getTypeTDataPacket(&data);
      }
    }

    if (data.my < Y_SOGLIA_MIN)
    {
      for (; data.my < Y_SOGLIA_MIN;)
      {
        distanza3 = abs(data.my - distanza3);
        if (distanza3 > 30)
          Muovi_Gomito_x_Analisi(SPALLA_AVANTI, 1);
        else
          Muovi_Gomito_x_Analisi(GOMITO_AVANTI, 1);
        distanza3 = data.my;
        Camera.getTypeTDataPacket(&data);
      }
    }

    if (data.my > Y_SOGLIA_MAX)
    {
      for (; data.my > Y_SOGLIA_MAX;)
      {
        distanza4 = abs(data.my - distanza4);
        if (distanza4 > 30)
          Muovi_Gomito_x_Analisi(SPALLA_INDIETRO, 1);
        else
          Muovi_Gomito_x_Analisi(GOMITO_INDIETRO, 1);

        distanza4 = data.my;
        Camera.getTypeTDataPacket(&data);
      }
    }
  }
  return false;
}

void Muovi_Servo_Automatico(byte servo_mov, int gradi)
{
  Connessione_Motori.write(MOVIMENTO_AUTOMATICO);
  Connessione_Motori.write(servo_mov);
  Connessione_Motori.write(gradi);
  return;
}

double Mappa_Gradi(double output)
{
  return (output * 180) / 360;
}
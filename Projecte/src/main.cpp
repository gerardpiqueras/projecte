#include <Arduino.h>

// INCLUDES PER L'ALTAVEU
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"

// Funcions pel funcionament de l'altaveu.
AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

// INCLUDES I FUNCIONS PER DISPLAY I SENSOR
void init_temp_hum_task(void);
#include <Wire.h>
#include "SSD1306Wire.h"
#include "images.h"
#include "SparkFunHTU21D.h"


HTU21D myHumidity; // htu21D es el sensor que utilitzem per a calcular la temperatura i l'humitat.
SSD1306Wire display(0x3c, SDA, SCL); // SSD1306Wire display es la pantalla que hem utilitzat.

#define DEMO_DURATION 3000
typedef void (*Demo)(void);

// INCLUDES Y DEFINES PEL CORREU
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#endif
#include <ESP_Mail_Client.h>
#define WIFI_SSID "Grauvi"
#define WIFI_PASSWORD "21222328"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "adriaserran@gmail.com"
#define AUTHOR_PASSWORD "ccvaujlnpttogqrz"
#define RECIPIENT_EMAIL "adriaserran@gmail.com"

// Servidor smtp per poder enviar el correu.
SMTPSession smtp; 

// Funcions per enviar un correu i fer que soni la alarma, estan definides porteriorment.
void correo(); 
void alarma();

int demoMode = 0;
int counter = 1;

// Funció setup(). Nomes s'executarà una vegada.
void setup(){
Serial.begin(115200); // Funció per establir la comunicació amb el port serie.
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out -> SetGain(0.125);
  out -> SetPinout(12,14,32);
  aac->begin(in, out);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  myHumidity.begin();

  display.init();  // Amb la funció display.init inicialitzarem la pantalla.
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

// Funció loop(). S'executarà en bucle moltes vegades.
void loop(){
  float humd = myHumidity.readHumidity(); // definim la funcio humd. LLegeix la humitat del sensor declarat com a myHumidity.
  float temp = myHumidity.readTemperature(); // definim la funcio temp. LLegeix la temperatura del sensor declarat com a myHumidity.
 
  // Fem un print de les coses que volem mostrar per la pantalla respecte a la temperatura i la humitat.
  Serial.print(" Temperatura:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humedad:");
  Serial.print(humd, 1);
  Serial.print("%");

  // Funció per escriure les dades dels anteriors Serial.print() pel port serie.
  Serial.println();

  // Funció per netejar la pantalla.
  display.clear();

  // Definim com volem el text que mostrarem per pantalla. Tipografia, mida, etc.
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(128/2, 0, "HUMEDAD"); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(128/2, 11, String(humd)+ "%");
  display.setFont(ArialMT_Plain_10);
  display.drawString(128/2, 30, "TEMPERATURA");
  display.setFont(ArialMT_Plain_16);
  display.drawString(128/2, 41, String(temp)+ "ºC");

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 54, String(millis()/3600000)+String(":")\
          +String((millis()/60000)%60)+String(":")\
          +String((millis()/1000)%(60)));

  // Mostrem a la nostra pantalla el contigut que li hem assignat anteriorment.
  display.display();
  delay(2000);

  // Començem una funció condicional (if).
  if (temp > 35.0)
  {
     correo(); // Cridem a la funció per enviar el correu quan la temperatura supera el valor indicat.
  }
  // Començem una funció bucle (while).
  while(temp > 35.0)
  {
    alarma(); // Cridem a la funció per fer solar la alarma quan la temperatura supera el valor indicat.         
  }
}

void smtpCallback(SMTP_Status status){ // Funció agafada d'una pàgina web
  
  Serial.println(status.info());

  
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

void correo()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to AP"); // S'està connectant a internet
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected."); // Connectat
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  smtp.debug(1);

  void smtpCallback(SMTP_Status status);
  
  ESP_Mail_Session session;

  
  session.server.host_name = SMTP_HOST; // Totes aquestes dades estan definides al principi del codi
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  
  SMTP_Message message;

  
  message.sender.name = "ESP"; // Nom de la persona que envia el correu
  message.sender.email = AUTHOR_EMAIL; // Des de quin correu s'envia
  message.subject = "L'ALARMA ESTÀ SONANT"; // Quin assumpte tindrà el correu
  message.addRecipient("Adrià", RECIPIENT_EMAIL);

  // El que escriu al "cos" del missatge
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>La temperatura rebuda pel detector és de més de 35ºC i l'alarma està sonant</h1><p>- Enviat per Gerard i Adrià</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))
    return;

  
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error enviant el mail, " + smtp.errorReason());// Acaba enviant missatge i tanca la sessió de l'usuari
}

void alarma()
{
  if(aac->isRunning())
  {
    aac->loop();
  }else {
  // aac -> stop();
  Serial.printf("ALARMA SONANT\n"); // Escriu per pantalla alarma sonant cada vegada que l'altaveu acaba el bucle
  delay(1000);

  // Aquesta part de codi fa que el so no pari de sonar, és a dir, que torni a començar, sino només sonaria un cop
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac->begin(in,out); // Fa que el so torni a començar
  }

}
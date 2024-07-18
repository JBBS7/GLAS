#include <ESP8266WiFi.h>
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial mySerial(1,3); // ({GPIO3}RX=TXD{gsm} , {GPIO1}TX=RXD{gsm})
TinyGPSPlus gps;
SoftwareSerial sgps(0,2); // (D3 - brown - RX, D4 - red - TX)

#define BLYNK_TEMPLATE_ID "TMPLJ0DvWSUE"
#define BLYNK_TEMPLATE_NAME "GLAS" /*GAS LEAK ALERT SYSTEM*/
#define BLYNK_AUTH_TOKEN "jLEL4xCf7HO7aFU-BnGY3Rl65duMxjKC"
#define BLYNK_PRINT Serial

int buzzer = D5;
int gasA0 = A0;
int gasdata = 0;
int led = D7;
int sensorThres = 600;

float lati, longi;

byte Bell[8] = {
0b00100,
0b01110,
0b01110,
0b01110,
0b11111,
0b00000,
0b00100,
0b00000
};
 
String textForSMS;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "****"; //wifi-user name
char pass[] = "******"; //wifi-password
 
SimpleTimer timer;
 
void setup() 
{
 randomSeed(analogRead(0));
 Serial.begin(9600); 
 mySerial.begin(9600);
 sgps.begin(9600);
 pinMode(gasA0, INPUT);
 pinMode(buzzer, OUTPUT); 
 pinMode(led, OUTPUT);
 delay(1000); 
  
  lcd.begin();  
  lcd.backlight();
  lcd.setCursor(3,0); //Set cursor to character 3 on line 0
  lcd.print("*GLAS*");
  
  lcd.createChar(0,Bell);
  lcd.setCursor(2,1); //Set cursor to character 2 on line 1
  lcd.write(0);
  lcd.setCursor(4,1); //Set cursor to character 4 on line 1
  lcd.print("Alert System");
  
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, Sensors);
  delay(1000);
}
 
void loop() 
{
  gasdata = analogRead(gasA0);
  timer.run(); // Initiates SimpleTimer
  lcd.setCursor(3,0); //Set cursor to character 3 on line 0
  lcd.print("*GLAS*");
  lcd.setCursor(1,1); //Set cursor to character 0 on line 1
  lcd.print("Gas%:");
  lcd.setCursor(6,1); //Set cursor to character 5 on line 1
  lcd.print(gasdata);
  digitalWrite(led, LOW);

  sgps.listen();
  while (sgps.available())
  {
    if (gps.encode(sgps.read()))
    {
      //gps.f_get_position(&lati, &longi);
      lati = (gps.location.lat());
      longi = (gps.location.lng());
    }
  }
  
  Blynk.run(); 
  /*timer.setInterval(1000L, Sensors);//on comment*/
  timer.run();
}
 
void sendSMS(String message)
{
  Blynk.begin(auth, ssid, pass);
  mySerial.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(200);
  mySerial.println("AT + CMGS = \"+919940537047\""); // recipient's mobile number, in international format
  delay(200);
  mySerial.println(message);                         // message to send
  delay(200);
  mySerial.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(200); 
  mySerial.println();
  delay(100);                                      // give module time to send SMS
  /* Serial.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(200);
  Serial.println("AT + CMGS = \"+919940537047\""); // recipient's mobile number, in international format
  delay(200);
  Serial.println(message);                         // message to send
  delay(200);
  Serial.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(200); 
  Serial.println();
  delay(100);                                      // give module time to send SMS */
}
 
void Sensors()
{
  //int gasdata = analogRead(gasA0);
  Blynk.virtualWrite(V0, gasdata);
    
  if (gasdata > sensorThres)
  {
    textForSMS =  "\nValue Exceeded!!!\n"; 
    textForSMS = textForSMS + " Sensor Value: "; 
    textForSMS = textForSMS + gasdata; 
    textForSMS = textForSMS + "\n"; 
    
    digitalWrite(buzzer, HIGH);//tone(buzzer, 1000, 200);
    digitalWrite(led, HIGH);
    Blynk.logEvent("alert_gas_leak","!Alert! 'CHECK' Gas Leak Detected!");
    
Serial.print("LAT:  ");
Serial.println(lati, 6);  
Serial.print("LONG: ");
Serial.println(longi, 6);

    lcd.clear(); 
    lcd.setCursor(2,1);  //Set cursor to character 2 on line 1
    lcd.print("GLAS DANGER !!");
 
    sendSMS(textForSMS);
    Serial.println(textForSMS);
    Serial.println("SMS sent."); 
    textForSMS = ""; 
    delay(200);
    lcd.clear();
  }
 
  if (gasdata < sensorThres)  
  {
    textForSMS =  "\nValue Got Normal\n"; 
    textForSMS = textForSMS + " Sensor Value: "; 
    textForSMS = textForSMS + gasdata; 
    textForSMS = textForSMS + "\n";
    
    digitalWrite(buzzer, LOW);//noTone(buzzer);
    digitalWrite(led, LOW);
    
    sendSMS(textForSMS);
    Serial.println(textForSMS);
    textForSMS = "";
    delay(200);
  }
}

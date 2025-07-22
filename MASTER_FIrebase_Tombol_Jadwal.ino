
//---------------------------------------- Include Library.
#include <SPI.h>  //ini utnuk lora
#include <LoRa.h> //ini utnuk lora
#include <Wire.h> //LCD I2c
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>   //wifi
#include <Firebase_ESP_Client.h> //untuk firebase
// Provide the token generation process info.
#include "addons/TokenHelper.h" //library bantu firebase
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"  //library bantu firebase
#include <WiFiClient.h>   //untuk wifi
#include <ezButton.h>     //untuk tombol numpad
#include <NTPClient.h>    //untuk jadwal jam
#include <WiFiUdp.h>      //untuk jadwal jam
#include <LiquidCrystal_I2C.h>    //LCD I2c
//---------------------------------------- 
// Insert your network credentials
#define WIFI_SSID "Pret"              //nama wifi
#define WIFI_PASSWORD "12345678"      //pw

// Insert Firebase project API Key
#define API_KEY "AIzaSyCgfGNG1Y61duTjKXNaJ83dqXgHMej2AMs"       //untuk API firebase
// Insert RTDB URL
#define DATABASE_URL "https://tugasakhir-5ebe6-default-rtdb.firebaseio.com/"   //link firebase
//untuk deklarasi fungsi pada firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//deklarasi hari
String Day= "";

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
String status;

int statusInt;
int statusInt_before = 0;
int counting = 0;


//deklasi pin untuk NUMPAD
int lastKeyPressed = 1;
#define KEY_NUM 4  // Jumlah tombol

// Pin yang terhubung ke tombol pada ESP32
#define PIN_KEY_1 27// Pin 13 pada ESP32
#define PIN_KEY_2 26  // Pin 12 pada ESP32
#define PIN_KEY_3 13  // Pin 27 pada ESP32
#define PIN_KEY_4 12  // Pin 26 pada ESP32

// Mendeklarasikan array keypad dengan pin yang sesuai
ezButton keypad_1x4[] = {
  ezButton(PIN_KEY_1),
  ezButton(PIN_KEY_2),
  ezButton(PIN_KEY_3),
  ezButton(PIN_KEY_4)
};
//habis deklasi numpad

String Tombol ="";
String kondisi_Firebase ="";

//masuk ke fungsi hari, dan waktu, jam 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

//Month names
String months[12]={"Januari", "Februari", "Maret", "April", "Mai", "Juni", "Juli", "Augustus", "September", "Oktober", "November", "Desember"};

//deklasi atau inisialisasi fungsi kalimat
String Siram = "";
String ket = "";


//---------------------------------------- 
//untuk fungsi LCD I2C 
LiquidCrystal_I2C lcd(0x27, 16, 2);
//----------------------------------------

//---------------------------------------- LoRa Pin / GPIO configuration.
//deklarasi pin Lora
#define ss 5
#define rst 14
#define dio0 2
//----------------------------------------

//---------------------------------------- Variable declaration to hold incoming and outgoing data.
String Incoming = "";
String Message = "";     
String send_LED_1_State_Slave_1 = "";        
//----------------------------------------

//---------------------------------------- LoRa data transmission configuration.
byte LocalAddress = 0x01;               //--> address of this device (Master Address).  ini untuk alamat ESP32
byte Destination_ESP32_Slave_1 = 0x02;  //--> destination to send to Slave 1 (ESP32). untuk alamat arduino mega
//---------------------------------------- 

//---------------------------------------- Variable declaration for Millis/Timer.
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 1000;

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//---------------------------------------- 

//---------------------------------------- Declaration of helper variables for the operation of the buttons.
bool Trig_Button_1_State = false;
//---------------------------------------- 

//---------------------------------------- Variable declaration to control the LEDs on the Slaves.
bool LED_1_State_Slave_1 = false;
//---------------------------------------- 

//---------------------------------------- Variable declaration to display the state of the LEDs on the Slaves in the OLED LCD.
bool LED_1_State_Disp_Slave_1 = false;
//---------------------------------------- 

//---------------------------------------- Variable declaration to get temperature and humidity values received from Slaves.
//deklarasi atau inisialisasi tipe data nilai yang ada karena float dia bisa koma
float T_in[2];
float T_fung[2];
float K_Tanah1[2];
float K_Tanah2[2];
float K_Tanah12[2];
float flowRate[2];
//---------------------------------------- 

// Variable declaration to count slaves.
byte Slv = 0;

// Declare a variable as a counter to update the OLED LCD display if there is no incoming message from the slaves.
byte Count_OLED_refresh_when_no_data_comes_in = 0;

// Variable declaration to get the address of the slaves.
byte SL_Address;

// Variable declaration to get LED state on slaves.
String LEDs_State = "";

// Declaration of variable as counter to restart Lora Ra-02.
byte Count_to_Rst_LORA = 0;

//________________________________________________________________________________ Subroutines for sending data (LoRa Ra-02).
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> start packet
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutines for receiving data (LoRa Ra-02).
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  //---------------------------------------- read packet header bytes:
  int recipient = LoRa.read();        //--> recipient address
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length
  //---------------------------------------- 

  // Get the address of the senders or slaves.
  SL_Address = sender;

  // Clears Incoming variable data.
  Incoming = "";

  //---------------------------------------- Get all incoming data / message.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  // Resets the value of the Count_to_Rst_LORA variable if a message is received.
  Count_to_Rst_LORA = 0;

  // Reset the value of the Count_OLED_refresh_when_no_data_comes_in variable if a message is received.
  Count_OLED_refresh_when_no_data_comes_in = 0;

  //---------------------------------------- Check length for error.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("er"); //--> "er" = error: message length does not match length.
    //Serial.println("error: message length does not match length");
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //---------------------------------------- Checks whether the incoming data or message for this device.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("!");  //--> "!" = This message is not for me.
    //Serial.println("This message is not for me.");
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  //----------------------------------------  if message is for this device, or broadcast, print details:
  Serial.println();
  Serial.println("Rc from: 0x" + String(sender, HEX));
  Serial.println("Message: " + Incoming);
  //---------------------------------------- 

  // Calls the Processing_incoming_data() subroutine.
  Processing_incoming_data();
}
//________________________________________________________________________________ 


void Processing_incoming_data() {
  //---------------------------------------- Conditions for processing data or messages from Slave 1 (ESP32 Slave 1).
  if (SL_Address == Destination_ESP32_Slave_1) {
    //menggunakan array dimana mulai penyimpanan angkanya dari 0 
    T_in[0] = GetValue(Incoming, ',', 0).toFloat(); //insektisida
    T_fung[0] = GetValue(Incoming, ',', 1).toFloat(); //fungisida
    K_Tanah1[0] = GetValue(Incoming, ',', 2).toFloat(); //sensor kelembapan tanah 1
    K_Tanah2[0] = GetValue(Incoming, ',', 3).toFloat(); //sensor kelembapan tnaah 2
    K_Tanah12[0] = GetValue(Incoming, ',', 4).toFloat(); //rata-rata
    flowRate[0] = GetValue(Incoming, ',', 5).toFloat(); //waterflow
    // if (LEDs_State == "1" || LEDs_State == "0") {
    //   LED_1_State_Slave_1 = LEDs_State.toInt();
    //   LED_1_State_Disp_Slave_1 = LEDs_State.toInt();
    // }
  }
  //---------------------------------------- 

  // Calls the Update_OLED_Display() subroutine.
  Update_LED_Display();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to display message content from slaves on OLED LCD.
void Update_LED_Display() {
  lcd.setCursor(0, 0);
  lcd.print("Apps =");
  lcd.print(kondisi_Firebase);
  lcd.setCursor(0, 1);
  lcd.print("W= ");
  lcd.print(Siram);
  lcd.print(" - T=");
  lcd.print(Tombol);
  

  if(K_Tanah12[0] != 0)
  {
  lcd.setCursor(0, 0);
  lcd.print("KT = ");
  lcd.print(K_Tanah12[0]);
  lcd.setCursor(0, 1);
  lcd.print("I=");
  lcd.print(T_in[0]);
  lcd.print("-F=");
  lcd.print(T_fung[0]);
  delay(2000);
  lcd.clear();

  //fungsi fuzzy logic
  if(T_in[0] >= 70)
  {
    if(T_fung[0] >= 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 40)
    {
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
    }
    else if ((T_fung[0] <= 10) && (T_fung[0] >= 0))
    {
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungsisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
    }
  }
  else if (T_in[0] >= 40)
  {
    if(T_fung[0] >= 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 40)
    {
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
    }
    else if ((T_fung[0] <= 10) && (T_fung[0] >= 0))
    {
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungsisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
    }
  }
  else if (T_in[0] >= 10)
  {
    if(T_fung[0] >= 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisid Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 40)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisid Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    else if(T_fung[0] >= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisid Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
    }
    else if ((T_fung[0] <= 10) && (T_fung[0] >= 0))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisid Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungsisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
    }
  }
  else if ((T_in[0] <= 10) && (T_in[0] >= 0))
  {
    if(T_fung[0] >= 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pengisian Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    if(T_fung[0] >= 40)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Segera Isi Ulang");
      lcd.setCursor(0, 1);
      lcd.print("Fungisida");
      delay(3000);
    }
    if(T_fung[0] >= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 1);
      lcd.print("Harus Isi Ulang");
      delay(3000);
    }
    else if ((T_fung[0] <= 10) && (T_fung[0] >= 0))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fungsisida");
      lcd.setCursor(0, 1);
      lcd.print("Habis Total");
      delay(3000);
    }
  }
  //habis untuk notifikasi LCD keterangan pupuk
  lcd.clear();

  }
  Incoming = "";
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to get messages from slaves containing the latest data.
//  - The master sends a message to the slaves.
//  - The slaves reply to the message from the master by sending a message containing the latest data of temperature, humidity and LED state to the Master.

void Getting_data_for_the_first_time() { //menerima data
  Serial.println();
  Serial.println("Getting data for the first time...");

  //---------------------------------------- Loop to get data for the first time.
  while(true) {
    unsigned long currentMillis_SendMSG = millis();

    if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
      previousMillis_SendMSG = currentMillis_SendMSG;

      Slv++;
      if (Slv > 1) {
        Slv = 0;
        Serial.println();
        Serial.println("Getting data for the first time has been completed.");
        break;
      }
  
      Message = "N,N";

      //::::::::::::::::: Condition for sending message / command data to Slave 1 (ESP32 Slave 1).
      if (Slv == 1) {
        Serial.println();
        Serial.print("Send message to ESP32 Slave " + String(Slv));
        Serial.println(" for first time : " + Message);
        sendMessage(Message, Destination_ESP32_Slave_1);
      }
      //::::::::::::::::: 

      //::::::::::::::::: 
    }

    //---------------------------------------- parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
    //----------------------------------------
  }
  //---------------------------------------- 
}
//________________________________________________________________________________ String function to process the data received
// I got this from : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
String GetValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to check the state of buttons.

//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to reset Lora Ra-02.
void Rst_LORA() { 
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println("Restart LoRa...");
  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 or 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");

  // Reset the value of the Count_to_Rst_LORA variable.
  Count_to_Rst_LORA = 0;
}
//______________________________________
void ESP_Restart(){ //untuk fungsi restart ESP ketika tidak ada internet
  unsigned long restartclue = 0;
  const long RestartDalam = 7000;
  Serial.println (millis());
  if ((millis()-restartclue) >= RestartDalam){
    ESP.restart();
  }
}
void Auto_Reconnect_5dtk(){  //untuk restar ESP
  ESP_Restart();
  delay(5000);
}
//________________________________________________________________________________ untuk menrima data dari firebase
int getFirebaseKey() {
  int key = 0;
  if (Firebase.RTDB.getString(&fbdo, "/DATA/Jenis_Penyiraman")) {
    if (fbdo.dataType() == "string") {
      String status = fbdo.stringData();
      Serial.println("Firebase Data: " + status);
      key = status.toInt();
    }
  }
  return key;
}
//________________________________________________________________________________ VOID SETUP
void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  lcd.init();  // Inisialisasi LCD
  lcd.backlight();  // Nyalakan backlight LCD
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to ");
    lcd.setCursor(0, 1);
    lcd.print("Wifi........");
    delay(500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SSID : Pret ");
    lcd.setCursor(0, 1);
    lcd.print("Pw: 12345678");
    delay(1000);
    lcd.clear();
      counting = counting + 1;
      Serial.println(counting);
        
      if (counting >= 20 ) {
        Auto_Reconnect_5dtk();
        delay(2000);}
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;     //untuk firebse pada token
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;   //untuk link firbase

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  //----------------------------------------

  lcd.setCursor(0, 0);
  lcd.print("Hello, SF_ITERA");
  lcd.setCursor(0, 1);
  lcd.print("---------------");
  delay(2000);
  lcd.clear();


  //---------------------------------------- Clears the values of the Temp and Humd array variables for the first time.
  for (byte i = 0; i < 2; i++) {
    T_in[i] = 0.00;
    T_fung[i] = 0.00;
    K_Tanah1[i] = 0.00;
    K_Tanah2[i] = 0.00;
    K_Tanah12[i] = 0.00;
    flowRate[i] = 0.00;
  }
  //---------------------------------------- 
  
  // Set debounce time untuk setiap tombol
  for (byte i = 0; i < KEY_NUM; i++) {
    keypad_1x4[i].setDebounceTime(100);  // Set debounce time 100 milidetik
  }

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
  timeClient.update();

    //Get a time structure
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday; //angka tanggal
  int currentMonth = ptm->tm_mon+1; //angka bulan 
  String currentMonthName = months[currentMonth-1]; //nama bulan
  int currentYear = ptm->tm_year+1900;
  
  int Hour = timeClient.getHours();  //angka jam
  // Serial.print("Hour: ");
  // Serial.println(Hour);  

  int Minute = timeClient.getMinutes(); //angka menit
  // Serial.print("Minutes: ");
  // Serial.println(Minute); 
   
  int Second = timeClient.getSeconds(); //angka detik
  // Serial.print("Seconds: ");
  // Serial.println(Second);  

  String Day = weekDays[timeClient.getDay()]; //nama hari
  // Serial.print("Week Day: ");
  // Serial.println(Day);    

  //Print complete date:
  String currentDay = String(Day);
  String currentDate =  String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear) ;
  
  
  lcd.setCursor(0, 0);
  lcd.print(currentDay);
  lcd.setCursor(0, 1);
  lcd.print(currentDate);
  delay(2000);
  lcd.clear();
  // Calls the Update_OLED_Display() subroutine

  delay(1000);

  // Calls the Rst_LORA() subroutine.
  Rst_LORA();

  // Calls the Getting_data_for_the_first_time() subroutine.
  Getting_data_for_the_first_time();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
void loop() {

  jadwal();
  int key = getKeyPressed();  // Mendapatkan tombol yang ditekan
  timeClient.update();

  //Get a time structure
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday; //angka tanggal
  int currentMonth = ptm->tm_mon+1; //angka bulan 
  String currentMonthName = months[currentMonth-1]; //nama bulan
  int currentYear = ptm->tm_year+1900;
  
  int Hour = timeClient.getHours();  //angka jam
  // Serial.print("Hour: ");
  // Serial.println(Hour);  

  int Minute = timeClient.getMinutes(); //angka menit
  // Serial.print("Minutes: ");
  // Serial.println(Minute); 
   
  int Second = timeClient.getSeconds(); //angka detik
  // Serial.print("Seconds: ");
  // Serial.println(Second);  

  String Day = weekDays[timeClient.getDay()]; //nama hari
  // Serial.print("Week Day: ");
  // Serial.println(Day);    

  //fungsi mengirim nilai ke firebase
  if (Firebase.ready()) 
  {
    sendDataPrevMillis = millis();
    if (T_fung[0] != 0) {
      Firebase.RTDB.setFloat(&fbdo, "/DATA/fungisida", T_fung[0]);
      Firebase.RTDB.setFloat(&fbdo, "/DATA/insektisida", T_in[0]);
      Firebase.RTDB.setFloat(&fbdo, "/DATA/keltanah1", K_Tanah1[0]);
      Firebase.RTDB.setFloat(&fbdo, "/DATA/keltanah2", K_Tanah2[0]);
      Firebase.RTDB.setFloat(&fbdo, "/DATA/keltanah", K_Tanah12[0]);
      Firebase.RTDB.setFloat(&fbdo, "/DATA/flowmeter", flowRate[0]);
      Firebase.RTDB.setString(&fbdo, "/DATA/Hari", Day);
      Serial.println("Data sent successfully.");
      delay(1000);
    }
  }

  Update_LED_Display();
  // put your main code here, to run repeatedly:

  //---------------------------------------- Button condition to change the LED state on Slave 1.
  //----------------------------------------

  //---------------------------------------- Millis/Timer to send messages to slaves every 1 second (see interval_SendMSG variable).
  //  Messages are sent every one second is alternately.
  //  > Master sends message to Slave 1, delay 1 second.
  //  > Master sends message to Slave 2, delay 1 second.
  
  //fungsi lora
  unsigned long currentMillis_SendMSG = millis();
  
  if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
    previousMillis_SendMSG = currentMillis_SendMSG;
    //::::::::::::::::: The condition to update the OLED LCD if there is no incoming message from all slaves.
    Count_OLED_refresh_when_no_data_comes_in++;
    if (Count_OLED_refresh_when_no_data_comes_in > 5) {
      Count_OLED_refresh_when_no_data_comes_in = 0;
      Processing_incoming_data();
    }
    //::::::::::::::::: 

//mengambil data dari firebase
  int firebaseKey = getFirebaseKey();
  if (firebaseKey != statusInt_before) {
    if (firebaseKey == 1) {
      Serial.println("Status 1, maka kirim 1");
      kondisi_Firebase = "Siram Air";
      send_LED_1_State_Slave_1 = "1"; 
    } else if (firebaseKey == 0) {
      Serial.println("Status 0, maka kirim 0");
      kondisi_Firebase = "OFF    ";
      send_LED_1_State_Slave_1 = "0";
    } else if (firebaseKey == 2) {
      kondisi_Firebase = "FUNG";
      Serial.println("Status 2, maka kirim 2");
      send_LED_1_State_Slave_1 = "2";
    } else if (firebaseKey == 3) {
      Serial.println("Status 3, maka kirim 3");
      kondisi_Firebase = "INSEK";
      send_LED_1_State_Slave_1 = "3";
    }
    statusInt_before = firebaseKey;
  }
    //::::::::::::::::: Count the slaves.
    Slv++;
    if (Slv > 1) Slv = 1;
    //::::::::::::::::: 
    String formattedTime = timeClient.getFormattedTime();
    Serial.println(formattedTime);

    Serial.print("tombol : ");
    //fungsi tombol
    Serial.println(key);
      if (key == 1) {  // Jika tombol ditekan
        Tombol = "0";
        ket = "Off";
      }
      else if (key == 2){
        Tombol = "2";
        ket = "Fungi";
      }
      else if (key == 3) {
        Tombol = "3";
        ket = "Insek";
      }
      else if (key == 4) {
        Tombol = "4";
      }
    // Enter the values of the send_LED_1_State_Slave_1 and send_LED_1_State_Slave_2 variables to the Message variable.
    //data yang akan di kirim ke lora dalam bentuk string atau kata/kalimat
    Message = send_LED_1_State_Slave_1+ "," + Siram + ","+ Tombol;

    //::::::::::::::::: Condition for sending message / command data to Slave 1 (ESP32 Slave 1).
    if (Slv == 1) {
      Serial.println();
      Serial.println("Tr to  : 0x" + String(Destination_ESP32_Slave_1, HEX));
      Serial.println("Messahhhge: " + Message);
    T_in[0] = 0;
    T_fung[0] = 0;
    K_Tanah1[0] = 0;
    K_Tanah2[0] = 0;
    K_Tanah12[0] = 0;
    flowRate[0] = 0;
      sendMessage(Message, Destination_ESP32_Slave_1);
    }
    //::::::::::::::::: 
  }
  //---------------------------------------- 

  //---------------------------------------- Millis/Timer to reset Lora Ra-02.
  //  - Lora Ra-02 reset is required for long term use.
  //  - That means the Lora Ra-02 is on and working for a long time.
  //  - From my experience when using Lora Ra-02 for a long time, there are times when Lora Ra-02 seems to "freeze" or an error, 
  //    so it can't send and receive messages. It doesn't happen often, but it does happen sometimes. 
  //    So I added a method to reset Lora Ra-02 to solve that problem. As a result, the method was successful in solving the problem.
  //  - This method of resetting the Lora Ra-02 works by checking whether there are incoming messages, 
  //    if no messages are received for approximately 30 seconds, then the Lora Ra-02 is considered to be experiencing "freezing" or error, so a reset is carried out.


  unsigned long currentMillis_RestartLORA = millis();
  
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    Count_to_Rst_LORA++;
    if (Count_to_Rst_LORA > 20) {
      LoRa.end();
      Rst_LORA();
    }
  }
  //----------------------------------------

  //----------------------------------------parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  //----------------------------------------
}

//fungsi bantu dari numpad
//kalau di pencet angka 2 di akan bertahan angka 2 (tidak akan berubah sampai mencet angka lain)
int getKeyPressed() {
  // Panggil fungsi loop() untuk setiap tombol untuk memeriksa statusnya
  for (byte i = 0; i < KEY_NUM; i++) {
    keypad_1x4[i].loop();  // Fungsi ini harus dipanggil agar tombol dapat diproses
  }

  // Cek apakah ada tombol yang ditekan setelah debounce
  for (byte i = 0; i < KEY_NUM; i++) {
    // Dapatkan status tombol setelah debounce
    int key_state = keypad_1x4[i].getState();

    if (keypad_1x4[i].isPressed() && lastKeyPressed != (i + 1)) {  
      // Jika tombol ditekan dan belum pernah ditekan sebelumnya (untuk mencegah perubahan yang tidak diinginkan)
      lastKeyPressed = i + 1;  // Simpan nomor tombol yang ditekan (1-4)
      return lastKeyPressed;    // Kembalikan nomor tombol yang ditekan
    }
  }

  // Jika tidak ada tombol yang ditekan, kembalikan nomor tombol yang terakhir ditekan
  return lastKeyPressed;  // Tetap kembalikan nomor tombol yang terakhir ditekan
}

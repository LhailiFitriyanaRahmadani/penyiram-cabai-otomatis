
//---------------------------------------- Include Library.
//library
#include <SPI.h>    //penggunaan lora
#include <LoRa.h>   //penggunaan lora
#include "Wire.h" // For I2C LCD 20x4
#include "LiquidCrystal_I2C.h" // Added library* 20x4
#include <avr/wdt.h> // Tambahkan ini di awal program
//Set the pins on the I2C chip used for LCD connections
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 is the default I2C bus address

//===============Arduino Mega=================
// pembacaan waterflow
const int flowPin = 3; // digunakan pada pin 3 (pin digital yang 3)
//deklrasi kebutuhan pembacaan waterflow
volatile int pulseCount = 0; // Variabel untuk menghitung pulsa
float flowRate = 0.0; // Variabel untuk menghitung aliran dalam L/min
unsigned long previousMillis = 0; // Untuk menghitung waktu
unsigned long interval = 1000; // Interval waktu (1 detik)

//---------------------------------------- 

//---------------------------------------- 

//---------------------------------------- Defines LED Pins.

//deklarasi pin relay (aktuator)
#define LED_1_Pin   6 //Pengisian Air
#define LED_2_Pin   7 //pengaduakn
#define LED_3_Pin   8 //Fungisida
#define LED_4_Pin   9 //penyiraman 
#define LED_5_Pin   10 //pompa dari pengaduk
#define LED_6_Pin   11 //insektisida

//pembacaan sensor kelembapan tanah 
const int sensorPin1 = A0; //pakai pin A0 karena dia menggunakan pembacaan analog (ADC) dengan membaca berdasarkan perbedaan tegangan 
const int sensorPin2 = A1; 
//--------------------------------///

//pembacaan sensor ultrasonic
//---------------------------------------- 
//menggunakan pin digital maka dia langsung angka
const int trigPin1 = 38; // Pin Trig untuk sensor pertama
const int echoPin1 = 36; // Pin Echo untuk sensor pertama

// Deklarasi pin untuk sensor ultrasonik kedua
const int trigPin2 = 46; // Pin Trig untuk sensor kedua
const int echoPin2 = 44; // Pin Echo untuk sensor kedua

//---------------------------------------- LoRa Pin / GPIO configuration.
//deklarasi pin Lora berdasarkan fungsi nya
#define ss 5
#define rst 14
#define dio0 2
//----------------------------------------

//deklarasi untuk tipe data nilai sensor
//int sifat nilainya tidak ada koma
//float tipe data yang berkoma
float h;
float t;
int kelembaban_akhir;
int sensorValue1;
int sensorValue2;
float kelembaban1;
float kelembaban2;
float distance3;
float distance4;
int durasi;

//----------------------------------------String variable for LoRa

//tipe data string digunakan untuk lora 
//tipe data yang berbentuk kata atau kalimat
String Incoming = "";  //deklarasi pin untuk data yang diterima (dari ESP ke arduino mega)
String Message = "";   //pesan yang akan di kirim ke ESP dari arduino mega
String CMD_LED_1_State = "";   //deklarasi atau inisialisasi untuk data yang di terimaa
String CMD_SIRAM_State = "";  //deklarasi atau inisialisasi untuk data yang di terimaa
String CMD_Tombol_State = ""; //deklarasi atau inisialisasi untuk data yang di terimaa
String ket_tanah = "";  //deklarasi atau inisialisasi untuk data yang di terimaa
//----------------------------------------

//---------------------------------------- LoRa data transmission configuration.
////////////////////////////////////////////////////////////////////////////
// PLEASE UNCOMMENT AND SELECT ONE OF THE "LocalAddress" VARIABLES BELOW. //
////////////////////////////////////////////////////////////////////////////

byte LocalAddress = 0x02;       //--> address of this device (Slave 1) untuk alamat arduino mega
//byte LocalAddress = 0x03;       //--> address of this device (Slave 2).

byte Destination_Master = 0x01; //--> destination to send to Master (ESP32).  untuk alamat esp32
//----------------------------------------

//---------------------------------------- Variable declarations for temperature and humidity values.
//---------------------------------------- 


//---------------------------------------- Variable declaration for Millis/Timer.

unsigned long previousMillis_RestartLORA = 0;
const long interval_RestartLORA = 1000;
//---------------------------------------- 

// Declaration of variable as counter to restart Lora Ra-02.
byte Count_to_Rst_LORA = 0;

//________________________________________________________________________________ Subroutines for sending data (LoRa Ra-02).
//pengiriman data melalui lora
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
//penerimaan data dari lora ESP32
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  //---------------------------------------- read packet header bytes:
  int recipient = LoRa.read();        //--> recipient address
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length
  //---------------------------------------- 

  //---------------------------------------- Condition that is executed if message is not from Master.
  if (sender != Destination_Master) {
    Serial.println();
    Serial.println("i"); //--> "i" = Not from Master, Ignore.
    //Serial.println("Not from Master, Ignore");

    // Resets the value of the Count_to_Rst_LORA variable.
    Count_to_Rst_LORA = 0;
    return; //--> skip rest of function
  }
  //---------------------------------------- 

  // Clears Incoming variable data.
  Incoming = "";

  //---------------------------------------- Get all incoming data.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }
  //---------------------------------------- 

  // Resets the value of the Count_to_Rst_LORA variable.
  Count_to_Rst_LORA = 0;

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

    // Calls the Processing_incoming_data_for_LEDs() subroutine.
    Processing_incoming_data_for_LEDs();
    return; //--> skip rest of function
  } else {
    // if message is for this device, or broadcast, print details:
    Serial.println();
    Serial.println("Rc from: 0x" + String(sender, HEX));
    Serial.println("Message: " + Incoming); //bagian yang akan disimpan datanya pada incoming

    // Calls the Processing_incoming_data() subroutine.
    Processing_incoming_data();
  }
}
//________________________________________________________________________________ 

//________________________________________________________________________________ Subroutine to process the data to be sent, after that it sends a message to the Master.
void Processing_incoming_data() {
  // Calls the Processing_incoming_data_for_LEDs() subroutine.
  Processing_incoming_data_for_LEDs(); 

  // Get the last state of the LED.
  byte LED_1_State = digitalRead(LED_1_Pin);
  Serial.println("LED_1_State");
  Serial.println(LED_1_State);

  // Fill in the "Message" variable with the value of humidity, temperature and the last state of the LED.
  Message = String(distance3) + "," + String(distance4) + "," + String(kelembaban1) + "," + String(kelembaban2) + "," + String(kelembaban_akhir) + "," + String(int(flowRate));
  //massage = "100, 90, 32, 22, 28, 2"
  Serial.println();
  Serial.println("Tr to  : 0x" + String(Destination_Master, HEX));
  Serial.println("Message: " + Message);

  // Send a message to Master.
  sendMessage(Message, Destination_Master);
}

void Processing_incoming_data_for_LEDs() { 
//1,0,0
CMD_LED_1_State = GetValue(Incoming, ',', 0);  //berdasarkan aplikasi   
CMD_SIRAM_State = GetValue(Incoming, ',', 1);   //berdasarkan waktu atau penjadwalan
CMD_Tombol_State = GetValue(Incoming, ',', 2);  //berdasarkan tombol 
}
//________________________________________________________________________________ 
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

//________________________________________________________________________________ Subroutine to reset Lora Ra-02.
void Rst_LORA() {
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println(F("Restart LoRa..."));
  Serial.println(F("Start LoRa init..."));

  if (!LoRa.begin(433E6)) { // Gagal inisialisasi LoRa
    Serial.println(F("LoRa init failed. Check your connections."));
    delay(1000); // Tunggu sebentar agar pesan sempat dikirim ke Serial Monitor
    wdt_enable(WDTO_15MS); // Aktifkan watchdog dengan timeout 15ms
    while (true) {
      // Menunggu watchdog melakukan reset
    }
  }

  Serial.println(F("LoRa init succeeded."));

  // Reset variabel penghitung
  Count_to_Rst_LORA = 0;
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP
//program yang hanya berjalan 1 kali di awal ketika mulai
void setup() {
  // put your setup code here, to run once:
  // Set off LCD module
  lcd.begin (20,4); // 20 x 4 LCD module
  // Turn on the blacklight and print a message.
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Hello, Smart Farming");
  lcd.setCursor(0, 1);
  lcd.print("TERASFARM");
  delay(2000);
  lcd.clear();


  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  // Mengatur pin untuk sensor kedua
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  Serial.begin(115200);

  pinMode(LED_1_Pin, OUTPUT);
  pinMode(LED_2_Pin, OUTPUT);
  pinMode(LED_3_Pin, OUTPUT);
  pinMode(LED_4_Pin, OUTPUT);
  pinMode(LED_5_Pin, OUTPUT);
  pinMode(LED_6_Pin, OUTPUT);
  digitalWrite(LED_1_Pin, LOW);
  digitalWrite(LED_2_Pin, LOW);
  digitalWrite(LED_3_Pin, LOW);
  digitalWrite(LED_4_Pin, LOW);
  digitalWrite(LED_5_Pin, LOW);
  digitalWrite(LED_6_Pin, LOW);
  //____________________________
  // pinMode(SENSOR, INPUT_PULLUP);

  // pinMode(sensorPin, INPUT); // Mengatur pin sensor sebagai input
  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), countPulse, FALLING);
  // Calls the Rst_LORA() subroutine.
  Rst_LORA(); //hanya menjalankan 1 kali 

}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP
  //program yang terus berulang
void loop() {

  float distance1 = readUltrasonic(trigPin1, echoPin1);
  distance1 = 23.023 * (distance1) + 0.5778; //kalibrasi CM

  // Membaca jarak dari sensor kedua
  float distance2 = readUltrasonic(trigPin2, echoPin2);
  distance2 = 23.923 * (distance2) + 0.2683; //kalibrasi CM
  // put your main code here, to run repeatedly:

  //perubahan dari jarak menjadi persen
  distance3 = ((12 - distance1) * 100)/12;
  // distance1 = ((12 - (A_distance1)100)/12)
  // Membaca jarak dar sensor kedua
  distance4 = ((12 - distance2) * 100)/12;

    // Membaca jarak dari sensor pertama

  // Menampilkan hasil dari sensor pertama
  Serial.print("Sensor 1 - Jarak: ");
  Serial.print(distance1); 
  Serial.println("%");

  // Menampilkan hasil dari sensor kedua
  Serial.print("Sensor 2 - Jarak: ");
  Serial.print(distance2);
  Serial.println("%");

  Serial.println("========================");

  // Membaca nilai analog dari setiap sensor
  //pembacaan sensor kelembapan tanah 
  sensorValue1 = analogRead(sensorPin1);
  sensorValue2 = analogRead(sensorPin2);

  // Konversi nilai ke persentase kelembaban
  //pembacaan analognya itu dari 0-1023 atau 8 bit atau 2^8 (1024)
  kelembaban1 = (1.5232 * (map(sensorValue1, 0, 1023, 100, 0))) - 27.824;
  kelembaban2 = (0.5069 * (map(sensorValue2, 0, 1023, 100, 0))) - 37.49;

  kelembaban1 = constrain(kelembaban1, 0, 100);
  kelembaban2 = constrain(kelembaban2, 0, 100);

  // Menampilkan hasil di Serial Monitor
  Serial.println("=== Pembacaan Sensor ===");
  Serial.print("Sensor 1 - Nilai Analog: ");
  Serial.print(sensorValue1);
  Serial.print(" | Kelembaban (%): ");
  Serial.println(kelembaban1);

  Serial.print("Sensor 2 - Nilai Analog: ");
  Serial.print(sensorValue2);
  Serial.print(" | Kelembaban (%): ");
  Serial.println(kelembaban2);

  Serial.println("========================");

  //rata rata dari kelembapan
  kelembaban_akhir = (kelembaban1 + kelembaban2)/2;
  //---------------------------------------- 
   unsigned long currentMillis = millis();
  
    //=============
    //pembacaan waterflow
  if (currentMillis - previousMillis >= interval) {
    // Setel waktu sebelumnya
    previousMillis = currentMillis;
    
    // Menghitung flow rate (L/min)
    flowRate = pulseCount / 7.5; // 7.5 pulsa per liter (nilai ini bisa bervariasi)
    pulseCount = 0; // Reset penghitung pulsa
    
    // Menampilkan data di serial monitor
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.println(" L/min");
    lcd.setCursor(0, 3);  //menampilan pada kolam ke 0 pada barus ke 4
    lcd.print("Flow = ");
    lcd.print(flowRate);  //ganti bagia ini jadi lcd.print("12");
    lcd.print(" L/min");
  }
  //=====================================
  //fungsi logic untuk kelembapan tanah

  //=====================================
  //menampilkan pengukuran pupuk
  //=====================================
  lcd.setCursor(0, 0);  //menampilan pada kolom pertama, baris pertama
  lcd.print("K-Tanah=");
  lcd.print(kelembaban_akhir); //ganti angka
  lcd.print("%-");
  lcd.print(ket_tanah); //ganti keterangan
  
  lcd.setCursor(0, 1);  //menampilan pada kolom pertama, baris kedua
  lcd.print("Insektisida= "); 
  lcd.print(distance3); //ganti angka
  lcd.print("%");

  lcd.setCursor(0, 2); // menampilkan pada kolom pertama, barus ke 3
  lcd.print("Fungisida= ");
  lcd.print(distance4);   //ganti angka
  lcd.print("%");
  delay(1000);

  lcd.clear();

  if (distance3 == 100)
  {
  if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }
  
  else if((distance3 > 70) && (distance3 <= 99))
  {
    if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Penuh");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }

  if(distance3 == 70)
  {
  if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }
  else if ((distance3 >= 40) && (distance3 <= 69))
  {
  if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida CUkup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Cukup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Cukup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Cukup");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }

  else if ((distance3 >= 11) && (distance3 <= 39))
  {
  if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Menipis");
      lcd.setCursor(0, 1);
      lcd.print("Insektisida= ");
      lcd.print(distance3);
      lcd.print("%");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }
  else if (distance3 <= 10)
  {
      if((distance4 > 70) && (distance4 <= 99))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if (distance4 == 70)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 40) && (distance4 <= 69))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Cukup");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if ((distance4 >= 11) && (distance4 <= 39))
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Menipis");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida= ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
    else if(distance4 <= 10)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida Habis");
      lcd.setCursor(0, 3);
      lcd.print("Isi Ulang Pupuk!");
      delay(1000);
    }
    else if(distance4 == 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("Insektisida Zonk");
      lcd.setCursor(0, 1);
      lcd.print("Isi Ulang Pupuk!");
      lcd.setCursor(0, 2);
      lcd.print("Fungisida penuh");
      lcd.setCursor(0, 3);
      lcd.print("Fungisida : ");
      lcd.print(distance4);
      lcd.print("%");
      delay(1000);
    }
  }
    // habis untuk menampikan kondisi pupuk atau dari sensor ultrasonic
  //=========
  // fuzzy logic untuk menampilkan data di LCD



  if (kelembaban_akhir >= 71)
  {
    ket_tanah = "Basah";
  }
  else if ((kelembaban_akhir >= 61) && (kelembaban_akhir < 70))
  {
    ket_tanah = "Normal";
  }
  else if (kelembaban_akhir < 60)
  {
    ket_tanah = "Kering";
  }


  if (CMD_LED_1_State == "1")
  {
  int durasi = fuzzySugeno(kelembaban_akhir);
    if (durasi > 0) {
      digitalWrite(LED_4_Pin, LOW);
      digitalWrite(LED_1_Pin, HIGH);
      digitalWrite(LED_2_Pin, HIGH);
      digitalWrite(LED_3_Pin, HIGH);
      digitalWrite(LED_5_Pin, HIGH);
      digitalWrite(LED_6_Pin, HIGH);
      delay(durasi * 1000); // menyiram sesuai durasi fuzzy
      digitalWrite(LED_4_Pin, HIGH);
      digitalWrite(LED_1_Pin, HIGH);
      digitalWrite(LED_2_Pin, HIGH);
      digitalWrite(LED_3_Pin, HIGH);
      digitalWrite(LED_5_Pin, HIGH);
      digitalWrite(LED_6_Pin, HIGH);
      delay(1000);
    }
  }
  if ((CMD_LED_1_State == "2") || (CMD_SIRAM_State == "2") ||(CMD_Tombol_State == "2")) { // Penyiraman Fungisida
  lcd.clear();
  digitalWrite(LED_1_Pin, LOW);
  digitalWrite(LED_4_Pin, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("P1 - Isi Air");
  int count = 0;
    while (count < 40) {
      // Delay 1 detik
      delay(1000);
      digitalWrite(LED_1_Pin, LOW);
      // Hitung flow rate (L/min)
      flowRate = pulseCount / 7.5; // 7.5 pulsa per liter (nilai ini bisa bervariasi)
      pulseCount = 0; // Reset penghitung pulsa

      // Tampilkan data di Serial Monitor
      Serial.print("Flow rate: ");
      Serial.print(flowRate);
      Serial.println(" L/min");
      
      // Tampilkan di LCD
      lcd.setCursor(0, 2);
      lcd.print("Flow = ");
      lcd.print(flowRate);
      lcd.print(" L/min");
      count++;
    }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P1 - Isi Air");
  digitalWrite(LED_1_Pin, HIGH);
  Serial.println("FUNGSISIDA");
  digitalWrite(LED_3_Pin, LOW);
  lcd.setCursor(0, 1);
  lcd.print("P2 - Fungisida");
  delay(1000);
  digitalWrite(LED_3_Pin, HIGH);
  digitalWrite(LED_2_Pin, LOW);
  lcd.setCursor(0, 2);
  lcd.print("P3 : Pengadukan");
  delay(10000);
  digitalWrite(LED_2_Pin, HIGH);
  lcd.clear();
  Serial.println("POMPA 1");
  lcd.setCursor(0, 0);
  lcd.print("Proses Siram");
  lcd.setCursor(0, 1);
  lcd.print("Tanaman");
  lcd.setCursor(0, 2);
  lcd.print("Fungisida");
  digitalWrite(LED_5_Pin, LOW);
  delay(40000);
  CMD_LED_1_State = "0";
  CMD_Tombol_State = "0";
  CMD_SIRAM_State = "0"; // Reset Incoming setelah proses
}

else if ((CMD_LED_1_State == "3") || (CMD_SIRAM_State == "3")|| (CMD_Tombol_State == "3")) { // Penyiraman Insektisida
  lcd.clear();
  digitalWrite(LED_1_Pin, LOW);
  digitalWrite(LED_4_Pin, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("P1 - Isi Air");
  int count = 0;
    while (count < 40) {
      // Delay 1 detik
      delay(1000);
      // Hitung flow rate (L/min)
      flowRate = pulseCount / 7.5; // 7.5 pulsa per liter (nilai ini bisa bervariasi)
      pulseCount = 0; // Reset penghitung pulsa

      // Tampilkan data di Serial Monitor
      Serial.print("Flow rate: ");
      Serial.print(flowRate);
      Serial.println(" L/min");
      
      // Tampilkan di LCD
      lcd.setCursor(0, 2);
      lcd.print("Flow = ");
      lcd.print(flowRate);
      lcd.print(" L/min");
      count++;
    }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P1 - Isi Air");
  digitalWrite(LED_1_Pin, HIGH);
  Serial.println("INSEKTISIDA");
  digitalWrite(LED_6_Pin, LOW);
  lcd.setCursor(0, 1);
  lcd.print("P2 - Insektisida");
  delay(1000);
  digitalWrite(LED_6_Pin, HIGH);
  digitalWrite(LED_2_Pin, LOW);
  lcd.setCursor(0, 2);
  lcd.print("P3 - Pengadukan");
  delay(10000);
  digitalWrite(LED_2_Pin, HIGH);
  lcd.clear();
  Serial.println("POMPA 1");
  lcd.setCursor(0, 0);
  lcd.print("Proses Siram");
  lcd.setCursor(0, 1);
  lcd.print("Tanaman");
  lcd.setCursor(0, 2);
  lcd.print("Insektisida");
  digitalWrite(LED_5_Pin, LOW);
  delay(40000);
  CMD_LED_1_State = "0";
  CMD_Tombol_State = "0";
  CMD_SIRAM_State = "0"; // Reset Incoming setelah proses
}
  else 
  {
    digitalWrite(LED_4_Pin, HIGH);
    digitalWrite(LED_1_Pin, HIGH);
    digitalWrite(LED_2_Pin, HIGH);
    digitalWrite(LED_3_Pin, HIGH);
    digitalWrite(LED_5_Pin, HIGH);
    digitalWrite(LED_6_Pin, HIGH);
  }
  //---------------------------------------- Millis/Timer to reset Lora Ra-02.
  // Please see the Master program code for more detailed information about the Lora reset method.
  

  //fungsi pengiriman lora
  unsigned long currentMillis_RestartLORA = millis();
  
  if (currentMillis_RestartLORA - previousMillis_RestartLORA >= interval_RestartLORA) {
    previousMillis_RestartLORA = currentMillis_RestartLORA;

    Count_to_Rst_LORA++;
    if (Count_to_Rst_LORA > 30) {  //kalo gak 30 kali ngirim atau nerima di akan restar
      LoRa.end();
      Rst_LORA();
    }
  }
  //---------------------------------------- 

  //---------------------------------------- parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  lcd.clear();
  //----------------------------------------
}

float readUltrasonic(int trigPin, int echoPin) {  //alat bantu atau code bantu untuk pembacaan ultrasonic
  // Mengirimkan sinyal ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Membaca durasi pantulan sinyal
  long duration = pulseIn(echoPin, HIGH);

  // Menghitung jarak dalam cm
  float distance = duration * 0.034 / 2;

  return distance;
}

void countPulse() { 
  pulseCount++; // Increment hitungan pulsa setiap kali interrupt terjadi
}

float membershipTriangle(float x, float a, float b, float c) {
  if (a == b || b == c) return 0;
  if (x <= a || x >= c) return 0;
  else if (x == b) return 1;
  else if (x < b) return (x - a) / (b - a);
  else return (c - x) / (c - b);
}

// Fungsi fuzzy Sugeno
int fuzzySugeno(float x) {
  if (x < 0) x = 0;
  if (x > 100) x = 100;

  float kering = membershipTriangle(x, 0, 55, 65);
  float normal = membershipTriangle(x, 55, 65, 75);
  float basah  = membershipTriangle(x, 75, 85, 100);

  float z1 = -0.7 * x + 90;
  float z2 = -0.7 * x + 60;
  float z3 = -0.7 * x + 30;

  float total = kering + normal + basah;
  if (total == 0) return 0;

  float output = (kering * z1 + normal * z2 + basah * z3) / total;

  if (output < 0) output = 0;
  if (output > 60) output = 60;

  return (int)output;
}
//________________________________________________________________________________ 
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
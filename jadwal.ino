void jadwal()
{ 
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
  String formattedTime = timeClient.getFormattedTime();
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
      if ((Minute == 31) || (Minute == 1))
      {
        Serial.println("Siram");
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Siram", "Hidup");
        Siram = "1";
      }
      else if ((Day == "Senin")&&(timeClient.getHours() == 7) && (timeClient.getMinutes() == 0))
      {
        Serial.println("Fungisida");
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Fungisida", "Hidup");
        Siram = "2";
      }

      else if (((Day == "Rabu") && (timeClient.getHours() == 7) && (timeClient.getMinutes() == 0)) || ((Day == "Jumat") && (timeClient.getHours() == 7) && (timeClient.getMinutes() == 0)))
      {
        Siram = "3";
        Serial.println("Insektisida");
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Insektisida", "Hidup");
      }

      else if ((Day == "Sabtu") && (timeClient.getHours() == 7))
      {
        Serial.println("NPK");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Penyiraman");
        lcd.setCursor(0, 1);
        lcd.print("NPK");
        delay(1000);
        lcd.clear();
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Insektisida", "Hidup");
      }

      else {
        Siram = "0";
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Siram", "Mati");
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Insektisida", "Mati");
        Firebase.RTDB.setString(&fbdo, "/DATA/Kondisi_Fungisida", "Mati");

      }
  }
}
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
 
// Номер пина Arduino с подключенным датчиком
#define PIN_DS18B20 13

 
// Создаем объект OneWire
OneWire oneWire(PIN_DS18B20);
 
// Создаем объект DallasTemperature для работы с сенсорами, передавая ему ссылку на объект для работы с 1-Wire.
DallasTemperature dallasSensors(&oneWire);
 
// Специальный объект для хранения адреса устройства
DeviceAddress sensorAddress;

boolean stringComplete = false;  // Флажок на окончание приема данных
String inputString = "";        // Строковая переменная
String inputString1 = "";        // Строковая переменная

/////////////////////////////////////////////////////////////
    String VERSION =  "1.11";
    String dVERSION =  "  (2019 04 01)  ";
//
//
//    
    
/////////////////////////////////////////////////////////////

int aPin=A0;
int avalue=0;
Adafruit_AM2320 am2320 = Adafruit_AM2320();

///////////////////////////////////////////////////////////////////////////////////////////////////////
//   Setup statment
///////////////////////////////////////////////////////////////////////////////////////////////////////
void setup(void){
  Serial.begin(115200);
  Serial.println("{\"start\":"+String(millis()));

  Serial.print(",\"version\": \""+VERSION+" "+dVERSION+"\"");
  
  am2320.begin();
  
  dallasSensors.begin();
  Serial.print(",\"countSensors\":");
  Serial.print(dallasSensors.getDeviceCount(), DEC);
  Serial.println(" }");
 
  for (int i=0; i<dallasSensors.getDeviceCount(); i++){
    if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("Не можем найти первое устройство");
    dallasSensors.setResolution(sensorAddress, 12);
  } 
}

//
// именно тут происходит волшебство, тут читается строка из порта, ну или посчти тут.
//
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); // получаем данные с порта
    inputString += inChar;  // собираем всю строку
    if (inChar == '\n')  // если наткнулись на перевод коретки
    {
      for (int i=1; i<=inputString.length()-1; i++ ){ // собираем строку после первого символа, т.е читаем значение переданного параметра
        inputString1 = inputString1 + inputString.charAt(i); // собираем наш полученный параметр
      }
      if (inputString != "clr\n") { // исключение команды очистки порта
        inputString=inputString.charAt(0); // берем наш первый символ, который определяет функцию
      }
      stringComplete = true;
    }
  }
}

// Вспомогательная функция печати значения температуры для устрйоства
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = dallasSensors.getTempC(deviceAddress);
  Serial.print(",\"temperatuteC\": ");
  Serial.print(tempC);
  Serial.print(",\"temperatuteF\": ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (i!=0) {Serial.print(":");}
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("\"sensor_addres\": \"");
  printAddress(deviceAddress);
  Serial.print("\" ");
  printTemperature(deviceAddress);
}

void getSensors(int numSens)
{
  Serial.print("{\"sens_count\":"+String(dallasSensors.getDeviceCount())+",\"sensors\":[");
  if (numSens>=0){
    if (!dallasSensors.getAddress(sensorAddress, numSens)) Serial.println("{\"error\":\"100000 Can't found first sensor\"}");
    Serial.print("{ \"sensor_number\":"+String(numSens)+",");
      dallasSensors.requestTemperatures(); // Просим ds18b20 собрать данные
      printData(sensorAddress);
      Serial.println("}");
    }
  else {  
    for (int i=0; i<  dallasSensors.getDeviceCount(); i++){
      if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("{\"error\":\"100001 Can't found first sensor\"}");
      if (i!=0){Serial.print(",");}
      Serial.print("{ \"sensor_number\":"+String(i)+",");
      dallasSensors.requestTemperatures(); // Просим ds18b20 собрать данные
      printData(sensorAddress);
      Serial.println("}");
    }
  }  
  Serial.println("]}"); 
}
 
void loop(void){  
  serialEvent(); // переходим в процедуру опроса порта
  if (stringComplete) {  // если мы получили данные
    if (inputString=="R" ){
      if (inputString1.toInt()>=0){
        getSensors(inputString1.toInt());
      }
    } 
    else if (inputString == "G"){
      Serial.print("{\"num_sens\": 0,\"sens_type\" : \"AM2320\", \"sens_id\" : \"none\",\"sens_Val\": { \"temp\":"+String(am2320.readTemperature())+", \"hum\":"+String(am2320.readHumidity())+"}}"); 
      delay(1000);
    }
    else if (inputString == "W"){
      avalue=analogRead(aPin);
      //delay(1000);
      Serial.print("{\"num_sens\": 0,\"sens_type\" : \"water\", \"sens_id\" : \"none\",\"sens_Val\": { \"volt\":"+String(avalue)+"}}"); 
    }     
    else if (inputString == "V"){
      Serial.print("{\"version\": \""+VERSION+" "+dVERSION+"\",\"HardType\":\"mattGrowniMini (UNO + 5 * DS18B20 + AM2320 + Water in terra level)\"}");
    }
    else if (inputString == "H" or inputString == "?" ){
      Serial.print("{\"command\":[{\"R\":\"value 0 sensor\"},{\"/R[0-"+String(dallasSensors.getDeviceCount())+"]\":\"value sensors 0-"+String(dallasSensors.getDeviceCount())+"\"},{\"G\":\"Box Climat\"},{\"W\":\"water in terra\"},{\"V\":\"Hard version\"},{\"H/?\":\"Help this is msg\"}]}"); 
    }            
    // не особо понятно за что отвечает этот параметр, внёс в 1.2
//    if (inputString=="clr\n"){
    if (inputString=="\n"){
      Serial.flush();
      Serial.println("{\"ping\":\"pong\"}");
    }
  }

  inputString="";
  inputString1="";
  stringComplete = false;
  // Задержка для того, чтобы можно было что-то разобрать на экране
  delay(1000);
}

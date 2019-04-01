#include <OneWire.h>
#include <DallasTemperature.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

// Include the libraries we need


// Номер пина Arduino с подключенным датчиком
#define PIN_DS18B20 13

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(PIN_DS18B20);

// Создаем объект DallasTemperature для работы с сенсорами, передавая ему ссылку на объект для работы с 1-Wire.
DallasTemperature dallasSensors(&oneWire);

// Специальный объект для хранения адреса устройства
DeviceAddress sensorAddress;

// датчик влажности
int aPin=A0;
int avalue=0;

Adafruit_AM2320 am2320 = Adafruit_AM2320();
int num_temp;
int val; // Задаем переменную val для отслеживания нажатия клавиши
//int ledpin = 13; // задаем цифровой интерфейс ввода/вывода 13 - это наш светодиод
String readString;

boolean stringComplete = false;  // Флажок на окончание приема данных
String inputString = "";        // Строковая переменная
String inputString1 = "";        // Строковая переменная
String inputString2 = "";        // Строковая переменная
String delfun = "";

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }
  /*  */
  Serial.println("Hard is ready...");
  Serial.println("Press H or ? to help.");
  am2320.begin();

  // Выполняем поиск устрйоств на линии
  Serial.print("Ищем устройства...");
  dallasSensors.begin();
  Serial.print("Найдено ");
  Serial.print(dallasSensors.getDeviceCount(), DEC);
  Serial.println(" устройств.");

  // Определяем режим питания (по отдельной линии или через паразитное питание по линии данных)
  Serial.print("Режим паразитного питания: ");
  if (dallasSensors.isParasitePowerMode()) Serial.println("ВКЛЮЧЕН");
  else Serial.println("ВЫКЛЮЧЕН");


  for (int i=0; i<dallasSensors.getDeviceCount(); i++){
    if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("Не можем найти первое устройство");
      Serial.print("Адрес устройства: ");
      printAddress(sensorAddress);
      Serial.println();
      dallasSensors.setResolution(sensorAddress, 12);
      Serial.print("Разрешение датчика: ");
      Serial.print(dallasSensors.getResolution(sensorAddress), DEC);
      Serial.println();
    }
  
}

// function to print a device address
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

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = dallasSensors.getTempC(deviceAddress);
  Serial.print(",\"temperatuteC\": ");
  Serial.print(tempC);
  Serial.print(",\"temperatuteF\": ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(dallasSensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("\"sensor_addres\": \"");
  printAddress(deviceAddress);
  Serial.print("\" ");
  printTemperature(deviceAddress);
//  Serial.print();
}

void getSensors(void)
{
//   for (int i=0; i<dallasSensors.getDeviceCount(); i++){
//    if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("Не можем найти первое устройство");
//
//      printAddress(sensorAddress);
//      Serial.println();
//      Serial.print("Измеряем температуру...");
//      dallasSensors.requestTemperatures(); // Просим ds18b20 собрать данные
//  Serial.println("Выполнено");
// 
//  //  Запрос на получение сохраненного значения температуры
//  printTemperature(sensorAddress);
//
//    }
  Serial.print("{\"sens_count\":"+String(dallasSensors.getDeviceCount())+",\"sensors\":[");
  for (int i=0; i<  dallasSensors.getDeviceCount(); i++){
    if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("Не можем найти первое устройство");
    if (i!=0){Serial.print(",");}
    Serial.print("{ \"sensor_number\":"+String(i)+",");
//    Serial.println(dallasSensors.getTempCByIndex(i));
    //if (!dallasSensors.getAddress(sensorAddress, i)) Serial.println("Unable to find address for Device "+String(i));
    // Устанавливаем разрешение датчика в 12 бит (мы могли бы установить другие значения, точность уменьшится, но скорость получения данных увеличится
    dallasSensors.requestTemperatures(); // Просим ds18b20 собрать данные
    printData(sensorAddress);
    Serial.println("}");
  }
 Serial.println("]}"); 
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); // получаем данные с порта
    inputString += inChar;  // собираем всю строку
    if (inChar == '\n')  // если наткнулись на перевод коретки
    {
      for (int i=1; i<=inputString.length()-1; i++ ) // собираем строку после первого символа, т.е читаем значение переданного параметра
      {
      inputString1 = inputString1 + inputString.charAt(i); // собираем наш полученный параметр
      }
      if (inputString != "clr\n") // исключение команды очистки порта
      {
      inputString=inputString.charAt(0); // берем наш первый символ, который определяет функцию
      }
      stringComplete = true;
    }
  }
}

void loop() {
  int sel_sens = 0;

  serialEvent(); // переходим в процедуру опроса порта
  if (stringComplete) {  // если мы получили данные
    //Serial.println("Command: " + inputString); // для проверки того, что ардуино получил
    //Serial.println("Function: " + inputString1);
    if (inputString=="R"){
      //Serial.println("R DeTECTED");
      getSensors();
    }
    else if (inputString == "G"){
      Serial.print("{\"num_sens\": 0,\"sens_type\" : \"AM2320\", \"sens_id\" : \"none\",\"sens_Val\": { \"temp\":"+String(am2320.readTemperature())+", \"hum\":"+String(am2320.readHumidity())+"}}"); 
      delay(1000);
    }
    else if (inputString == "W"){
      avalue=analogRead(aPin);
      // вывод значения в монитор последовательного порта Arduino
      Serial.print("{\"num_sens\": 0,\"sens_type\" : \"water\", \"sens_id\" : \"none\",\"sens_Val\": { \"volt\":"+String(avalue)+"}}"); 
      }     
    if (inputString=="clr\n"){
      Serial.flush();
      Serial.println("Port is clear");
    }




 inputString="";
 inputString1="";
 stringComplete = false;

}

//  while (Serial.available()) {
//    delay(3);  //delay to allow buffer to fill 
//    if (Serial.available() >0) {
//      char c = Serial.read();  //gets one byte from serial buffer
//      readString += c; //makes the string readString
//    } 
//  }
  
//  //val = Serial.read (); // Считываем команду посланную с компьютера через консоль IDE Arduino
//  if (readString == 'R'){ // Задаем букву условие на букву "R", при нажатии которой в консоли будет зажигался светодиод и появится строка "Hello World!"
//   get_term(1);} 
//  else if (val == 'G'){
//    Serial.print("{\"num_sens\": 0,\"sens_type\" : \"AM2320\", \"sens_id\" : \"none\",\"sens_Val\": { \"temp\":"+String(am2320.readTemperature())+", \"hum\":"+String(am2320.readHumidity())+"}}"); 
//    delay(2000);
//  } 
//  else if (val == 'H' || val == '?'){
//  Serial.println("Проверим русскую кодировку!");
//  Serial.println("Hard is ready...");
//  Serial.println("Press H or ? to help.");
//  Serial.println("Press S for scan OneWire line");
//  Serial.println("Press R for read DS18B20");
//  Serial.println("Press G for read AM2320");    
//  }else if (val == 'S'){
//
//    Serial.println("{\"sensors\":[");
//
//    byte sc_res=10;
//    while (sc_res!=0) {
//    if (sel_sens !=0 and sc_res!="") {
//      Serial.print(",");
//    }      
//    sc_res = scanner_wire1(sel_sens);
//    sel_sens++;
//  }
//  Serial.println("{}]}");  

  }

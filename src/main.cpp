#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <U8x8lib.h>

//SSID et mot de passe du wifi
const char* ssid = "SSID";
const char* password = "PASSWORD";

#define DHTPIN 23 // PIN connectée au capteur
#define DHTTYPE DHT22 //Capteur DHT22

DHT dht(DHTPIN, DHTTYPE);

//Création d'un objet AsyncWebServer sur le port 80
AsyncWebServer server(80);
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE); //OLED sans broche reset

String readDHTTemperature() {
  float t = dht.readTemperature();
  
  //Vérifier si une lecture a échoué.
  if (isnan(t)) {    
    //Serial.println("Échec de la lecture du capteur DHT");
    u8x8.drawUTF8(2,2,"Échec de");
    u8x8.drawString(1,3, "la lecture du");
    u8x8.drawString(2,4,"capteur DHT");
    return "--";
  }
  else {
    //Serial.println(t);
    u8x8.setCursor(0,1);
    u8x8.print(t);
    u8x8.drawUTF8(8,1," °C");
    return String(t);
  }
}


String readDHTHumidity() {
  float h = dht.readHumidity();

  //Vérifier si une lecture a échoué.
  if (isnan(h)) {
    //Serial.println("Échec de la lecture du capteur DHT");
    u8x8.drawUTF8(2,2,"Échec de");
    u8x8.drawString(1,3, "la lecture du");
    u8x8.drawString(2,4,"capteur DHT");
    return "--";
  }
  else {
    //Serial.println(h);
    u8x8.setCursor(0,4);
    u8x8.print(h);
    u8x8.drawUTF8(8,4," %");
    return String(h);
  }
}


//Code HTML de la page Web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT22</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Température</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidité</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";


//Remplace l'espace réservé dans la page Web par les valeurs du capteur 
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}


void setup(){
  u8x8.begin(); //Instancie l'affichage sur l'écran
  u8x8.setFont(u8x8_font_courB18_2x3_f); //police d'écriture. _f : prend en charge + de caractères
  delay(100); //temps d'initialisation de l'écran

  Serial.begin(115200);
  dht.begin();
  
  //Connexion au WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  //Affiche l'adresse IP de l'ESP
  Serial.println(WiFi.localIP());

  //Chemin d'accès
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  //Démarrage du serveur
  server.begin();
}


void loop(){
  
}
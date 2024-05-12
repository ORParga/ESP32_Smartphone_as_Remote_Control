// main_B_ROLLER
// Based on Name:		ROLLER_01.ino
// Based on Created:	25/12/2023 17:43:24
// Author:	ORParga

#include <Arduino.h>

#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

#include "ArduinoJson.h"

bool HTTP_POST_ECHO_ON= false;
bool HTTP_POST_ECHO_Json_Send_ON= true;
bool HTTP_POST_ECHO_Json_Reci_ON= false;
bool HTTP_POST_ECHO_Vari_Reci_ON= true;
AsyncWebServer server(80);

//ledBuid-in variables
#define LED_BUILD_IN_ESP32 2
long lastMilis = 0;
uint8_t build_in_led_Status = 1;
long periodMilis =500;

//Nombre de la Wifi en modo AP
const char*             g_ssid_AP = "ESP32_AP";
String                  g_conectParam, g_ssdiParam, g_passParam;
bool                    g_request_from_AP = false;
AsyncWebServerRequest*  g_request;
int                     g_intento = 0;

//HTML files
const char* HTMLfile = "/ROLLER_02.html";

String HTML_AP_STA_REQUEST_WAY = "UNKNOW";
String HTML_AP_SSDI = "UNKNOW";
String HTML_AP_IP = "UNKNOW";
String HTML_STA_SSDI = "UNKNOW";
String HTML_STA_IP = "UNKNOW";
String HTML_STA_MESSAGE = "-";
String HTML_RESP_TABLE_ROWS = "UNKNOW";

const char* PARAM_FORGET = "forget";
const char* PARAM_CONECT = "conect";
const char* PARAM_SSDI = "ssdi";
const char* PARAM_PASS = "pass";
const char* PARAM_INPUT_1 = "wifi";
const char* PARAM_INPUT_2 = "password";

//Variables para Depuracion*******************************************
int HTTP_POST_blink = 0;
int HTTP_POST_count = 0;

//Constantes para definir pines **************************************
#define L298N_ENA_LEDPIN 27
#define L298N_ENB_LEDPIN 26
#define L298N_ENA_LED_CHANEL 0
#define L298N_ENB_LED_CHANEL 1
#define L298N_IN1 25
#define L298N_IN2 33
#define L298N_IN3 32
#define L298N_IN4 14

#define L298N_ENA2_LEDPIN 22
#define L298N_ENB2_LEDPIN 23
#define L298N_ENA2_LED_CHANEL 2
#define L298N_ENB2_LED_CHANEL 3
#define L298N_IN12 17
#define L298N_IN22 5
#define L298N_IN32 18
#define L298N_IN42 19


//Vriables de control de motores
float ESP32_Y = 0, ESP32_X = 0;
float ESP32_Y2 = 0, ESP32_X2 = 0;
#define ESP32_XY_DEADZONE 5
#define ESP32_MAX  100

//VAriables que representan la velocidad de los motores
float SpeedLF=0,SpeedRF=0,SpeedLB=0,SpeedRB=0;
#include "ROLLER_Header.h"

String processor(const String& var) {
    Serial.println("processor 1");

    if (var == "HTML_AP_STA_REQUEST_WAY") {
        return HTML_AP_STA_REQUEST_WAY;
    }
    if (var == "HTML_AP_SSDI") {
        return HTML_AP_SSDI;
    }
    //if (var == "HTML_AP_IP") {
    //    return HTML_AP_IP;
    //}
    if (var == "HTML_AP_IP") {
        return HTML_AP_IP;
    }
    if (var == "HTML_STA_SSDI") {
        return HTML_STA_SSDI;
    }
    if (var == "HTML_STA_IP") {
        Serial.print("processor() HTML_STA_IP=");
        Serial.println(HTML_STA_IP);
        return HTML_STA_IP;
    }
    if (var == "HTML_STA_MESSAGE") {
        return HTML_STA_MESSAGE;
    }
    else if (var == "HTML_RESP_TABLE_ROWS") {
        return HTML_RESP_TABLE_ROWS;
    }

    return "desconocido";
}

/// <summary>
/// Inicializa el modo de conexion en WIFI_MODE_APSTA
/// y crea una red WiFi con el nombre ssid_AP
/// realiza intentos y no regresa hasta que ha conseguido crearla
/// Inicializa la variable HTML_AP_IP con el numero de IP para que sea usada en el HTML
/// </summary>
void connectWiFi_APSTA() {

    Serial.print("Creando WiFi AP:" + String(g_ssid_AP));
    //Iniciar la ventana de configuracion en el WiFi modo "AccessPoint"**********************
    WiFi.mode(WIFI_MODE_APSTA);

    int tried = 1;
    while (!WiFi.softAP(g_ssid_AP))
    {
        Serial.print(".");
        delay(100);
        tried++;
    }
    Serial.println();
    Serial.print("Iniciado AP ");
    Serial.println(g_ssid_AP);
    Serial.print("SoftIP address:\t");
    Serial.println(WiFi.softAPIP());
    IPAddress softAPIP = WiFi.softAPIP();
    uint8_t IP_array[4] = { softAPIP[0],softAPIP[1],softAPIP[2],softAPIP[3] };

    String strIP =
        String(IP_array[0]) + "." +
        String(IP_array[1]) + "." +
        String(IP_array[2]) + "." +
        String(IP_array[3]);
    HTML_AP_IP = strIP;
}

void notFound(AsyncWebServerRequest* request) {
    Serial.println("server.on(notFound)");
    request->send(404, "text/plain", "Not found");
}
void request_avaliable(AsyncWebServerRequest* request)
{
    //Comprueba si el request se ha realizado desde la WiFi AP o la WiFi STA
    if (ON_STA_FILTER(request))
    {
        Serial.println("Request from web in STA WiFi");
        HTML_AP_STA_REQUEST_WAY = "STA";
    }
    else if (ON_AP_FILTER(request))
    {
        Serial.println("Request from web in AP WiFi");
        HTML_AP_STA_REQUEST_WAY = "AP";
    }
    else
    {
        Serial.println("Request from web in UNKNOW WiFi (not STA or AP)");
        HTML_AP_STA_REQUEST_WAY = "Unknow request way";
    }
    Serial.println(" ready to send response SPIFFS, ""/ROLLER_02.html""");
    request->send(SPIFFS, "/ROLLER_02.html", "text/html", false, processor);
    Serial.println(" request sended SPIFFS, ""/ROLLER_02.html""");

}
void serverConfig() {

    

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("server.on(\"/\",...)");
        request_avaliable(request);
        });
    server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {

            if(HTTP_POST_ECHO_ON) Serial.print(HTTP_POST_count++);
            if(HTTP_POST_ECHO_ON) Serial.print(" server.on(..., HTTP_POST ...");
            digitalWrite(16, HTTP_POST_blink);
            HTTP_POST_blink = 1 - HTTP_POST_blink;
            char param[200];
            int sizeWord = len;
            if (sizeWord >= 200)sizeWord = 199;
            size_t i;
            if(HTTP_POST_ECHO_Json_Reci_ON) {
            Serial.print("Recieved by Post:");
            for (i = 0; i < sizeWord; i++) {
                param[i] = data[i];
                Serial.print(param[i]);
            }
            }
            if(HTTP_POST_ECHO_Json_Reci_ON) Serial.println();
            param[i] = 0;
            DynamicJsonDocument json_received_doc(200);
            deserializeJson(json_received_doc, data);
            if (json_received_doc.is<JsonArray>()) { if(HTTP_POST_ECHO_ON) Serial.println(".root is JsonArray."); }
            if (json_received_doc.is<JsonArrayConst>()) { if(HTTP_POST_ECHO_ON) Serial.println(".root is JsonArrayConst."); }
            if (json_received_doc.is<JsonObjectConst>()) {if(HTTP_POST_ECHO_ON)  Serial.println(".root is JsonObjectConst."); }
            if (json_received_doc.is<JsonObject>())
            {
                if(HTTP_POST_ECHO_ON) Serial.println(".root is JsonObject.");
                JsonObject root = json_received_doc.as<JsonObject>();
                if(HTTP_POST_ECHO_Vari_Reci_ON)  Serial.print("Variables Reci HTML->ESP32 =");
                if (json_received_doc.containsKey("ESP32_X"))
                {
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(" ESP32_X=");
                    ESP32_X = json_received_doc["ESP32_X"].as<float>();
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(ESP32_X);
                }
                if (json_received_doc.containsKey("ESP32_Y"))
                {
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(" ESP32_Y=");
                    ESP32_Y = json_received_doc["ESP32_Y"].as<float>();
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(ESP32_Y);
                }
                if (json_received_doc.containsKey("ESP32_X2"))
                {
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(" ESP32_X2=");
                    ESP32_X2 = json_received_doc["ESP32_X2"].as<float>();
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(ESP32_X2);
                }
                if (json_received_doc.containsKey("ESP32_Y2"))
                {
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.print(" ESP32_Y2=");
                    ESP32_Y2 = json_received_doc["ESP32_Y2"].as<float>();
                    if(HTTP_POST_ECHO_Vari_Reci_ON) Serial.println(ESP32_Y2);
                }
            }
            
            if(HTTP_POST_ECHO_ON) Serial.println();

            //The ESP32 checks if the Web is demanding the 'lightState' parameter
            if (strcmp(param, "lightState") == 0)
            {
                //The ESP32 sends an 'on' or an 'off' to the web as response to the web's request
                if (digitalRead(4))
                {
                    request->send(200, "text/html", "on");
                }
                else
                {
                    request->send(200, "text/html", "off");
                }

            }
            else {
                //the Web is NOT demanding the 'lightState' parameter                
                DynamicJsonDocument doc(1024);
                doc["answer"] = analogRead(34);
                // char buff[20];
                // buff[0]=(char)0;
                // String test="";
                // float pi=3.141592;
                // snprintf(buff,sizeof(buff),"%f",pi);
                // dtostrf(pi,1,1,buff);
                doc["LF"]=(int)SpeedLF;
                doc["RF"]=(int)SpeedRF;
                doc["LB"]=(int)SpeedLB;
                doc["RB"]=(int)SpeedRB;
                String str_doc;
                serializeJson(doc, str_doc);
                if(HTTP_POST_ECHO_Json_Send_ON){
                    Serial.print("Json Sent ESP32->HTML =");
                    Serial.println(str_doc);}
                //  response to request contains {"answer":1024}
                request->send(200, "application/json", str_doc);
            }
        });

    server.onNotFound([](AsyncWebServerRequest* request) {
        Serial.println("server.on(notFound)");
        request->send(404, "text/plain", "Not found, really.");
    });
    server.begin();
}
// the setup function: Inicializa:
// - Monitor Serial
// - Sistema de archivos SPIFFS
// - Wifi en modo AP ( a traves de APSTA)
// - server AsyncWebServer
// - GPIO's en modo INPUT o OUTPUT
// - GPIO's en modo PWM
void setup() {
    //Iniciate Serial Monitor**************************************************************
    Serial.begin(115200);
    Serial.println("hola");
    //Iniciate File System*********************************************************
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    //List contents of SPIFFS. Just to show if index.html is loaded on ESP32
    listDir(SPIFFS, "/", 0);

    //Inicialize WiFi, "**********************
    connectWiFi_APSTA();

    // Send web page to client*********************************************************
    serverConfig();
    Serial.println("Server configured");

    //Configure build-in led for output
    pinMode(LED_BUILD_IN_ESP32, OUTPUT);//Led Build-in
    pinMode(16, OUTPUT);
    pinMode(34, INPUT);//Analog Read
    pinMode(4, INPUT);



    const int frequency = 5000;  // Frecuencia en Hz
    const int resolution = 8;    // Resolución en bits (de 1 a 15)

    ledcSetup(L298N_ENA_LED_CHANEL, frequency, resolution);
    ledcAttachPin(L298N_ENA_LEDPIN, L298N_ENA_LED_CHANEL);
    ledcSetup(L298N_ENB_LED_CHANEL, frequency, resolution);
    ledcAttachPin(L298N_ENB_LEDPIN, L298N_ENB_LED_CHANEL);
    ledcSetup(L298N_ENA2_LED_CHANEL, frequency, resolution);
    ledcAttachPin(L298N_ENA2_LEDPIN, L298N_ENA2_LED_CHANEL);
    ledcSetup(L298N_ENB2_LED_CHANEL, frequency, resolution);
    ledcAttachPin(L298N_ENB2_LEDPIN, L298N_ENB2_LED_CHANEL);

    pinMode(L298N_IN1, OUTPUT);// in 1 motorA
    pinMode(L298N_IN2, OUTPUT);// in 2 motorA
    pinMode(L298N_IN3, OUTPUT);// in 3 motorB
    pinMode(L298N_IN4, OUTPUT);// in 4 motorB
    pinMode(L298N_IN12, OUTPUT);// in 1 motorC
    pinMode(L298N_IN22, OUTPUT);// in 2 motorC
    pinMode(L298N_IN32, OUTPUT);// in 3 motorD
    pinMode(L298N_IN42, OUTPUT);// in 4 motorD

}

float Motor_LF_Update ()
{
    float DesiredSpeed=ESP32_Y-ESP32_X+ESP32_Y2-ESP32_X2;
    if(DesiredSpeed>ESP32_MAX)DesiredSpeed=ESP32_MAX;
    if(DesiredSpeed<-ESP32_MAX)DesiredSpeed=-ESP32_MAX;
    float Speed= 255 * (DesiredSpeed / ESP32_MAX);

        ledcWrite(L298N_ENA_LED_CHANEL, abs(Speed));
        if(Speed==0){digitalWrite(L298N_IN1, 0);digitalWrite(L298N_IN2, 0);}
        if(Speed<0){digitalWrite(L298N_IN1, 0);digitalWrite(L298N_IN2, 1);}
        if(Speed>0){digitalWrite(L298N_IN1, 1);digitalWrite(L298N_IN2, 0);}
    return Speed;
}

float Motor_RF_Update ()
{
    float DesiredSpeed=ESP32_Y+ESP32_X+ESP32_Y2+ESP32_X2;
    if(DesiredSpeed>ESP32_MAX)DesiredSpeed=ESP32_MAX;
    if(DesiredSpeed<-ESP32_MAX)DesiredSpeed=-ESP32_MAX;
    float Speed= 255 * (DesiredSpeed / ESP32_MAX);

        ledcWrite(L298N_ENB_LED_CHANEL, abs( Speed));
        if(Speed==0){digitalWrite(L298N_IN3, 0);digitalWrite(L298N_IN4, 0);}
        if(Speed<0){digitalWrite(L298N_IN3, 0);digitalWrite(L298N_IN4, 1);}
        if(Speed>0){digitalWrite(L298N_IN3, 1);digitalWrite(L298N_IN4, 0);}
    return Speed;
}


float Motor_LB_Update ()
{
    float DesiredSpeed=ESP32_Y-ESP32_X+ESP32_Y2+ESP32_X2;
    if(DesiredSpeed>ESP32_MAX)DesiredSpeed=ESP32_MAX;
    if(DesiredSpeed<-ESP32_MAX)DesiredSpeed=-ESP32_MAX;
    float Speed= 255 * (DesiredSpeed / ESP32_MAX);

        ledcWrite(L298N_ENA2_LED_CHANEL, abs(Speed));
        if(Speed==0){digitalWrite(L298N_IN12, 0);digitalWrite(L298N_IN22, 0);}
        if(Speed<0){digitalWrite(L298N_IN12, 0);digitalWrite(L298N_IN22, 1);}
        if(Speed>0){digitalWrite(L298N_IN12, 1);digitalWrite(L298N_IN22, 0);}
    return Speed;
}

float Motor_RB_Update ()
{
    float DesiredSpeed=ESP32_Y+ESP32_X+ESP32_Y2-ESP32_X2;
    if(DesiredSpeed>ESP32_MAX)DesiredSpeed=ESP32_MAX;
    if(DesiredSpeed<-ESP32_MAX)DesiredSpeed=-ESP32_MAX;
    float Speed= 255 * (DesiredSpeed / ESP32_MAX);

        ledcWrite(L298N_ENB2_LED_CHANEL, abs( Speed));
        if(Speed==0){digitalWrite(L298N_IN32, 0);digitalWrite(L298N_IN42, 0);}
        if(Speed<0){digitalWrite(L298N_IN32, 0);digitalWrite(L298N_IN42, 1);}
        if(Speed>0){digitalWrite(L298N_IN32, 1);digitalWrite(L298N_IN42, 0);}
    
    return Speed;
}

//loop()
//  -Hace parpadear el led impreso para mostrar que el esp esta vivo
//  -Actualiza el valor de las salidas que controlan los drivers de los motores
//en funcion de las ariables globales ESP32_X,ESP32_Y,ESP32_X2,ESP32_Y2...  
//que representan la posicion de los joysticks
//  -Actualiza el valor de las variables globales 
// SpeedLF,SpeedRF,SpeedLB,SpeedRB, que representan 
//la velocidad de giro de los motores de las ruedas 
void loop() {
    //Hace parpadear el BUILD_IN led para mostrar que el ESP32 está vivo
    if (lastMilis+periodMilis < millis()) {
        lastMilis = millis();
        build_in_led_Status = 1-build_in_led_Status;
        digitalWrite(LED_BUILD_IN_ESP32,build_in_led_Status);
    }

    //Joystick en deadzone
    if ((ESP32_X > -ESP32_XY_DEADZONE)&&(ESP32_X < ESP32_XY_DEADZONE))ESP32_X=0;
    if ((ESP32_Y > -ESP32_XY_DEADZONE)&&(ESP32_Y < ESP32_XY_DEADZONE))ESP32_Y=0;
    if ((ESP32_X2 > -ESP32_XY_DEADZONE)&&(ESP32_X2 < ESP32_XY_DEADZONE))ESP32_X2=0;
    if ((ESP32_Y2 > -ESP32_XY_DEADZONE)&&(ESP32_Y2 < ESP32_XY_DEADZONE))ESP32_Y2=0;

    //Control de motores
    SpeedLF= Motor_LF_Update();
    SpeedLB= Motor_LB_Update();
    SpeedRF= Motor_RF_Update();
    SpeedRB= Motor_RB_Update();
    
    delay(10);
}
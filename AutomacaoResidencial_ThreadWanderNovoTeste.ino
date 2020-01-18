#include <SerialRelay.h>
const byte NumModulos = 1;
SerialRelay relays(4,5,NumModulos); // (data, clock, number of modules)

#include "Thread.h"
#include "ThreadController.h"

#include <Ethernet_v2.h> //para segunda geracao do shield ethernet
//#include <Ethernet.h> //para primeira geracao do shield ethernet
#include <SD.h>
#include <SPI.h>

const int carga1 = 9;// tenho que ver as cargas

byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x6F, 0x66};
IPAddress ip(192, 168, 0, 2);
EthernetServer server(80);

File webFile;

#define REQ_BUF_SZ    40
char HTTP_req[REQ_BUF_SZ] = { 0 };
char req_index = 0;
//defini abaixo o estado de cada carga inicial
int estadocarga1=0;
int estadocarga2=0;
int estadocarga3=0;
int estadocarga4=0;

ThreadController sensor;
ThreadController button1;
ThreadController button2;
ThreadController button3;
ThreadController button4;

Thread lerSensores;
const int LM35 = A0;
const int LDR = A1;
int luz = 0;
int temp = 0;

void sensores() {
  luz = map(analogRead(LDR), 0, 1023, 0, 100);
  temp = (float(analogRead(LM35))*5/(1023))/0.01; 
  //Serial.println(luz);
  //Serial.println(temp); 
}

//EthernetClient novoCliente; //tava sendo 
Thread debugDados;
EthernetClient novoCliente;
void lerDados(){
  novoCliente.print(luz);
  novoCliente.println("%");
  novoCliente.print("|");
  novoCliente.print(temp);
  novoCliente.println("°C");
  novoCliente.print("|");
  //novoCliente.print(flag1);//essa flag1 que comentei mas serve para dizer a pagina que tem que mudar a cor do botao
  //novoCliente.print("|");
  //espero receber algo como 90%|25*C|0|
  }
  
Thread botaoCarga1;  
void setCarga1(){
  if(estadocarga1==0){
    relays.SetRelay(1, SERIAL_RELAY_ON, 1);
    //Serial.println("Ligado");
    estadocarga1=1;
    }
    else{relays.SetRelay(1, SERIAL_RELAY_OFF, 1);
    estadocarga1=0;
    //Serial.println("DESLigado");
    }
    //digitalWrite(carga1, !digitalRead(carga1));
}
Thread botaoCarga2;
void setCarga2(){
  if(estadocarga2==0){
    relays.SetRelay(2, SERIAL_RELAY_ON, 1);
    //Serial.println("Ligado");
    estadocarga2=1;
    }
    else{relays.SetRelay(2, SERIAL_RELAY_OFF, 1);
    estadocarga2=0;
    //Serial.println("DESLigado");
    }
    //digitalWrite(carga2, !digitalRead(carga2));
}
Thread botaoCarga3;
void setCarga3(){
  if(estadocarga3==0){
    relays.SetRelay(3, SERIAL_RELAY_ON, 1);
    //Serial.println("Ligado");
    estadocarga3=1;
    }
    else{relays.SetRelay(3, SERIAL_RELAY_OFF, 1);
    estadocarga3=0;
    //Serial.println("DESLigado");
    }
    //digitalWrite(carga3, !digitalRead(carga3));
}
Thread botaoCarga4;
void setCarga4(){
  if(estadocarga4==0){
    relays.SetRelay(4, SERIAL_RELAY_ON, 1);
    //Serial.println("Ligado");
    estadocarga4=1;
    }
    else{relays.SetRelay(4, SERIAL_RELAY_OFF, 1);
    //Serial.println("Ligado");
    estadocarga4=0;
    }
    //digitalWrite(carga4, !digitalRead(carga4));
}
//Bruna
//88669659

void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}


char StrContains(char *str, char *sfind){
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }

  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }
  return 0;
}


void setup() {
  //comente  a linha abaixo pq tirei o modo de 1 relé que tava antes
  pinMode(carga1, OUTPUT);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);
  Serial.println("Inicializando cartao MicroSD...");
  if (!SD.begin(4)) {
    Serial.println("ERRO - iniciallizacao do cartao falhou!");
    return;
  }
  Serial.println("SUCESSO - cartao MicroSD inicializado.");

  if (!SD.exists("index.htm")) {
    Serial.println("ERRO - index.htm nao foi encontrado!");
    return;
  }
  Serial.println("SUCESSO - Encontrado arquivo index.htm.");
  
    lerSensores.setInterval(500);//vai ler nesse intervalo
    lerSensores.onRun(sensores);//to dizendo que essa função pertence a Thread sensores

    debugDados.setInterval(500);
    debugDados.onRun(lerDados);
    
    botaoCarga1.setInterval(100);
    botaoCarga1.onRun(setCarga1);
    botaoCarga2.setInterval(100);
    botaoCarga2.onRun(setCarga2);
    botaoCarga3.setInterval(100);
    botaoCarga3.onRun(setCarga3);
    botaoCarga4.setInterval(100);
    botaoCarga4.onRun(setCarga4);
    
    sensor.add(&lerSensores);
    sensor.add(&debugDados);
    button1.add(&botaoCarga1);
    button2.add(&botaoCarga2);
    button3.add(&botaoCarga3);
    button4.add(&botaoCarga4);
}

void loop() {
  //sensores();
  EthernetClient client = server.available();
  novoCliente = client;//comentei so pra fazer um teste, mas devo apagar o comentario

  if(client) {//
    //novoCliente = client;//qualquer coisa apagar aqui e descomentar acima
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {

        char c = client.read();

        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;
          req_index++;
        }

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          
          if(StrContains(HTTP_req, "ajax_carga1")){
                button1.run();
              } 
            if(StrContains(HTTP_req, "ajax_carga2")){
                button2.run();
              }
            if(StrContains(HTTP_req, "ajax_carga3")){
                button3.run();
              }
            if(StrContains(HTTP_req, "ajax_carga4")){
                button4.run();
              }

          if(StrContains(HTTP_req, "ajax_LerDados")){
            //LerDados(client);
            sensor.run();//quando eu emito esse comando toda função que esta dentro dessa thread sera lida
            
          } 
            
            else {
                webFile = SD.open("index.htm");
                  if (webFile){
                    while (webFile.available()) {
                      client.write(webFile.read());
                    }
                  webFile.close();
                  }
            }
          
          Serial.println(HTTP_req);
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
          break;
          
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();

  }

}


#include <ESP8266WiFi.h>

//defines
#define SSID_REDE     "NomedaRede"  //coloque aqui o nome da rede que se deseja conectar
#define SENHA_REDE    "SenhaAqui"  //Senha do Wifi
#define INTERVALO_ENVIO_THINGSPEAK  30000  //intervalo entre envios de dados ao ThingSpeak (em ms)
 
//constantes e variáveis globais
char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "Sua chave de API aqui"; //API
long lastConnectionTime; 
WiFiClient client;
 
//prototypes
void EnviaInformacoesThingspeak(String StringDados);
void FazConexaoWiFi(void);
float FazLeituraUmidade(void);
 
/*
 * Implementações
 */
 
//Função: envia informações ao ThingSpeak
//Parâmetros: String com a  informação a ser enviada
//Retorno: nenhum
void EnviaInformacoesThingspeak(String StringDados)
{
    if (client.connect(EnderecoAPIThingSpeak, 80))
    {         
        //faz a requisição HTTP ao ThingSpeak
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+ChaveEscritaThingSpeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(StringDados.length());
        client.print("\n\n");
        client.print(StringDados);
  
        lastConnectionTime = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
     }   
}
 
//Função: faz a conexão WiFI
//Parâmetros: nenhum
//Retorno: nenhum
void FazConexaoWiFi(void)
{
    client.stop();
    Serial.println("Conectando-se à rede WiFi...");
    Serial.println();  
    delay(1000);
    WiFi.begin(SSID_REDE, SENHA_REDE);
 
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connectado com sucesso!");  
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
 
    delay(1000);
}
 
//  Função para conexão com o Thingspark

int pino_sensor_baixo = 16;  // Sensor inferior
int pino_sensor_topo = 4;   // Sensor superior

// LEDs indicadores (apenas para acompanhar)
int pino_led_cheio = 12;
int pino_led_vazio = 14;

int tanque_qtd = 0;

// Controle da bomba
int pino_bomba = 5;  // GPIO do relé da bomba

void setup() {
  Serial.begin(9600);

  // Configuração dos sensores
  pinMode(pino_sensor_baixo, INPUT);
  pinMode(pino_sensor_topo, INPUT);

  // Configuração dos LEDs e da bomba
  pinMode(pino_led_cheio, OUTPUT);
  pinMode(pino_led_vazio, OUTPUT);
  pinMode(pino_bomba, OUTPUT);
  digitalWrite(pino_bomba, 0);

  lastConnectionTime = 0; 
    FazConexaoWiFi();
    Serial.println("Controle do fluxo de água de um tanque com ESP8266 e Thingspark");
}

void loop() {
  int estado_baixo = digitalRead(pino_sensor_baixo);
  int estado_topo = digitalRead(pino_sensor_topo);

  Serial.print("Sensor Baixo: ");
  Serial.print(estado_baixo);
  Serial.print(" | Sensor Topo: ");
  Serial.println(estado_topo);
  Serial.print(" | BOMBA: ");
  Serial.println(pino_bomba);

  // Controle da bomba
  if (estado_baixo == 0 && estado_topo == 1) {
    digitalWrite(pino_bomba, 0);  // Liga a bomba (enche o tanque)
    digitalWrite(pino_led_vazio, HIGH);
    digitalWrite(pino_led_cheio, LOW);
    Serial.println("🔵 Bomba LIGADA - Enchendo o tanque...");
    delay(10000);
  } 
  else if (estado_baixo == 1 && estado_topo == 0) {
    digitalWrite(pino_bomba, 5);  // Desliga a bomba (tanque cheio)
    digitalWrite(pino_led_cheio, HIGH);
    digitalWrite(pino_led_vazio, LOW);
    Serial.println("🛑 Bomba DESLIGADA - Tanque cheio!");
    tanque_qtd = tanque_qtd + 1;
    Serial.print("Quantidade de vezes que o tanque encheu : ");
    Serial.print(tanque_qtd);
    delay(10000);
  }

      //Força desconexão ao ThingSpeak (se ainda estiver desconectado)
    if (client.connected())
    {
        client.stop();
        Serial.println("- Desconectado do ThingSpeak");
        Serial.println();
    }

    //verifica se está conectado no WiFi e se é o momento de enviar dados ao ThingSpeak
    if(!client.connected() && 
      (millis() - lastConnectionTime > INTERVALO_ENVIO_THINGSPEAK))
    {
        EnviaInformacoesThingspeak("field1=" + String(tanque_qtd));
    }

  delay(500);
}

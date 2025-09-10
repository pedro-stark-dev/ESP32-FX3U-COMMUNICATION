#include <WiFi.h>
#include <WebServer.h>
#include <ModbusMaster.h>

ModbusMaster node;

// UART1 para Modbus RTU
#define RX1 16
#define TX1 17

// Configurações WiFi
const char* ssid = "TP-LINK_46FE36";
const char* password = "";

// Servidor web na porta 80
WebServer server(80);

// Estados atuais dos M0..M6
bool mStates[7] = {false, false, false, false, false, false, false};

void setup() {
  Serial.begin(19200, SERIAL_8N1);
  while (!Serial);

  // UART1 para Modbus RTU com paridade Even
  Serial1.begin(19200, SERIAL_8E1, RX1, TX1);
  node.begin(1, Serial1); // Slave ID do FX3U

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.println(WiFi.localIP());

  // Rotas do servidor
  server.on("/", handleRoot);
  for (int i = 0; i <= 6; i++) {
    server.on(("/m" + String(i)).c_str(), [i]() { toggleM(i); });
  }

  server.begin();
  Serial.println("Servidor web iniciado!");
}

void loop() {
  server.handleClient();
}

// Página web com botões
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>CLP FX3U</title></head><body>";
  html += "<h1>Controle M0..M6</h1>";

  for (int i = 0; i <= 6; i++) {
    String label = "M" + String(i) + (mStates[i] ? " (ON)" : " (OFF)");
    html += "<form action='/m" + String(i) + "' method='GET'>"
            "<button type='submit'>" + label + "</button></form>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Função para alternar estado
void toggleM(uint8_t m) {
  mStates[m] = !mStates[m]; // inverte o estado
  uint8_t result = node.writeSingleCoil(m, mStates[m] ? 1 : 0);

  String message = (result == node.ku8MBSuccess)
                   ? "M" + String(m) + (mStates[m] ? " ligado!" : " desligado!")
                   : "Erro ao escrever M" + String(m);

  Serial.println(message);
  server.sendHeader("Location", "/");
  server.send(303); // Redireciona para a página principal
}

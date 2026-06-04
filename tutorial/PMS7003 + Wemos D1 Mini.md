# Monitoramento de Material Particulado com PMS7003 e Wemos D1 Mini
Este repositório contém o guia e os códigos necessários para integrar o sensor de material particulado PMS7003 a uma placa Wemos D1 Mini, enviando os dados para um Raspberry Pi 3B+.

## 🛠️ Estrutura do Hardware
* Sensor: PMS7003 (Plantower).

* Microcontrolador: Wemos D1 Mini (ESP8266).

* Host: Raspberry Pi 3B+ (via USB).
## 🔌 Passo 0: Conexões Físicas (Hardware)
Antes de colocar a mão no terminal, precisamos garantir que o Wemos D1 Mini esteja conversando corretamente com o PMS7003. Para isso, utilizaremos jumpers para conectar o sensor à placa.

PMS7003&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;WEMOS D1 MINI <br>
&nbsp;VCC ---------- 5V <br>
&nbsp;GND ---------- GND <br>
&nbsp;&nbsp;RX  ---------- D6 (GPIO12) <br>
&nbsp;&nbsp;TX  ---------- D7 (GPIO13) <br>
## 🚀 Passo 1: Preparação do Sistema (Raspberry Pi)
Antes de programar a placa, precisamos garantir que o Raspberry Pi tenha as ferramentas necessárias e permissão para acessar as portas USB.

### 1.1. Atualização do Sistema
```
sudo apt update && sudo apt upgrade -y
```
### 1.2. Configuração de Permissões (udev rules)
O Linux, por padrão, restringe o acesso direto a dispositivos USB. As regras do PlatformIO permitem que você faça o upload do código sem precisar de sudo.

```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo service udev restart
sudo udevadm control --reload-rules
sudo udevadm trigger
```
## 🐍 Passo 2: Ambiente Python e Bibliotecas
Para processar os dados que chegarão no Raspberry Pi, utilizamos um ambiente virtual para evitar conflitos de dependências.

```
python3 -m venv env
source env/bin/activate
```

## Instala bibliotecas para comunicação serial, manipulação de dados e MQTT
* pyserial: Essencial para ler os dados da Wemos via porta USB.
* pandas: Útil para organizar os logs de poeira em tabelas/CSVs.
```
pip install pyserial pandas paho-mqtt
```

## 💻 Passo 3: Instalação do PlatformIO Core
O PlatformIO é a ferramenta de linha de comando (CLI) que usaremos para compilar e subir o código para a Wemos.

```
# Instalação via script oficial
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
```
```
# Exporta o PIO para o seu PATH (ajuste conforme seu diretório)
source /home/lactea1/.platformio/penv/bin/activate
```
## 📂 Passo 4: Configuração do Projeto (Firmware)
Crie a estrutura do projeto para a Wemos D1 Mini.

### 4.1. Estrutura de Pastas
```
mkdir -p IC_Daniel/wemos_pms7003/src
cd IC_Daniel/wemos_pms7003
```
### 4.2. Arquivo de Configuração (platformio.ini)
Este arquivo diz ao PIO qual é a placa e quais bibliotecas usar.

```
[env:d1_mini_pro]
platform = espressif8266
board = d1_mini_pro
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient
```
### 4.3. Código Fonte (src/main.cpp)
```
#include <SoftwareSerial.h>

SoftwareSerial pmsSerial(D7, D6);

#define FRAME_LENGTH 32

uint8_t buffer[FRAME_LENGTH];

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600);
  delay(2000);
  Serial.println("Iniciando PMS7003...");
}

void loop() {

  if (pmsSerial.available() >= FRAME_LENGTH) {

    if (pmsSerial.peek() == 0x42) {

      pmsSerial.readBytes(buffer, FRAME_LENGTH);

      if (buffer[0] == 0x42 && buffer[1] == 0x4D) {

        uint16_t pm1  = (buffer[10] << 8) | buffer[11];
        uint16_t pm25 = (buffer[12] << 8) | buffer[13];
        uint16_t pm10 = (buffer[14] << 8) | buffer[15];

        Serial.print("PM1.0: ");
        Serial.print(pm1);
        Serial.print("  PM2.5: ");
        Serial.print(pm25);
        Serial.print("  PM10: ");
        Serial.println(pm10);
      }

    } else {
      pmsSerial.read(); // descarta byte desalinhado
    }
  }
}
```
### ⚡ Passo 5: Compilação e Flashing
Com a Wemos conectada ao Raspberry Pi via USB:

```
# Compila o código e verifica erros
pio run
```
```
# Faz o upload (flash) para a placa
pio run --target upload
```
```
# Abre o monitor serial para ver os dados do PMS7003 chegando
pio device monitor
```

# Sistema de Sensores com Raspberry Pi, MQTT, PlatformIO, Node-RED, InfluxDB e Grafana

Este repositório organiza a instalação em três módulos executados em sequência:

1. **Módulo 1 — Hotspot do Raspberry Pi**: configura o Raspberry Pi como ponto de acesso Wi-Fi para os sensores.
2. **Módulo 2 — MQTT + PlatformIO + projetos dos sensores**: instala Mosquitto, cria o logger Python e gera os projetos PlatformIO dos sensores Alphasense e PMS7003.
3. **Módulo 3 — Node-RED, InfluxDB e Grafana**: instala as ferramentas de visualização e armazenamento para o dashboard.

> **Atenção:** antes de executar os dois primeiros scripts, altere o nome da rede e a senha nos campos indicados. O nome da rede e a senha devem ser os mesmos no Módulo 1 e no Módulo 2.

---

## Estrutura dos arquivos

```bash
ic-sensores-raspberry/
├── 01_setup_hotspot.sh
├── 02_setup_pio_mqtt.sh
├── 03_setup_ing.sh
└── README.md
```

---

## Requisitos

Este tutorial foi pensado para Raspberry Pi OS ou sistema Debian semelhante em um Raspberry Pi com:

- acesso à internet durante a instalação;
- interface Wi-Fi, normalmente `wlan0`;
- internet chegando por cabo Ethernet, normalmente `eth0`;
- permissão de administrador para executar comandos com `sudo`.

Antes de começar, atualize o sistema:

```bash
sudo apt update
sudo apt upgrade -y
```

---

## 1. Baixar ou criar os arquivos

Crie uma pasta para o projeto:

```bash
mkdir -p ~/ic-sensores-raspberry
cd ~/ic-sensores-raspberry
```

Depois, copie os três scripts deste repositório para dentro dessa pasta.

Dê permissão de execução:

```bash
chmod +x 01_setup_hotspot.sh
chmod +x 02_setup_pio_mqtt.sh
chmod +x 03_setup_ing.sh
```

---

## 2. Módulo 1 — Configuração do hotspot

O primeiro script configura o Raspberry Pi como hotspot. Ele instala e configura `hostapd`, `dnsmasq`, `iptables`, `dhcpcd5` e serviços auxiliares para manter o Wi-Fi desbloqueado e com economia de energia desativada.

Antes de executar, abra o arquivo:

```bash
nano 01_setup_hotspot.sh
```

Altere estas linhas:

```bash
SSID="NOME_DA_SUA_REDE"
PASSWORD="SENHA_FORTE_AQUI"
```

Também é possível alterar o IP local do Raspberry no hotspot, caso queira usar outra faixa de rede privada:

```bash
AP_IP="10.10.10.1"
AP_CIDR="10.10.10.1/24"
DHCP_START="10.10.10.10"
DHCP_END="10.10.10.50"
```

Execute:

```bash
sudo bash 01_setup_hotspot.sh
```

Verifique se o hotspot subiu corretamente:

```bash
nmcli dev status
systemctl status hostapd --no-pager
systemctl status dnsmasq --no-pager
rfkill list
iw dev wlan0 get power_save
ip addr show wlan0
iw dev
```

Resultado esperado:

- `hostapd` ativo;
- `dnsmasq` ativo;
- `wlan0` com IP configurado;
- Wi-Fi operando como `type AP`;
- rede Wi-Fi aparecendo para os sensores ESP8266.

Depois desse módulo, recomenda-se reiniciar:

```bash
sudo reboot
```

---

## 3. Módulo 2 — MQTT, logger Python e PlatformIO

O segundo script instala as ferramentas principais do sistema de sensores:

- `mosquitto` e `mosquitto-clients`;
- ambiente Python virtual;
- biblioteca `paho-mqtt`;
- logger Python para salvar dados em CSV e JSON Lines;
- PlatformIO;
- projeto PlatformIO para sensores Alphasense NO2/OX;
- projeto PlatformIO para PMS7003.

Antes de executar, abra o arquivo:

```bash
nano 02_setup_pio_mqtt.sh
```

Altere estas linhas para os mesmos valores usados no Módulo 1:

```bash
WIFI_SSID="NOME_DA_SUA_REDE"
WIFI_PASS="SENHA_FORTE_AQUI"
```

Confirme também se o IP do broker MQTT é o mesmo IP definido no hotspot:

```bash
MQTT_HOST="10.10.10.1"
MQTT_PORT="1883"
```

Execute:

```bash
sudo bash 02_setup_pio_mqtt.sh
```

Esse módulo cria a estrutura:

```bash
~/IC_SENSORES/
├── scripts/
│   └── mqtt_sensor_logger.py
├── data/
│   ├── no2/
│   ├── ox/
│   ├── pms/
│   └── json/
├── alphasense/
│   ├── platformio.ini
│   └── src/main.cpp
└── pms/
    ├── platformio.ini
    └── src/main.cpp
```

Depois da instalação do PlatformIO, atualize o terminal:

```bash
source ~/.bashrc
```

ou feche e abra o terminal novamente.

### Testar o Mosquitto

Teste se o broker MQTT está ouvindo:

```bash
systemctl status mosquitto --no-pager
sudo ss -ltnp | grep 1883
```

Assine todos os tópicos dos sensores:

```bash
mosquitto_sub -h localhost -t 'sensores/#' -v
```

Para testar a partir da rede do hotspot:

```bash
mosquitto_sub -h 10.10.10.1 -t 'sensores/#' -v
```

### Compilar os projetos PlatformIO

Alphasense NO2:

```bash
cd ~/IC_SENSORES/alphasense
pio run -e no2
```

Alphasense OX:

```bash
cd ~/IC_SENSORES/alphasense
pio run -e ox
```

PMS7003:

```bash
cd ~/IC_SENSORES/pms
pio run -e pms7003
```

### Enviar código para as placas

Liste as portas conectadas:

```bash
pio device list
```

Upload Alphasense NO2:

```bash
cd ~/IC_SENSORES/alphasense
pio run -e no2 -t upload
```

Upload Alphasense OX:

```bash
cd ~/IC_SENSORES/alphasense
pio run -e ox -t upload
```

Upload PMS7003:

```bash
cd ~/IC_SENSORES/pms
pio run -e pms7003 -t upload
```

Monitor serial:

```bash
pio device monitor -p /dev/ttyUSB0 -b 115200
```

A porta pode variar, por exemplo `/dev/ttyUSB0`, `/dev/ttyUSB1` ou `/dev/ttyACM0`.

### Rodar o logger Python manualmente

```bash
~/IC_SENSORES/scripts/mqtt_sensor_logger.py
```

Os dados são salvos em:

```bash
~/IC_SENSORES/data/no2/NO2_B43F.csv
~/IC_SENSORES/data/ox/OX_B431.csv
~/IC_SENSORES/data/pms/PMS7003.csv
~/IC_SENSORES/data/json/sensores.jsonl
```

---

## 4. Módulo 3 — Node-RED, InfluxDB e Grafana

O terceiro script instala:

- Node-RED;
- InfluxDB 2;
- Grafana.

Ele **não instala MQTT**, porque o Mosquitto já é instalado no Módulo 2.

Execute:

```bash
sudo bash 03_setup_ing.sh
```

Depois da instalação, acesse pelo navegador usando o IP configurado no hotspot:

```text
Node-RED:  http://10.10.10.1:1880
InfluxDB:  http://10.10.10.1:8086
Grafana:   http://10.10.10.1:3000
```

> **Importante:** o Node-RED deve ser configurado manualmente neste terceiro módulo. O script apenas instala e habilita o serviço. A criação dos fluxos, conexão com o broker MQTT, configuração dos nós do InfluxDB e montagem dos dashboards deve ser feita pela interface web.

No InfluxDB, configure manualmente:

- usuário;
- senha;
- organização;
- bucket;
- token de acesso.

No Grafana, a credencial padrão inicial geralmente é:

```text
usuário: admin
senha: admin
```

Após o primeiro login, altere a senha.

Verifique os serviços:

```bash
systemctl status nodered.service --no-pager
systemctl status influxdb --no-pager
systemctl status grafana-server --no-pager
```

---

## 5. Ordem correta de execução

A ordem recomendada é:

```bash
sudo bash 01_setup_hotspot.sh
sudo reboot
```

Depois de reconectar no Raspberry:

```bash
cd ~/ic-sensores-raspberry
sudo bash 02_setup_pio_mqtt.sh
source ~/.bashrc
```

Por fim:

```bash
sudo bash 03_setup_ing.sh
```

---

## 6. Observações importantes

- O `SSID` e a `PASSWORD` do Módulo 1 definem a rede criada pelo Raspberry.
- O `WIFI_SSID` e o `WIFI_PASS` do Módulo 2 são gravados nos códigos dos ESP8266, por isso devem ser iguais aos valores do Módulo 1.
- O `MQTT_HOST` do Módulo 2 deve ser o IP do Raspberry dentro da rede hotspot.
- O IP `10.10.10.1` usado neste tutorial é apenas um exemplo genérico de rede privada.
- Não use senhas reais em repositórios públicos.
- Para publicar esse material em GitHub, mantenha os scripts com valores genéricos e deixe as credenciais reais apenas no Raspberry.

---

## 7. Scripts

Os scripts completos estão disponíveis na seguinte pasta:

https://github.com/emiliomercuri/IoT-Lactea/tree/main/tutorial


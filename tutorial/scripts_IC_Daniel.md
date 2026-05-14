# IC Daniel — Instalação do Raspberry Pi para sensores via MQTT

Este tutorial organiza a instalação em **três módulos**, na ordem recomendada:

1. **Módulo 1 — Hotspot do Raspberry Pi**
2. **Módulo 2 — PlatformIO, MQTT/Mosquitto, logger Python e projetos dos sensores**
3. **Módulo 3 — Node-RED, InfluxDB e Grafana**

> **Atenção antes de começar:** os **dois primeiros scripts precisam ser editados antes da execução** para configurar corretamente o nome da rede e a senha.  
> No **Módulo 1**, altere `SSID` e `PASSWORD`.  
> No **Módulo 2**, altere `WIFI_SSID` e `WIFI_PASS` para ficarem **iguais** ao `SSID` e `PASSWORD` do hotspot criado no Módulo 1.

---

## Visão geral do sistema

A ideia do sistema é transformar o Raspberry Pi em um ponto central de coleta:

- o Raspberry cria uma rede Wi-Fi própria;
- os ESP8266 dos sensores conectam nessa rede;
- os sensores enviam dados por MQTT para o Raspberry;
- o Raspberry salva os dados em arquivos `.csv` e `.jsonl`;
- Node-RED, InfluxDB e Grafana são instalados para visualização e armazenamento em banco de séries temporais.

---

## Pré-requisitos

Antes de executar os scripts, recomenda-se usar um Raspberry Pi com:

- Raspberry Pi OS instalado;
- acesso à internet via cabo Ethernet, se possível;
- terminal com acesso `sudo`;
- interface Wi-Fi disponível como `wlan0`;
- interface de internet principal como `eth0`.

Caso suas interfaces tenham nomes diferentes, altere as variáveis:

```bash
WLAN_IF="wlan0"
INTERNET_IF="eth0"
```

no script do Módulo 1.

---

# Módulo 1 — Configuração do hotspot

Este módulo configura o Raspberry Pi como ponto de acesso Wi-Fi. Ele instala e configura `hostapd`, `dnsmasq`, `dhcpcd5`, regras de NAT e serviços auxiliares para manter o Wi-Fi desbloqueado e com economia de energia desativada.

## 1. Criar o arquivo do script

Crie o arquivo:

```bash
micro setup_hotspot.sh
```

Cole o código abaixo:

```bash
#!/bin/bash
set -e

# ============================================================
# MÓDULO 1 - CONFIGURAÇÃO DO HOTSPOT DO RASPBERRY PI
# Projeto IC - Sensores NO2-B43F, OX-B431 e PMS7003
# ============================================================

# -------------------------
# CONFIGURAÇÕES DO HOTSPOT
# -------------------------
SSID="lactea0_daniel"
PASSWORD="iotempire0"        # mínimo 8 caracteres
WLAN_IF="wlan0"
INTERNET_IF="eth0"

AP_IP="192.168.4.1"
AP_CIDR="192.168.4.1/24"
DHCP_START="192.168.4.10"
DHCP_END="192.168.4.50"

echo "===================================================="
echo "Configurando hotspot do Raspberry Pi..."
echo "SSID: $SSID"
echo "IP do Raspberry no hotspot: $AP_IP"
echo "===================================================="

# -------------------------
# VERIFICAÇÃO DE ROOT
# -------------------------
if [ "$EUID" -ne 0 ]; then
  echo "Execute este script com sudo:"
  echo "sudo bash setup_ic.sh"
  exit 1
fi

# -------------------------
# ATUALIZAÇÃO E PACOTES
# -------------------------
echo "[1/11] Atualizando sistema e instalando dependências..."

export DEBIAN_FRONTEND=noninteractive

apt update
apt upgrade -y

apt install -y \
  dhcpcd5 \
  hostapd \
  dnsmasq \
  iptables \
  iptables-persistent \
  rfkill \
  wireless-tools \
  iw \
  network-manager

# -------------------------
# IMPEDIR CONFLITO COM NETWORKMANAGER
# -------------------------
echo "[2/11] Configurando NetworkManager para ignorar $WLAN_IF..."

mkdir -p /etc/NetworkManager/conf.d

cat > /etc/NetworkManager/conf.d/unmanaged-wlan0.conf <<EOF
[keyfile]
unmanaged-devices=interface-name:$WLAN_IF;interface-name:p2p-dev-$WLAN_IF
EOF

systemctl restart NetworkManager || true

# -------------------------
# EVITAR CONFLITO COM WPA_SUPPLICANT
# -------------------------
echo "[3/11] Parando e mascarando wpa_supplicant para evitar conflito com $WLAN_IF..."

systemctl stop wpa_supplicant || true
systemctl disable wpa_supplicant || true
systemctl mask wpa_supplicant || true

# -------------------------
# SERVIÇO PARA DESBLOQUEAR WIFI
# -------------------------
echo "[4/11] Criando serviço wifi-unblock..."

cat > /etc/systemd/system/wifi-unblock.service <<EOF
[Unit]
Description=Garantir desbloqueio do Wi-Fi via rfkill
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/rfkill unblock all
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable wifi-unblock.service
systemctl start wifi-unblock.service || true

rfkill unblock all || true

# -------------------------
# DESATIVAR POWER SAVE DO WI-FI
# -------------------------
echo "[5/11] Criando serviço para desativar power save do Wi-Fi..."

cat > /etc/systemd/system/wlan0-powersave-off.service <<EOF
[Unit]
Description=Disable wlan0 power save
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/iw dev $WLAN_IF set power_save off
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable wlan0-powersave-off.service
systemctl start wlan0-powersave-off.service || true

iw dev "$WLAN_IF" set power_save off || true

# -------------------------
# CONFIGURAÇÃO DO DHCPCD
# -------------------------
echo "[6/11] Configurando IP fixo em $WLAN_IF..."

if [ -f /etc/dhcpcd.conf ]; then
  cp /etc/dhcpcd.conf /etc/dhcpcd.conf.backup.$(date +%Y%m%d_%H%M%S)

  # Remove bloco antigo do hotspot, caso exista
  sed -i '/# === IC HOTSPOT START ===/,/# === IC HOTSPOT END ===/d' /etc/dhcpcd.conf

  cat >> /etc/dhcpcd.conf <<EOF

# === IC HOTSPOT START ===
interface $WLAN_IF
    static ip_address=$AP_CIDR
    nohook wpa_supplicant
# === IC HOTSPOT END ===
EOF

  systemctl restart dhcpcd || service dhcpcd restart || true
fi

# Garante IP imediatamente mesmo se dhcpcd/netplan demorar
ip link set "$WLAN_IF" up || true
ip addr flush dev "$WLAN_IF" || true
ip addr add "$AP_CIDR" dev "$WLAN_IF" || true

# -------------------------
# CONFIGURAÇÃO DO DNSMASQ
# -------------------------
echo "[7/11] Configurando dnsmasq..."

if [ -f /etc/dnsmasq.conf ] && [ ! -f /etc/dnsmasq.conf.original ]; then
  mv /etc/dnsmasq.conf /etc/dnsmasq.conf.original
fi

cat > /etc/dnsmasq.conf <<EOF
interface=$WLAN_IF
bind-interfaces
dhcp-range=$DHCP_START,$DHCP_END,255.255.255.0,24h
domain-needed
bogus-priv
EOF

# -------------------------
# CONFIGURAÇÃO DO HOSTAPD
# -------------------------
echo "[8/11] Configurando hostapd..."

cat > /etc/hostapd/hostapd.conf <<EOF
interface=$WLAN_IF
driver=nl80211

ssid=$SSID
country_code=BR
hw_mode=g
channel=6

wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0

wpa=2
wpa_passphrase=$PASSWORD
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
EOF

# Aponta o arquivo de configuração do hostapd
if [ -f /etc/default/hostapd ]; then
  if grep -q "^#DAEMON_CONF=" /etc/default/hostapd; then
    sed -i 's|^#DAEMON_CONF=.*|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd
  elif grep -q "^DAEMON_CONF=" /etc/default/hostapd; then
    sed -i 's|^DAEMON_CONF=.*|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd
  else
    echo 'DAEMON_CONF="/etc/hostapd/hostapd.conf"' >> /etc/default/hostapd
  fi
fi

# -------------------------
# ATIVAR IP FORWARDING
# -------------------------
echo "[9/11] Ativando encaminhamento de IP..."

cat > /etc/sysctl.d/99-ipforward.conf <<EOF
net.ipv4.ip_forward=1
EOF

sysctl --system

# -------------------------
# CONFIGURAÇÃO DO NAT
# -------------------------
echo "[10/11] Configurando NAT para compartilhar internet via $INTERNET_IF..."

iptables -t nat -C POSTROUTING -o "$INTERNET_IF" -j MASQUERADE 2>/dev/null || \
iptables -t nat -A POSTROUTING -o "$INTERNET_IF" -j MASQUERADE

iptables-save > /etc/iptables.ipv4.nat

cat > /etc/rc.local <<EOF
#!/bin/sh -e
iptables-restore < /etc/iptables.ipv4.nat
/usr/sbin/rfkill unblock all
/usr/sbin/iw dev $WLAN_IF set power_save off || true
exit 0
EOF

chmod +x /etc/rc.local

# Serviço extra para garantir IP forwarding no boot
cat > /etc/systemd/system/ipforward.service <<EOF
[Unit]
Description=Enable IP forwarding for hotspot
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/sysctl -w net.ipv4.ip_forward=1
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable ipforward.service

# -------------------------
# ATIVAR SERVIÇOS
# -------------------------
echo "[11/11] Ativando hostapd e dnsmasq..."

systemctl unmask hostapd || true
systemctl enable hostapd
systemctl enable dnsmasq

# Reinício limpo dos serviços
systemctl stop hostapd || true
systemctl stop dnsmasq || true

rfkill unblock all || true
ip link set "$WLAN_IF" up || true
ip addr flush dev "$WLAN_IF" || true
ip addr add "$AP_CIDR" dev "$WLAN_IF" || true
iw dev "$WLAN_IF" set power_save off || true

systemctl start dnsmasq
systemctl start hostapd

echo "===================================================="
echo "HOTSPOT CONFIGURADO COM SUCESSO!"
echo "Rede Wi-Fi: $SSID"
echo "Senha: $PASSWORD"
echo "IP do Raspberry: $AP_IP"
echo ""
echo "Verificações recomendadas:"
echo "  nmcli dev status"
echo "  systemctl status hostapd --no-pager"
echo "  systemctl status dnsmasq --no-pager"
echo "  rfkill list"
echo "  iw dev $WLAN_IF get power_save"
echo "  ip addr show $WLAN_IF"
echo "  iw dev"
echo "  iw dev $WLAN_IF station dump"
echo ""
echo "Resultado esperado:"
echo "  wlan0 unmanaged no NetworkManager"
echo "  Power save: off"
echo "  hostapd active (running)"
echo "  dnsmasq active (running)"
echo "  wlan0 com IP $AP_CIDR"
echo "  iw dev mostrando type AP e ssid $SSID"
echo "===================================================="
```

## 2. Editar nome e senha da rede

Antes de executar, altere este trecho no início do script:

```bash
SSID="lactea0_daniel"
PASSWORD="iotempire0"
```

Exemplo:

```bash
SSID="IC_SENSORES"
PASSWORD="sensores123"
```

A senha precisa ter **pelo menos 8 caracteres**.

> Guarde exatamente esse nome e essa senha, porque eles deverão ser repetidos no Módulo 2.

## 3. Executar o Módulo 1

Execute com:

```bash
sudo bash setup_hotspot.sh
```

Não execute com `sh setup_hotspot.sh`.

## 4. Verificar se o hotspot funcionou

Depois da execução, confira:

```bash
systemctl status hostapd --no-pager
systemctl status dnsmasq --no-pager
ip addr show wlan0
iw dev
iw dev wlan0 get power_save
nmcli dev status
```

Resultado esperado:

- `hostapd` ativo;
- `dnsmasq` ativo;
- `wlan0` com IP `192.168.4.1/24`;
- `Power save: off`;
- `wlan0` ignorada pelo NetworkManager;
- rede Wi-Fi aparecendo com o nome configurado em `SSID`.

Se necessário, reinicie o Raspberry:

```bash
sudo reboot
```

---

# Módulo 2 — MQTT, PlatformIO, logger Python e projetos dos sensores

Este módulo instala ferramentas básicas, Mosquitto MQTT, PlatformIO, cria a estrutura da pasta `IC_Daniel`, cria o logger Python e gera os projetos PlatformIO dos sensores Alphasense e PMS7003.

## 1. Criar o arquivo do script

Crie o arquivo:

```bash
micro setup_pio_mqtt.sh
```

Cole o código abaixo:

```bash
#!/usr/bin/env bash

# ============================================================
# MÓDULO 2 - INSTALAÇÃO DE FERRAMENTAS E CRIAÇÃO DOS PROJETOS
# IC Daniel - Raspberry Pi + MQTT + PlatformIO
# Versão ajustada:
# - Wi-Fi em baixa potência para ESP8266
# - Payload MQTT enxuto, sem envio de constantes de calibração
# - PMS7003 com parser manual e SoftwareSerial
# - Alphasense com envio apenas de variáveis úteis para análise
# ============================================================

USERNAME="${SUDO_USER:-$USER}"
USER_HOME="$(eval echo ~$USERNAME)"

BASE_DIR="$USER_HOME/IC_Daniel"
SCRIPTS_DIR="$BASE_DIR/scripts"
DATA_DIR="$BASE_DIR/data"
ALPHASENSE_DIR="$BASE_DIR/alphasense"
PMS_DIR="$BASE_DIR/pms"

# ============================================================
# CONFIGURAÇÕES GERAIS
# Altere aqui se mudar o nome/senha do hotspot
# ============================================================

WIFI_SSID="IC_SENSORES"
WIFI_PASS="sensores123"

MQTT_HOST="192.168.4.1"
MQTT_PORT="1883"

echo "===================================================="
echo "Instalando ferramentas e criando estrutura da IC..."
echo "Usuário: $USERNAME"
echo "Diretório base: $BASE_DIR"
echo "Wi-Fi SSID: $WIFI_SSID"
echo "MQTT HOST: $MQTT_HOST"
echo "===================================================="

if [ "$EUID" -ne 0 ]; then
  echo "Execute com sudo:"
  echo "sudo bash setup_ic.sh"
  exit 1
fi

# -------------------------
# PACOTES BÁSICOS
# -------------------------
echo "[1/8] Instalando micro, Python, pip, venv, curl, git e MQTT..."

apt update
apt install -y \
  micro \
  python3 \
  python3-pip \
  python3-venv \
  curl \
  git \
  mosquitto \
  mosquitto-clients

# -------------------------
# CONFIGURAÇÃO DO MOSQUITTO
# -------------------------
echo "[2/8] Configurando Mosquitto para conexões externas..."

MOSQ_CONF="/etc/mosquitto/mosquitto.conf"
MOSQ_CONF_BAK="/etc/mosquitto/mosquitto.conf.bak"

# Faz backup do arquivo original, se ainda não existir
if [ ! -f "$MOSQ_CONF_BAK" ]; then
  cp "$MOSQ_CONF" "$MOSQ_CONF_BAK"
fi

# Remove configurações extras para evitar listener duplicado na porta 1883
rm -f /etc/mosquitto/conf.d/*.conf

# Recria a configuração principal do Mosquitto diretamente em /etc/mosquitto/mosquitto.conf
cat > "$MOSQ_CONF" <<EOF
pid_file /run/mosquitto/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log
log_dest stdout

listener 1883 0.0.0.0
allow_anonymous true
EOF

systemctl enable mosquitto
systemctl reset-failed mosquitto
systemctl restart mosquitto

# -------------------------
# CRIAÇÃO DAS PASTAS
# -------------------------
echo "[3/8] Criando estrutura de diretórios..."

mkdir -p "$SCRIPTS_DIR"
mkdir -p "$DATA_DIR/no2"
mkdir -p "$DATA_DIR/ox"
mkdir -p "$DATA_DIR/pms"
mkdir -p "$DATA_DIR/json"
mkdir -p "$ALPHASENSE_DIR/src"
mkdir -p "$PMS_DIR/src"

chown -R "$USERNAME:$USERNAME" "$BASE_DIR"

# -------------------------
# AMBIENTE PYTHON
# -------------------------
echo "[4/8] Criando ambiente Python e instalando paho-mqtt..."

if [ ! -d "$BASE_DIR/venv" ]; then
  sudo -u "$USERNAME" python3 -m venv "$BASE_DIR/venv"
fi

sudo -u "$USERNAME" "$BASE_DIR/venv/bin/pip" install --upgrade pip
sudo -u "$USERNAME" "$BASE_DIR/venv/bin/pip" install paho-mqtt

# -------------------------
# SCRIPT PYTHON LOGGER MQTT
# -------------------------
echo "[5/8] Criando script Python de registro MQTT..."

cat > "$SCRIPTS_DIR/mqtt_sensor_logger.py" <<'EOF'
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import csv
import json
import time
import signal
from pathlib import Path
from datetime import datetime

import paho.mqtt.client as mqtt

MQTT_HOST = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE = 60

TOPIC_NO2 = "sensores/alphasense/no2"
TOPIC_OX = "sensores/alphasense/ox"
TOPIC_PMS_BASE = "sensores/pms7003"

BASE_DIR = Path.home() / "IC_Daniel" / "data"

NO2_DIR = BASE_DIR / "no2"
OX_DIR = BASE_DIR / "ox"
PMS_DIR = BASE_DIR / "pms"
JSON_DIR = BASE_DIR / "json"

NO2_FILE = NO2_DIR / "NO2_B43F.csv"
OX_FILE = OX_DIR / "OX_B431.csv"
PMS_FILE = PMS_DIR / "PMS7003.csv"
JSON_FILE = JSON_DIR / "sensores.jsonl"

RUNNING = True


def now_timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def ensure_directories():
    NO2_DIR.mkdir(parents=True, exist_ok=True)
    OX_DIR.mkdir(parents=True, exist_ok=True)
    PMS_DIR.mkdir(parents=True, exist_ok=True)
    JSON_DIR.mkdir(parents=True, exist_ok=True)


def ensure_csv_headers():
    if not NO2_FILE.exists():
        with open(NO2_FILE, "w", newline="", encoding="utf-8") as f:
            csv.writer(f).writerow([
                "timestamp_rpi",
                "timestamp_ms",
                "sensor_type",
                "board_id",
                "topic",
                "we_raw_mv",
                "aux_raw_mv",
                "corrected_signal_mv",
                "concentration_ppb",
                "rssi"
            ])

    if not OX_FILE.exists():
        with open(OX_FILE, "w", newline="", encoding="utf-8") as f:
            csv.writer(f).writerow([
                "timestamp_rpi",
                "timestamp_ms",
                "sensor_type",
                "board_id",
                "topic",
                "we_raw_mv",
                "aux_raw_mv",
                "corrected_signal_mv",
                "concentration_ppb",
                "rssi"
            ])

    if not PMS_FILE.exists():
        with open(PMS_FILE, "w", newline="", encoding="utf-8") as f:
            csv.writer(f).writerow([
                "timestamp_rpi",
                "timestamp_ms",
                "sensor_type",
                "board_id",
                "topic",
                "pm1_0",
                "pm2_5",
                "pm10_0",
                "rssi"
            ])


def append_row(file_path, row):
    with open(file_path, "a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(row)
        f.flush()
        os.fsync(f.fileno())


def append_jsonl(payload):
    with open(JSON_FILE, "a", encoding="utf-8") as f:
        f.write(json.dumps(payload, ensure_ascii=False) + "\n")
        f.flush()
        os.fsync(f.fileno())


def handle_signal(signum, frame):
    global RUNNING
    print(f"[{now_timestamp()}] Encerrando serviço...")
    RUNNING = False


def normalize_alphasense_payload(payload, topic):
    return {
        "timestamp_rpi": now_timestamp(),
        "timestamp_ms": payload.get("timestamp_ms", ""),
        "sensor_type": payload.get("sensor_type", ""),
        "board_id": payload.get("board_id", ""),
        "topic": topic,
        "we_raw_mv": payload.get("we_raw_mv", ""),
        "aux_raw_mv": payload.get("aux_raw_mv", ""),
        "corrected_signal_mv": payload.get("corrected_signal_mv", ""),
        "concentration_ppb": payload.get("concentration_ppb", ""),
        "rssi": payload.get("rssi", "")
    }


def normalize_pms_payload(payload, topic):
    return {
        "timestamp_rpi": now_timestamp(),
        "timestamp_ms": payload.get("timestamp_ms", ""),
        "sensor_type": payload.get("sensor_type", "pms7003"),
        "board_id": payload.get("board_id", ""),
        "topic": topic,
        "pm1_0": payload.get("pm1_0", ""),
        "pm2_5": payload.get("pm2_5", ""),
        "pm10_0": payload.get("pm10_0", ""),
        "rssi": payload.get("rssi", "")
    }


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[{now_timestamp()}] Conectado ao broker MQTT.")
        client.subscribe(TOPIC_NO2, qos=1)
        client.subscribe(TOPIC_OX, qos=1)
        client.subscribe(f"{TOPIC_PMS_BASE}/#", qos=1)
        client.subscribe(TOPIC_PMS_BASE, qos=1)
    else:
        print(f"[{now_timestamp()}] Falha na conexão MQTT. rc={rc}")


def on_disconnect(client, userdata, rc):
    print(f"[{now_timestamp()}] MQTT desconectado. rc={rc}")


def on_message(client, userdata, msg):
    topic = msg.topic

    try:
        payload = json.loads(msg.payload.decode("utf-8"))
    except Exception as e:
        print(f"[{now_timestamp()}] Erro ao processar payload em {topic}: {e}")
        return

    if topic == TOPIC_NO2:
        data = normalize_alphasense_payload(payload, topic)

        append_row(NO2_FILE, [
            data["timestamp_rpi"],
            data["timestamp_ms"],
            data["sensor_type"],
            data["board_id"],
            data["topic"],
            data["we_raw_mv"],
            data["aux_raw_mv"],
            data["corrected_signal_mv"],
            data["concentration_ppb"],
            data["rssi"]
        ])

        append_jsonl(data)

        print(
            f"[{data['timestamp_rpi']}] NO2 salvo | "
            f"board_id={data['board_id']} | "
            f"conc={data['concentration_ppb']} ppb"
        )
        return

    if topic == TOPIC_OX:
        data = normalize_alphasense_payload(payload, topic)

        append_row(OX_FILE, [
            data["timestamp_rpi"],
            data["timestamp_ms"],
            data["sensor_type"],
            data["board_id"],
            data["topic"],
            data["we_raw_mv"],
            data["aux_raw_mv"],
            data["corrected_signal_mv"],
            data["concentration_ppb"],
            data["rssi"]
        ])

        append_jsonl(data)

        print(
            f"[{data['timestamp_rpi']}] OX salvo | "
            f"board_id={data['board_id']} | "
            f"conc={data['concentration_ppb']} ppb"
        )
        return

    if topic == TOPIC_PMS_BASE or topic.startswith(f"{TOPIC_PMS_BASE}/"):
        data = normalize_pms_payload(payload, topic)

        append_row(PMS_FILE, [
            data["timestamp_rpi"],
            data["timestamp_ms"],
            data["sensor_type"],
            data["board_id"],
            data["topic"],
            data["pm1_0"],
            data["pm2_5"],
            data["pm10_0"],
            data["rssi"]
        ])

        append_jsonl(data)

        print(
            f"[{data['timestamp_rpi']}] PMS salvo | "
            f"board_id={data['board_id']} | "
            f"PM1.0={data['pm1_0']} | "
            f"PM2.5={data['pm2_5']} | "
            f"PM10={data['pm10_0']}"
        )
        return


def main():
    global RUNNING

    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    ensure_directories()
    ensure_csv_headers()

    client = mqtt.Client(client_id="rpi-mqtt-sensor-logger")
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message

    print(f"[{now_timestamp()}] Iniciando logger MQTT...")
    client.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE)
    client.loop_start()

    try:
        while RUNNING:
            time.sleep(1)
    finally:
        client.loop_stop()
        client.disconnect()
        print(f"[{now_timestamp()}] Logger finalizado.")


if __name__ == "__main__":
    main()
EOF

chmod +x "$SCRIPTS_DIR/mqtt_sensor_logger.py"
chown "$USERNAME:$USERNAME" "$SCRIPTS_DIR/mqtt_sensor_logger.py"

# -------------------------
# INSTALAÇÃO DO PLATFORMIO
# -------------------------
echo "[6/8] Instalando PlatformIO..."

curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules \
  -o /etc/udev/rules.d/99-platformio-udev.rules

udevadm control --reload-rules
udevadm trigger

sudo -u "$USERNAME" curl -fsSL -o "$USER_HOME/get-platformio.py" \
  https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py

sudo -u "$USERNAME" python3 "$USER_HOME/get-platformio.py"

if ! grep -q "alias pio=" "$USER_HOME/.bashrc"; then
  echo "" >> "$USER_HOME/.bashrc"
  echo "# PlatformIO - IC Daniel" >> "$USER_HOME/.bashrc"
  echo "alias pio='$USER_HOME/.platformio/penv/bin/platformio'" >> "$USER_HOME/.bashrc"
fi

ln -sf "$USER_HOME/.platformio/penv/bin/platformio" /usr/local/bin/platformio
ln -sf "$USER_HOME/.platformio/penv/bin/pio" /usr/local/bin/pio || true

usermod -aG dialout "$USERNAME"

# -------------------------
# PROJETO PLATFORMIO - ALPHASENSE
# -------------------------
echo "[7/8] Criando projeto PlatformIO Alphasense..."

cat > "$ALPHASENSE_DIR/platformio.ini" <<'EOF'
[platformio]
default_envs = no2

[env]
platform = espressif8266
framework = arduino

upload_speed = 115200

monitor_speed = 115200
monitor_dtr = 0
monitor_rts = 0
monitor_filters = esp8266_exception_decoder

board_build.flash_mode = dout
board_build.f_cpu = 80000000L

lib_ldf_mode = deep+

build_flags =
        -D MQTT_MAX_PACKET_SIZE=512

lib_deps =
        knolleary/PubSubClient @ ^2.8
        robtillaart/ADS1X15 @ ^0.6.1

[env:no2]
board = nodemcuv2
build_flags =
        ${env.build_flags}
        -D SENSOR_TYPE=\"no2\"

[env:ox]
board = nodemcuv2
build_flags =
        ${env.build_flags}
        -D SENSOR_TYPE=\"ox\"
EOF

cat > "$ALPHASENSE_DIR/src/main.cpp" <<EOF
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ADS1X15.h>
#include <string.h>
#include <math.h>

// ======================================================
// CONFIGURACAO DO SENSOR NA COMPILACAO
// Definido no platformio.ini:
// -D SENSOR_TYPE="no2" ou -D SENSOR_TYPE="ox"
// ======================================================
#ifndef SENSOR_TYPE
  #define SENSOR_TYPE "unknown"
#endif

// ======================================================
// CONFIGURACOES Wi-Fi
// ======================================================
const char* WIFI_SSID = "$WIFI_SSID";
const char* WIFI_PASS = "$WIFI_PASS";

// ======================================================
// CONFIGURACOES MQTT
// ======================================================
const char* MQTT_HOST = "$MQTT_HOST";
const uint16_t MQTT_PORT = $MQTT_PORT;

// ======================================================
// ADS1115
// ======================================================
ADS1115 ads(0x48);

// AIN1 = WE
// AIN3 = AUX
const uint8_t CHANNEL_WE  = 1;
const uint8_t CHANNEL_AUX = 3;

const uint8_t ADS_GAIN = 0;

// ======================================================
// INTERVALOS
// ======================================================
const unsigned long PUBLISH_INTERVAL_MS = 1000;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

// ======================================================
// CONSTANTES DE CALIBRACAO ALPHASENSE
// Usadas apenas no calculo interno, nao enviadas por MQTT
// Substitua pelos valores reais do datasheet individual
// ======================================================

// -------- NO2-B43F --------
const float NO2_WE_ZERO_MV = 0.0;
const float NO2_AUX_ZERO_MV = 0.0;
const float NO2_SENS_MV_PER_PPB = 0.250;
const float NO2_NT_FACTOR = 1.0;

// -------- OX-B431 --------
// OX mede oxidantes: NO2 + O3
const float OX_WE_ZERO_MV = 228.0;
const float OX_AUX_ZERO_MV = 243.0;
const float OX_SENS_MV_PER_PPB = 0.387;
const float OX_NT_FACTOR = 1.0;

// ======================================================
// OBJETOS GLOBAIS
// ======================================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastPublish = 0;
unsigned long lastMqttReconnectAttempt = 0;

String boardId;
String mqttClientId;
String mqttTopic;

// ======================================================
// ESTRUTURA DE CALIBRACAO
// ======================================================
struct AlphaCalibration {
  float weZeroMv;
  float auxZeroMv;
  float sensitivityMvPerPpb;
  float nT;
  bool valid;
};

AlphaCalibration getCalibration() {
  if (strcmp(SENSOR_TYPE, "no2") == 0) {
    return {
      NO2_WE_ZERO_MV,
      NO2_AUX_ZERO_MV,
      NO2_SENS_MV_PER_PPB,
      NO2_NT_FACTOR,
      true
    };
  }

  if (strcmp(SENSOR_TYPE, "ox") == 0) {
    return {
      OX_WE_ZERO_MV,
      OX_AUX_ZERO_MV,
      OX_SENS_MV_PER_PPB,
      OX_NT_FACTOR,
      true
    };
  }

  return {0.0, 0.0, 1.0, 1.0, false};
}

// ======================================================
// FUNCOES AUXILIARES
// ======================================================
String buildBoardId() {
  uint32_t chipId = ESP.getChipId();

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "ESP-%06X", chipId);

  return String(buffer);
}

void setupWiFiRadio() {
  WiFi.persistent(false);

  WiFi.mode(WIFI_OFF);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // Reduz pico de corrente do radio Wi-Fi
  WiFi.setOutputPower(8.5);

  // Modo mais estavel para ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);

  WiFi.setAutoReconnect(true);

  delay(1000);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long t0 = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) {
    delay(1000);
    yield();

    Serial.print("status=");
    Serial.println(WiFi.status());
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi conectado.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("Falha ao conectar no Wi-Fi.");
    Serial.print("Status final: ");
    Serial.println(WiFi.status());
  }
}

bool reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (mqttClient.connected()) return true;

  Serial.print("Conectando ao MQTT... ");

  bool ok = mqttClient.connect(mqttClientId.c_str());

  if (ok) {
    Serial.println("conectado.");
  } else {
    Serial.print("falhou, rc=");
    Serial.println(mqttClient.state());
  }

  return ok;
}

void setupADS() {
  Wire.begin();

  if (!ads.begin()) {
    Serial.println("Erro: ADS1115 nao encontrado.");

    while (true) {
      delay(1000);
      yield();
    }
  }

  ads.setGain(ADS_GAIN);

  Serial.println("ADS1115 inicializado.");
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);
}

// ======================================================
// PUBLICACAO DOS DADOS
// Payload enxuto, sem envio das constantes de calibracao
// ======================================================
void publishSensorData() {
  ads.setGain(ADS_GAIN);

  int16_t rawWE  = ads.readADC(CHANNEL_WE);
  int16_t rawAUX = ads.readADC(CHANNEL_AUX);

  float factor = ads.toVoltage(1);

  float weV = rawWE * factor;
  float auxV = rawAUX * factor;

  float weMv = weV * 1000.0;
  float auxMv = auxV * 1000.0;

  AlphaCalibration cal = getCalibration();

  float weCorrectedMv = weMv - cal.weZeroMv;
  float auxCorrectedMv = auxMv - cal.auxZeroMv;
  float correctedSignalMv = weCorrectedMv - cal.nT * auxCorrectedMv;

  float concentrationPpb = NAN;

  if (cal.valid && cal.sensitivityMvPerPpb != 0.0) {
    concentrationPpb = correctedSignalMv / cal.sensitivityMvPerPpb;
  }

  String payload = "{";
  payload += "\\"timestamp_ms\\":" + String(millis()) + ",";
  payload += "\\"sensor_type\\":\\"" + String(SENSOR_TYPE) + "\\",";
  payload += "\\"board_id\\":\\"" + boardId + "\\",";
  payload += "\\"we_raw_mv\\":" + String(weMv, 3) + ",";
  payload += "\\"aux_raw_mv\\":" + String(auxMv, 3) + ",";
  payload += "\\"corrected_signal_mv\\":" + String(correctedSignalMv, 3) + ",";
  payload += "\\"concentration_ppb\\":" + String(concentrationPpb, 3) + ",";
  payload += "\\"rssi\\":" + String(WiFi.RSSI());
  payload += "}";

  bool ok = mqttClient.publish(mqttTopic.c_str(), payload.c_str(), false);

  Serial.print("WE=");
  Serial.print(weMv, 3);
  Serial.print(" mV | AUX=");
  Serial.print(auxMv, 3);
  Serial.print(" mV | Signal=");
  Serial.print(correctedSignalMv, 3);
  Serial.print(" mV | ");
  Serial.print(SENSOR_TYPE);
  Serial.print("=");
  Serial.print(concentrationPpb, 3);
  Serial.print(" ppb | MQTT=");
  Serial.println(ok ? "OK" : "FALHOU");
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(5000);

  Serial.println();
  Serial.println("Inicializando sistema Alphasense + MQTT");

  boardId = buildBoardId();
  mqttClientId = String("alphasense-") + SENSOR_TYPE + "-" + boardId;
  mqttTopic = String("sensores/alphasense/") + SENSOR_TYPE;

  Serial.print("SENSOR_TYPE: ");
  Serial.println(SENSOR_TYPE);
  Serial.print("BOARD_ID: ");
  Serial.println(boardId);
  Serial.print("MQTT_CLIENT_ID: ");
  Serial.println(mqttClientId);
  Serial.print("MQTT_TOPIC: ");
  Serial.println(mqttTopic);

  setupADS();

  setupWiFiRadio();

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectWiFi();
  reconnectMQTT();
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  yield();

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    unsigned long now = millis();

    if (now - lastMqttReconnectAttempt >= MQTT_RETRY_INTERVAL_MS) {
      lastMqttReconnectAttempt = now;
      reconnectMQTT();
    }
  } else {
    mqttClient.loop();
  }

  unsigned long now = millis();

  if (now - lastPublish >= PUBLISH_INTERVAL_MS) {
    lastPublish = now;

    if (mqttClient.connected()) {
      publishSensorData();
    }
  }
}
EOF

# -------------------------
# PROJETO PLATFORMIO - PMS7003
# -------------------------
echo "[8/8] Criando projeto PlatformIO PMS7003..."

cat > "$PMS_DIR/platformio.ini" <<'EOF'
[env:pms7003]
platform = espressif8266
board = d1_mini
framework = arduino

upload_speed = 115200

monitor_speed = 115200
monitor_dtr = 0
monitor_rts = 0
monitor_filters = esp8266_exception_decoder

board_build.flash_mode = dout
board_build.f_cpu = 80000000L

lib_ldf_mode = deep+

lib_deps =
    knolleary/PubSubClient @ ^2.8

build_flags =
    -D MQTT_MAX_PACKET_SIZE=256
EOF

cat > "$PMS_DIR/src/main.cpp" <<EOF
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// ======================================================
// CONFIGURACOES Wi-Fi
// ======================================================
const char* WIFI_SSID = "$WIFI_SSID";
const char* WIFI_PASS = "$WIFI_PASS";

// ======================================================
// CONFIGURACOES MQTT
// ======================================================
const char* MQTT_HOST = "$MQTT_HOST";
const uint16_t MQTT_PORT = $MQTT_PORT;

const char* MQTT_TOPIC_BASE = "sensores/pms7003";

// ======================================================
// DEBUG USB
// Coloque 0 para desligar os prints USB no uso final
// ======================================================
#define DEBUG_USB 1

#if DEBUG_USB
  #define DBG(x) Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif

// ======================================================
// PMS7003
// PMS TXD -> Wemos D7/GPIO13
// O TX do SoftwareSerial fica em D6, mas nao precisa conectar
// ======================================================
SoftwareSerial pmsSerial(D7, D6);

uint8_t buffer[32];

// ======================================================
// MQTT / Wi-Fi
// ======================================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

char boardId[20];
char mqttClientId[40];
char mqttTopic[80];

unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 1000;

// ======================================================
// FUNCOES AUXILIARES
// ======================================================
uint16_t read16(uint8_t high, uint8_t low) {
  return ((uint16_t)high << 8) | low;
}

void buildIds() {
  uint32_t chipId = ESP.getChipId();

  snprintf(boardId, sizeof(boardId), "ESP-%06X", chipId);
  snprintf(mqttClientId, sizeof(mqttClientId), "pms7003-%s", boardId);
  snprintf(mqttTopic, sizeof(mqttTopic), "%s/%s", MQTT_TOPIC_BASE, boardId);
}

void setupWiFiRadio() {
  WiFi.persistent(false);

  WiFi.mode(WIFI_OFF);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // Reduz pico de corrente do radio Wi-Fi
  WiFi.setOutputPower(8.5);

  // Modo mais estavel para ESP8266
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);

  WiFi.setAutoReconnect(true);

  delay(1000);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  DBG("Conectando ao Wi-Fi: ");
  DBGLN(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    delay(1000);
    yield();

    DBG("status=");
    DBGLN(WiFi.status());
  }

  if (WiFi.status() == WL_CONNECTED) {
    DBGLN("Wi-Fi conectado.");
    DBG("IP: ");
    DBGLN(WiFi.localIP().toString());
    DBG("RSSI: ");
    DBGLN(String(WiFi.RSSI()));
  } else {
    DBGLN("Falha ao conectar no Wi-Fi.");
    DBG("Status final: ");
    DBGLN(WiFi.status());
  }
}

bool connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (mqttClient.connected()) return true;

  DBG("Conectando ao MQTT... ");

  bool ok = mqttClient.connect(mqttClientId);

  if (ok) {
    DBGLN("conectado.");
  } else {
    DBG("falhou. Estado: ");
    DBGLN(mqttClient.state());
  }

  return ok;
}

bool readPMSFrame() {
  while (pmsSerial.available()) {
    if (pmsSerial.read() == 0x42) {
      unsigned long startHeader = millis();

      while (!pmsSerial.available() && millis() - startHeader < 100) {
        yield();
      }

      if (!pmsSerial.available()) return false;

      if (pmsSerial.read() == 0x4D) {
        buffer[0] = 0x42;
        buffer[1] = 0x4D;

        int index = 2;
        unsigned long start = millis();

        while (index < 32 && millis() - start < 1000) {
          if (pmsSerial.available()) {
            buffer[index++] = pmsSerial.read();
          }
          yield();
        }

        if (index < 32) {
          DBGLN("Frame incompleto.");
          return false;
        }

        uint16_t checksum = 0;

        for (int i = 0; i < 30; i++) {
          checksum += buffer[i];
        }

        uint16_t receivedChecksum = read16(buffer[30], buffer[31]);

        if (checksum != receivedChecksum) {
          DBGLN("Checksum invalido.");
          return false;
        }

        return true;
      }
    }
  }

  return false;
}

// ======================================================
// PUBLICACAO DOS DADOS
// Payload enxuto, sem CF1/ATM duplicado
// ======================================================
void publishPMSData() {
  // Usando os campos atmosfericos do PMS7003
  uint16_t pm1_atm  = read16(buffer[10], buffer[11]);
  uint16_t pm25_atm = read16(buffer[12], buffer[13]);
  uint16_t pm10_atm = read16(buffer[14], buffer[15]);

  char payload[256];

  snprintf(
    payload,
    sizeof(payload),
    "{"
      "\\"timestamp_ms\\":%lu,"
      "\\"sensor_type\\":\\"pms7003\\","
      "\\"board_id\\":\\"%s\\","
      "\\"pm1_0\\":%u,"
      "\\"pm2_5\\":%u,"
      "\\"pm10_0\\":%u,"
      "\\"rssi\\":%d"
    "}",
    millis(),
    boardId,
    pm1_atm,
    pm25_atm,
    pm10_atm,
    WiFi.RSSI()
  );

  bool ok = mqttClient.publish(mqttTopic, payload, false);

  DBG("PM1.0: ");
  DBG(pm1_atm);
  DBG("  PM2.5: ");
  DBG(pm25_atm);
  DBG("  PM10: ");
  DBG(pm10_atm);
  DBG("  MQTT: ");
  DBGLN(ok ? "OK" : "FALHOU");
}

// ======================================================
// SETUP
// ======================================================
void setup() {
#if DEBUG_USB
  Serial.begin(115200);
  delay(5000);

  Serial.println();
  Serial.println("BOOT OK - PMS7003 Wi-Fi MQTT");
#endif

  buildIds();

  DBG("Board ID: ");
  DBGLN(boardId);

  DBG("Topico MQTT: ");
  DBGLN(mqttTopic);

  setupWiFiRadio();

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectWiFi();
  connectMQTT();

  // Inicia o PMS depois do Wi-Fi/MQTT
  pmsSerial.begin(9600);
  DBGLN("PMS7003 iniciado.");
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  yield();

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    unsigned long now = millis();

    if (now - lastMqttAttempt >= MQTT_RETRY_INTERVAL) {
      lastMqttAttempt = now;
      connectMQTT();
    }
  } else {
    mqttClient.loop();
  }

  if (readPMSFrame()) {
    unsigned long now = millis();

    if (mqttClient.connected() && now - lastPublish >= PUBLISH_INTERVAL) {
      lastPublish = now;
      publishPMSData();
    }
  }
}
EOF

chown -R "$USERNAME:$USERNAME" "$BASE_DIR"

echo "===================================================="
echo "MÓDULO 2 FINALIZADO!"
echo ""
echo "Pastas criadas:"
echo "  $BASE_DIR"
echo "  $SCRIPTS_DIR"
echo "  $ALPHASENSE_DIR"
echo "  $PMS_DIR"
echo "  $DATA_DIR/json"
echo ""
echo "Configurações usadas:"
echo "  WIFI_SSID=$WIFI_SSID"
echo "  MQTT_HOST=$MQTT_HOST"
echo "  MQTT_PORT=$MQTT_PORT"
echo ""
echo "PlatformIO:"
echo "  use: pio"
echo "  talvez seja necessário rodar: source ~/.bashrc"
echo "  ou reiniciar o terminal"
echo ""
echo "Teste MQTT geral:"
echo "  mosquitto_sub -h localhost -t 'sensores/#' -v"
echo ""
echo "Teste MQTT PMS:"
echo "  mosquitto_sub -h 192.168.4.1 -t 'sensores/pms7003/#' -v"
echo ""
echo "Compilar Alphasense NO2:"
echo "  cd $ALPHASENSE_DIR && pio run -e no2"
echo ""
echo "Compilar Alphasense OX:"
echo "  cd $ALPHASENSE_DIR && pio run -e ox"
echo ""
echo "Compilar PMS7003:"
echo "  cd $PMS_DIR && pio run -e pms7003"
echo ""
echo "Upload Alphasense NO2:"
echo "  cd $ALPHASENSE_DIR && pio run -e no2 -t upload"
echo ""
echo "Upload Alphasense OX:"
echo "  cd $ALPHASENSE_DIR && pio run -e ox -t upload"
echo ""
echo "Upload PMS7003:"
echo "  cd $PMS_DIR && pio run -t upload"
echo ""
echo "Monitor:"
echo "  pio device list"
echo "  pio device monitor -p /dev/ttyUSB0 -b 115200"
echo ""
echo "Logger Python:"
echo "  $SCRIPTS_DIR/mqtt_sensor_logger.py"
echo ""
echo "Arquivos CSV:"
echo "  $DATA_DIR/no2/NO2_B43F.csv"
echo "  $DATA_DIR/ox/OX_B431.csv"
echo "  $DATA_DIR/pms/PMS7003.csv"
echo ""
echo "Backup JSON Lines:"
echo "  $DATA_DIR/json/sensores.jsonl"
echo "===================================================="
```

## 2. Editar a rede Wi-Fi no Módulo 2

Antes de executar, altere este trecho:

```bash
WIFI_SSID="IC_SENSORES"
WIFI_PASS="sensores123"
```

Esses valores precisam ser **exatamente iguais** ao `SSID` e `PASSWORD` definidos no Módulo 1.

Exemplo: se no Módulo 1 você usou:

```bash
SSID="lactea0_daniel"
PASSWORD="iotempire0"
```

então no Módulo 2 use:

```bash
WIFI_SSID="lactea0_daniel"
WIFI_PASS="iotempire0"
```

Em geral, mantenha:

```bash
MQTT_HOST="192.168.4.1"
MQTT_PORT="1883"
```

porque `192.168.4.1` é o IP configurado para o Raspberry no hotspot.

## 3. Executar o Módulo 2

Execute:

```bash
sudo bash setup_pio_mqtt.sh
```

Não execute com `sh setup_pio_mqtt.sh`.

## 4. Atualizar o terminal após instalar PlatformIO

Depois da instalação, rode:

```bash
source ~/.bashrc
```

ou feche e abra o terminal novamente.

Teste se o PlatformIO está disponível:

```bash
pio --version
```

## 5. Verificar o Mosquitto

Confira se o broker MQTT está rodando:

```bash
systemctl status mosquitto --no-pager
```

Confira se a porta 1883 está aberta:

```bash
sudo ss -ltnp | grep 1883
```

Para escutar todos os tópicos dos sensores:

```bash
mosquitto_sub -h localhost -t 'sensores/#' -v
```

## 6. Estrutura criada pelo script

O script cria a estrutura:

```bash
~/IC_Daniel/
├── alphasense/
│   ├── platformio.ini
│   └── src/main.cpp
├── pms/
│   ├── platformio.ini
│   └── src/main.cpp
├── scripts/
│   └── mqtt_sensor_logger.py
└── data/
    ├── no2/
    ├── ox/
    ├── pms/
    └── json/
```

Os dados serão salvos principalmente em:

```bash
~/IC_Daniel/data/no2/NO2_B43F.csv
~/IC_Daniel/data/ox/OX_B431.csv
~/IC_Daniel/data/pms/PMS7003.csv
~/IC_Daniel/data/json/sensores.jsonl
```

## 7. Compilar e enviar os códigos para os sensores

### Alphasense NO2

```bash
cd ~/IC_Daniel/alphasense
pio run -e no2
pio run -e no2 -t upload
```

### Alphasense OX

```bash
cd ~/IC_Daniel/alphasense
pio run -e ox
pio run -e ox -t upload
```

### PMS7003

```bash
cd ~/IC_Daniel/pms
pio run -e pms7003
pio run -e pms7003 -t upload
```

## 8. Monitor serial

Liste as portas:

```bash
pio device list
```

Abra o monitor:

```bash
pio device monitor -p /dev/ttyUSB0 -b 115200
```

Se a porta for outra, substitua `/dev/ttyUSB0`.

## 9. Rodar o logger Python

Para salvar os dados recebidos por MQTT em CSV e JSONL:

```bash
~/IC_Daniel/scripts/mqtt_sensor_logger.py
```

Enquanto esse script estiver rodando, ele deve escutar os tópicos:

```bash
sensores/alphasense/no2
sensores/alphasense/ox
sensores/pms7003
sensores/pms7003/#
```

---

# Módulo 3 — Node-RED, InfluxDB e Grafana

Este módulo instala Node-RED, InfluxDB 2 e Grafana. Ele **não instala MQTT**, porque o Mosquitto já foi instalado no Módulo 2.

> **Importante:** o Módulo 3 apenas instala e inicia os serviços.  
> A configuração do **Node-RED deve ser feita manualmente** depois, pelo navegador. Isso inclui importar/criar fluxos, configurar os nós MQTT, configurar conexão com InfluxDB e montar o fluxo de envio dos dados para o banco.

## 1. Criar o arquivo do script

Crie o arquivo:

```bash
micro setup_ing.sh
```

Cole o código abaixo:

```bash
#!/usr/bin/env bash
set -e

# ============================================================
# MÓDULO 3 - NODE-RED, INFLUXDB E GRAFANA
# IC Daniel - Dashboard e armazenamento de dados
# ============================================================

USERNAME="${SUDO_USER:-$USER}"
USER_HOME="$(eval echo ~$USERNAME)"

echo "===================================================="
echo "Instalando Node-RED, InfluxDB 2 e Grafana..."
echo "MQTT não será instalado neste módulo."
echo "===================================================="

if [ "$EUID" -ne 0 ]; then
  echo "Execute com sudo:"
  echo "sudo bash setup_ic.sh"
  exit 1
fi

export DEBIAN_FRONTEND=noninteractive

# -------------------------
# DEPENDÊNCIAS BÁSICAS
# -------------------------
echo "[1/7] Atualizando sistema e instalando dependências..."

apt update
apt upgrade -y

apt install -y \
  curl \
  gnupg \
  ca-certificates \
  apt-transport-https

# -------------------------
# INSTALAÇÃO DO NODE-RED
# -------------------------
echo "[2/7] Instalando Node.js e Node-RED..."

# O script oficial instala/atualiza Node.js e Node-RED.
# O parâmetro --confirm-root evita interação desnecessária quando rodado com sudo.
curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered -o install_nodered.sh
bash install_nodered.sh --confirm-root
rm install_nodered.sh

# -------------------------
# CONFIGURAÇÃO BÁSICA DO NODE-RED
# -------------------------
echo "[3/7] Configurando Node-RED como serviço..."

systemctl enable nodered.service
systemctl restart nodered.service

# Limite de memória para Raspberry Pi
mkdir -p /etc/systemd/system/nodered.service.d

cat > /etc/systemd/system/nodered.service.d/override.conf <<EOF
[Service]
Environment="NODE_OPTIONS=--max_old_space_size=256"
EOF

systemctl daemon-reload
systemctl restart nodered.service

# -------------------------
# INSTALAÇÃO DO INFLUXDB 2
# -------------------------
echo "[4/7] Adicionando repositório do InfluxDB..."

curl -fsSL https://repos.influxdata.com/influxdata-archive.key \
  | gpg --dearmor \
  | tee /usr/share/keyrings/influxdb-archive-keyring.gpg >/dev/null

echo "deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main" \
  > /etc/apt/sources.list.d/influxdb.list

apt update

echo "[5/7] Instalando e iniciando InfluxDB 2..."

apt install -y influxdb2

systemctl unmask influxdb || true
systemctl enable influxdb
systemctl restart influxdb

# -------------------------
# INSTALAÇÃO DO GRAFANA
# -------------------------
echo "[6/7] Adicionando repositório do Grafana..."

curl -fsSL https://apt.grafana.com/gpg.key \
  | gpg --dearmor \
  | tee /usr/share/keyrings/grafana-archive-keyrings.gpg >/dev/null

echo "deb [signed-by=/usr/share/keyrings/grafana-archive-keyrings.gpg] https://apt.grafana.com stable main" \
  > /etc/apt/sources.list.d/grafana.list

apt update

echo "[7/7] Instalando e iniciando Grafana..."

apt install -y grafana

systemctl enable grafana-server
systemctl restart grafana-server

echo "===================================================="
echo "MÓDULO 3 FINALIZADO!"
echo ""
echo "Serviços instalados:"
echo "  Node-RED:  porta 1880"
echo "  InfluxDB:  porta 8086"
echo "  Grafana:   porta 3000"
echo ""
echo "Acesse manualmente pelo navegador:"
echo "  Node-RED:  http://192.168.4.1:1880"
echo "  InfluxDB:  http://192.168.4.1:8086"
echo "  Grafana:   http://192.168.4.1:3000"
echo ""
echo "Credenciais padrão do Grafana:"
echo "  usuário: admin"
echo "  senha: admin"
echo ""
echo "Checar status:"
echo "  systemctl status nodered.service"
echo "  systemctl status influxdb"
echo "  systemctl status grafana-server"
echo "===================================================="

```

## 2. Executar o Módulo 3

Execute:

```bash
sudo bash setup_ing.sh
```

Não execute com `sh setup_ing.sh`.

## 3. Acessar os serviços no navegador

Depois da instalação, acesse:

```text
Node-RED:  http://192.168.4.1:1880
InfluxDB:  http://192.168.4.1:8086
Grafana:   http://192.168.4.1:3000
```

Se você estiver acessando a partir do próprio Raspberry, também pode usar:

```text
Node-RED:  http://localhost:1880
InfluxDB:  http://localhost:8086
Grafana:   http://localhost:3000
```

Credenciais padrão iniciais do Grafana:

```text
usuário: admin
senha: admin
```

## 4. Configuração manual necessária no Node-RED

Depois de abrir o Node-RED, será necessário configurar manualmente o fluxo. Em geral, o fluxo deve ter:

1. nós `mqtt in` escutando os tópicos dos sensores;
2. nós `json` para converter o payload recebido;
3. nós de tratamento/função, se necessário;
4. nós de saída para o InfluxDB;
5. dashboard ou debug, se quiser acompanhar os dados em tempo real.

Tópicos MQTT esperados:

```text
sensores/alphasense/no2
sensores/alphasense/ox
sensores/pms7003
sensores/pms7003/#
```

Broker MQTT no Node-RED:

```text
localhost
porta 1883
```

ou:

```text
192.168.4.1
porta 1883
```

## 5. Verificar status dos serviços

```bash
systemctl status nodered.service --no-pager
systemctl status influxdb --no-pager
systemctl status grafana-server --no-pager
```

---

# Ordem final de execução

A ordem recomendada é:

```bash
sudo bash setup_hotspot.sh
sudo reboot
sudo bash setup_pio_mqtt.sh
source ~/.bashrc
sudo bash setup_ing.sh
```

Depois disso:

1. envie os códigos para os ESP8266 com PlatformIO;
2. conecte os sensores ao hotspot;
3. teste os tópicos MQTT;
4. rode o logger Python;
5. configure manualmente Node-RED, InfluxDB e Grafana.

---

# Comandos rápidos de diagnóstico

## Ver rede Wi-Fi do Raspberry

```bash
iw dev
ip addr show wlan0
iw dev wlan0 get power_save
rfkill list
```

## Ver serviços do hotspot

```bash
systemctl status hostapd --no-pager
systemctl status dnsmasq --no-pager
```

## Ver MQTT

```bash
systemctl status mosquitto --no-pager
sudo ss -ltnp | grep 1883
mosquitto_sub -h localhost -t 'sensores/#' -v
```

## Ver Node-RED, InfluxDB e Grafana

```bash
systemctl status nodered.service --no-pager
systemctl status influxdb --no-pager
systemctl status grafana-server --no-pager
```

---

# Observações importantes

- Sempre execute os scripts com `sudo bash nome_do_script.sh`.
- Não use `sh nome_do_script.sh`.
- O nome e a senha da rede do Módulo 2 precisam bater com o nome e a senha criados no Módulo 1.
- O IP `192.168.4.1` deve ser mantido se o hotspot continuar usando essa faixa.
- O Node-RED precisa ser configurado manualmente após o Módulo 3.
- O Módulo 3 não instala Mosquitto; isso já é feito no Módulo 2.
- Os valores de calibração dos sensores Alphasense devem ser conferidos e substituídos no `main.cpp` conforme o datasheet individual de cada sensor.

# 🛰️ Configuração de Hotspot no Raspberry Pi 3B+ (Versão Final com Persistência Total)

Este guia detalha o processo de transformação de um Raspberry Pi 3B+ em um Access Point (Hotspot). Esta versão inclui a criação de um serviço de sistema para garantir que o rádio Wi-Fi nunca seja bloqueado no boot.

---

## 🛠️ 0. Instalação e Limpeza Inicial

No **RPi OS Lite**, precisamos instalar os pacotes essenciais e remover o que pode causar conflito.

```bash
# Atualiza repositórios e instala dependências
sudo apt update && sudo apt upgrade -y
sudo apt install dhcpcd5 hostapd dnsmasq iptables-persistent -y
```
```bash
# Desabilita o wpa_supplicant para evitar que ele tente controlar a wlan0
sudo systemctl stop wpa_supplicant
sudo systemctl disable wpa_supplicant
```

---

## 🔐 1. Garantindo o Desbloqueio do Wi-Fi (rfkill)

Para evitar que o Wi-Fi seja bloqueado por software no boot (o que derruba o `hostapd`), criaremos um serviço dedicado:

1. **Crie o arquivo do serviço:**
   ```bash
   sudo nano /etc/systemd/system/wifi-unblock.service
   ```

2. **Cole o conteúdo abaixo:**
   ```ini
   [Unit]
   Description=Garantir desbloqueio do Wi-Fi (rfkill)
   After=network.target

   [Service]
   Type=oneshot
   ExecStart=/usr/sbin/rfkill unblock all
   RemainAfterExit=yes

   [Install]
   WantedBy=multi-user.target
   ```

3. **Habilite o serviço:**
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable wifi-unblock
   ```

---

## 🏃 2. Configuração de Rede

### Configurar IP Fixo (`wlan0`)
```bash
sudo nano /etc/dhcpcd.conf
```
Adicione ao final:
```conf
interface wlan0
    static ip_address=192.168.4.1/24
    nohook wpa_supplicant
```
Reinicie o DHCP:
```bash
sudo service dhcpcd restart
```
### Configurar Servidor DHCP (`dnsmasq`)
```bash
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.original
sudo nano /etc/dnsmasq.conf
```
Conteúdo:
```conf
interface=wlan0
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,24h
```

### Configurar o Access Point (`hostapd`)
```bash
sudo nano /etc/hostapd/hostapd.conf
```
Conteúdo:
```conf
interface=wlan0
driver=nl80211
ssid=<nome_da_rede>
hw_mode=g
channel=7
wmm_enabled=0
auth_algs=1
wpa=2
wpa_passphrase=<senha_da_rede>
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
```

Aponte a config no arquivo padrão:
```bash
sudo nano /etc/default/hostapd
```
-> Altere a linha #DAEMON_CONF="" para: `DAEMON_CONF="/etc/hostapd/hostapd.conf"`

---

## 🌐 3. Roteamento e NAT Profissional

Para que a internet passe da `eth0` (cabo) para a `wlan0` (Wi-Fi) de forma permanente:

### Ativar Forwarding
```bash
sudo nano /etc/sysctl.d/99-ipforward.conf
```
Adicione: `net.ipv4.ip_forward=1`. Na sequência, aplique com:
```bash
sudo sysctl --system
```
### Configurar NAT Permanente
```bash
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

### Restaurar no Boot via `rc.local`
```bash
sudo nano /etc/rc.local
```
Adicione o seguinte:
```bash
#!/bin/sh -e

iptables-restore < /etc/iptables.ipv4.nat

exit 0
```
Dê permissão:
```bash
sudo chmod +x /etc/rc.local
```
---
## 🛡️ 4. Reforço de Persistência (IP Forwarding Service)

Como segurança adicional, criaremos um serviço que força o encaminhamento de IP caso o arquivo de configuração seja ignorado.

1. **Crie o arquivo:** `sudo nano /etc/systemd/system/ipforward.service`
2. **Cole o conteúdo:**
```ini
[Unit]
Description=Enable IP forwarding for hotspot
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/sysctl -w net.ipv4.ip_forward=1
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
```
3. **Ative:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable ipforward.service
```

---
## ✅ 5. Inicialização Final

Agora, desmascare e ative os serviços principais:

```bash
sudo systemctl unmask hostapd
sudo systemctl enable hostapd dnsmasq
sudo systemctl restart hostapd dnsmasq
```

---

## 🔍 Comandos de Diagnóstico
* Verifique o rádio: `rfkill list`
* Verifique o encaminhamento: `sysctl net.ipv4.ip_forward`
* Status dos serviços: `sudo systemctl status hostapd dnsmasq`

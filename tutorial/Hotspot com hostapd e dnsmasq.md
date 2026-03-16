# 🛰️ Configuração de Hotspot no Raspberry Pi 3B+ (Versão Robusta)

Este guia detalha o processo de transformação de um Raspberry Pi 3B+ (rodando **Raspberry Pi OS Lite**) em um Access Point (Hotspot). Esta versão inclui correções para bloqueios de rádio (rfkill) e conflitos com o `wpa_supplicant`.

---

## 🛠️ 0. Preparação e Desbloqueio de Hardware

No RPi OS Lite, o Wi-Fi pode vir "bloqueado por software" por padrão. Precisamos garantir que o rádio esteja livre antes de instalar os serviços.

```bash
# Verifica se o Wi-Fi está bloqueado (Soft blocked: yes)
sudo rfkill list
```
```bash
# Desbloqueia o rádio
sudo rfkill unblock wifi
sudo rfkill unblock all
```
```bash
# Confirme se agora aparece "Soft blocked: no"
rfkill list
```

Agora, instale as dependências:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install dhcpcd5 hostapd dnsmasq iptables-persistent -y
```

---

## 🏃 Passo a Passo de Configuração

### 1️⃣ Parar e Limpar Conflitos
O `wpa_supplicant` tenta gerenciar o Wi-Fi para se conectar a redes externas. Como seremos um *Access Point*, ele deve ser desativado para não conflitar com o `hostapd`.

```bash
# Para os serviços de rede
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq

# Desabilita o wpa_supplicant para evitar conflitos na wlan0
sudo systemctl disable wpa_supplicant
```

### 2️⃣ Configurar IP Fixo para a interface Wi-Fi (`wlan0`)
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

### 3️⃣ Configurar o Servidor DHCP (`dnsmasq`)
```bash
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.original
sudo nano /etc/dnsmasq.conf
```
Conteúdo:
```conf
interface=wlan0
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,24h
```

### 4️⃣ Configurar o Access Point (`hostapd`)
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

### 5️⃣ Apontar o arquivo de configuração
```bash
sudo nano /etc/default/hostapd
```
Altere a linha para:
`DAEMON_CONF="/etc/hostapd/hostapd.conf"`

---

## 🌐 6. Roteamento e NAT (Método Robusto)

Em vez de apenas editar o `sysctl.conf`, criaremos um arquivo específico de prioridade para garantir o encaminhamento de IP no boot.

```bash
# Cria a configuração de encaminhamento permanente
sudo nano /etc/sysctl.d/99-ipforward.conf
```
Coloque apenas esta linha:
```conf
net.ipv4.ip_forward=1
```

Aplique as configurações do sistema:
```bash
sudo sysctl --system
```

**Configurar o NAT (Cabo -> Wi-Fi):**
```bash
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

---

## 🔄 7. Persistência e Inicialização

### Restaurar NAT via `rc.local`
Abra o arquivo:
```bash
sudo nano /etc/rc.local
```
Adicione antes do `exit 0`:
```bash
iptables-restore < /etc/iptables.ipv4.nat
```
Garanta a permissão:
```bash
sudo chmod +x /etc/rc.local
```

### Ativar serviços
```bash
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl enable dnsmasq
sudo systemctl start hostapd
sudo systemctl start dnsmasq
```

---

## ✅ Verificação Final
Para ter certeza de que o roteamento está ativo:
`sysctl net.ipv4.ip_forward`
> Deve retornar `1`.

Se o Wi-Fi não aparecer, verifique novamente o `rfkill list`.

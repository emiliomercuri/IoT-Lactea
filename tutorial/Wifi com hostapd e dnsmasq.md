# 🛰️ Configuração de Hotspot no Raspberry Pi 3B+

Este guia detalha o processo de transformação de um Raspberry Pi 3B+ (rodando **Raspberry Pi OS Lite**) em um Access Point (Hotspot), permitindo a conexão de dispositivos via Wi-Fi e o roteamento de internet vinda da interface ethernet (`eth0`).

---

## 🛠️ 0. Preparação e Instalação de Dependências

No **RPi OS Lite**, muitos pacotes de gerenciamento de rede não vêm instalados por padrão. Antes de iniciar a configuração, instale todas as ferramentas necessárias:

```bash
# Atualiza a lista de repositórios
sudo apt update

# Instala o gerenciador de DHCP, o daemon do ponto de acesso, o DNS recursivo e utilitários de firewall
sudo apt install dhcpcd5 hostapd dnsmasq iptables-persistent -y
```

> [!IMPORTANT]
> Se o serviço `dhcpcd` não estiver ativo após a instalação, certifique-se de habilitá-lo com: `sudo systemctl enable dhcpcd`.

---

## 🏃 Passo a Passo de Configuração

### 1️⃣ Parar serviços para configuração
Antes de editar os arquivos, interrompa os processos que serão configurados:

```bash
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq
```

### 2️⃣ Configurar IP Fixo para a interface Wi-Fi (`wlan0`)
Edite o arquivo de configuração do DHCP do sistema:

```bash
sudo nano /etc/dhcpcd.conf
```

Adicione o seguinte bloco ao final do arquivo:

```conf
interface wlan0
    static ip_address=192.168.4.1/24
    nohook wpa_supplicant
```

Reinicie o serviço para aplicar as mudanças:
```bash
sudo service dhcpcd restart
```

### 3️⃣ Configurar o Servidor DHCP (`dnsmasq`)
O `dnsmasq` gerenciará a atribuição de IPs. Vamos criar um arquivo novo:

```bash
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.original
sudo nano /etc/dnsmasq.conf
```

Insira as configurações abaixo:
```conf
# Interface de escuta
interface=wlan0

# Faixa de IPs (Início, Fim, Máscara, Tempo de concessão)
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,24h
```

### 4️⃣ Configurar o Access Point (`hostapd`)
Defina as credenciais e parâmetros do Wi-Fi:

```bash
sudo nano /etc/hostapd/hostapd.conf
```

Cole o conteúdo abaixo:
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

### 5️⃣ Apontar o binário para o arquivo de configuração
Informe ao sistema onde o arquivo do `hostapd` está localizado:

```bash
sudo nano /etc/default/hostapd
```

Encontre a linha `#DAEMON_CONF=""` e altere para:
```bash
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

### 6️⃣ Ativar Roteamento de IP (Forwarding)
Ative o encaminhamento de pacotes no kernel:

```bash
sudo nano /etc/sysctl.conf
```

Descomente a linha:
```conf
net.ipv4.ip_forward=1
```

Aplique a alteração:
```bash
sudo sysctl -p
```

### 7️⃣ Configurar NAT (IP Tables)
Crie a regra de mascaramento para rotear o tráfego da `eth0` para a `wlan0`:

```bash
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

### 8️⃣ Inicialização dos Serviços
Inicie e habilite os serviços:

```bash
# Unmask necessário em algumas versões do RPi OS
sudo systemctl unmask hostapd
sudo systemctl start hostapd
sudo systemctl start dnsmasq

# Habilitar no boot
sudo systemctl enable hostapd
sudo systemctl enable dnsmasq
```

---

## 🔄 Garantindo Persistência no Boot

Para garantir que o roteamento e as regras de firewall (NAT) sobrevivam a reinicializações no **OS Lite**:

### 1️⃣ Confirmar Persistência do `ip_forward`
**Teste de validação:**
```bash
sysctl net.ipv4.ip_forward
```
> O retorno esperado deve ser: `net.ipv4.ip_forward = 1`

### 2️⃣ Restaurar NAT Automaticamente no Boot via `rc.local`
Abra (ou crie) o arquivo:
```bash
sudo nano /etc/rc.local
```

Certifique-se de que o conteúdo seja exatamente este, inserindo o comando antes do `exit 0`:

```bash
#!/bin/sh -e

# Restaura as regras de NAT salvas anteriormente
iptables-restore < /etc/iptables.ipv4.nat

exit 0
```

Dê permissão de execução ao script:
```bash
sudo chmod +x /etc/rc.local
```

---

## ✅ Conclusão
O Raspberry Pi agora está configurado para iniciar automaticamente como um roteador Wi-Fi após qualquer reinicialização.

---

# Tutorial para aquição dos dados do sensor DHT11 no Raspberry Pi e criação de um dashboard no Grafana

Nesse tutorial será mostrado como fazer a aquisição dos dados de temperatura do ar e umidade relativa do ar usando o sensor DHT11 conectado no microcomputador Raspberry Pi 3B+.

---

**Autor:** Daniel Antonio Gomes de Souza

**Supervisor:** Emílio Graciliano Ferreira Mercuri

---

As etapas são:

## 1. Conexões Físicas:

Imagem e diagrama do Fritzing

## 2. Instalação do sistema operacional 

Num cartão de memória instalar o sistema operacional Raspberry Pi OS (64-bit) - Debian Linux.

Configurar:
- Usuário: lactea
- Senha: iot******r

## 3. Conexão remota

Raspberry Connect e Netbird 

## 4. MQTT, Influxdb, Node-Red e Grafana (MING Stack)

* Explicar brevemente o que é o MING Stack e seu princípio de funcionamento

Antes de iniciar a instalação dos softwares, precisamos atualizar todos os pacotes instalados no seu sistema para as versões mais recentes disponíveis nos repositórios. Para isso, execute no terminal:

```bash
sudo apt update && sudo apt upgrade
```

Digite "y" para confirmar a instalação, que poderá demorar alguns instantes.

### Instalação do MQTT:

Uma vez que seu sistema foi atualizado, podemos instalar o Mosquitto MQTT Broker, que é um servidor que gerencia mensagens usando o protocolo MQTT (Message Queuing Telemetry Transport), através do seguinte comando:

```bash
sudo apt install -y mosquitto mosquitto-clients
```

Uma vez instalado, precisamos configurar o Mosquitto para iniciar automaticamente toda vez que o sistema for ligado, rodando no Prompt:

```bash
sudo systemctl enable mosquitto.service
```

Agora, para checar o status do Mosquitto, rode:

```bash
mosquitto -v
```

O que deverá retornar a geguinte mensagem:

![5](https://github.com/user-attachments/assets/20fb2900-38a6-46a5-9284-cae5df662150)





# Build Real-Time IoT Dashboard: Node-RED + InfluxDB + Grafana + MQTT Tutorial (MING)

Este tutorial foi criado com base no video do YouTube:
https://www.youtube.com/watch?v=0mluq0oJk7o&t=373s&ab_channel=IoTFrontier

Configurar o Raspberry Pi 3 ou 4 com o sistema operacional Raspberry Pi OS (64bits).

## MQTT Install
Para instalar o Mosquitto, basta rodar os seguintes comandos no prompt de comando do Raspberry Pi:

```bash
sudo apt update && sudo apt upgrade
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
```

mosquitto -v

sudo nano /etc/mosquitto/mosquitto.conf
	Acrescentar ao final:
		listener 1883
		allow_anonymous true

sudo systemctl restart mosquitto

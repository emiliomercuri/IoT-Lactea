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

Para verificar o status do Mosquitto:
```
mosquitto -v
```
Onde deve ser exibido o seguinte erro:

> Opening ipv4 listen socket on port 1883.
> 
> Error: Address already in use
> 
> Opening ipv6 listen socket on port 1883.
> 
> Error: Address already in use

Ou
```
sudo systemctl status mosquitto
```
One deve ser exibido:
> Loaded: *loaded (... ; *enabled; preset: *enabled)
> 
> Active: *active (running)

Para configurar a porta de comunicação do Mosquito, deve ser editado o arquivo "mosquitto.conf", utilizando o código:
```
sudo nano /etc/mosquitto/mosquitto.conf
```
E acrescentar ao final do arquivo de texto as seguintes linhas:
> listener 1883
> 
> allow_anonymous true

Por fim, reiniciar o Mosquito com o código:
```
sudo systemctl restart mosquitto
```

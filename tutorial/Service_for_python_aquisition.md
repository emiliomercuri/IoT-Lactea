# Como criar um serviço Linux que roda um código Python em background

Para criar um serviço no sistema operacional Linux que execute um Shell Script na inicialização, siga estas etapas:

## Criar um Shell Script executável

Dentro de uma pasta, crie o script que conterá o código Python. Aqui será utilizado o sensor de umidade e temperatura DHT11 como exemplo para coleta de dados:
```bash
micro aquisicao_dht11.sh
```
Dentro do editor Micro, escreva os seguintes comandos:
```bash
#!/bin/bash
cd ~/dht11
source env/bin/activate
python3 mqtt.py
```
Nesse caso, a pasta contendo o script é "dht11", e "mqtt" é um programa em Python que realiza a coleta dos dados. Após isso, salve as alterções utilizando "Ctrl + S" e saia do editor usando "Ctrl + Q"

Agora, para dar permissão total de execução do script digite no terminal:
```bash
chmod 777 aquisicao_dht11.sh
```
## Criar o seviço no systemd

Utilizando o editor micro, crie o serviço personalizado:

```bash
sudo micro /etc/systemd/system/myscript_dht11.service
```
Utilize as seguintes linhas de código dentro do editor.

```bash
[Unit]
Description=Run my dht11 acquisition script at boot
After=network.target

[Service]
ExecStart=/home/usuario/dht11/aquisicao_dht11.sh
Type=simple
Restart=always
User=usuario

[Install]
WantedBy=multi-user.target
```
Aqui, "dht11" é o nome do sensor exemplo, e "/home/usuario/dht11/aquisicao_dht11.sh" é o endereço onde está localizado o Shell Script criado anteriormente. No seu caso, altere essas informações conforme necessário.

## Registrar e habilitar o serviço

Para atualizar os serviços do systemd, no terminal, rode:
```bash
sudo systemctl daemon-reload
```
Faça com que o serviço seja iniciado com o sistema operacional:
```bash
sudo systemctl enable myscript_dht11.service
```
Agora, reinicie o sistema:
```bash
sudo reboot
```
Então, verifique se o serviço foi habilitado automaticamente.
```bash
sudo systemctl status myscript_dht11.service
```

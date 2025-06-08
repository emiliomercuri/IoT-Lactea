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

Para dar permissão total de execução do script digite:
```bash
chmod 777 aquisicao_dht11.sh
```



# Gerando uma rede Wifi através do Raspberry Pi

Para gerar uma rede wifi no Raspberry Pi, utilize os seguintes passos:

## Configuração do adaptador Wlan0
Para verificar o status do adaptador/placa wifi, utilize o código:
```
nmcli device
```

Caso o dispositivo *wlan0* esteja desconectado, deverá ser ativado pelo seguinte código:
```
sudo nmcli device wifi hotspot ssid <hotspot name> password <hotspot password> ifname wlan0
```

## Tornar prioritário o processo de gerar o wifi sempre que houver um boot no Raspberry Pi

Para garantir que após um boot, o Raspberry volte a gerar a rede wifi, utilize os seguintes passos:

Encontrar as conexões atuais:
```
nmcli connection
```

Encontre o dispositivo *wlan0* e copie a identificação **UUID**.

Para visualizar o status desta conexão, utilize o seguinte código:
```
nmcli connection show <hotspot UUID>
```

Caso o status esteja como:
> connection.autoconnect:                 no
> connection.autoconnect-priority:        0

Então será necessário alterar este status com o seguinte código:
```
sudo nmcli connection modify <hotspot UUID> connection.autoconnect yes connection.autoconnect-priority 100
```

Para verificar se o status da auto conexão foi alterado, execute novamente o comando:
```
nmcli connection show <hotspot UUID>
```

E o status observado deve ser:
> connection.autoconnect:                 yes
> connection.autoconnect-priority:        100

# Integração entre a Plataforma IoT Empire + sensor de CO2 SCD4x + MQTT

## Instalando IoT Empire

Para verificar os requerimentos para a instalação da plataforma, execute o seguinte código:
```
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
```

Para fazer o donwload, execute o código:
```
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
```
Instalando a PlatformIo utilizando Python 3:
```
python3 get-platformio.py
```

Utilizando um editor de texto, neste caso será utilizado o editor *Micro*, edite a pasta oculta *.profile* com o comando:
```
micro .profile
```
E acrescente ao final do código a seguinte linha:
```
export PATH=$PATH:$HOME/.local/bin
```
Para salvar, aperte *Ctrl+S* e para fechar, aperte *Ctrl+Q*

Após estes passos, crie uma pasta chamada *bin* com o seguinte comando:
```
mkdir bin
```
Posteriormente, mova a pasta para os diretorios da PlatformIo com os seguinte comandos:
```
ln -s ~/.platformio/penv/bin/platformio ~/.local/bin/platformio
ln -s ~/.platformio/penv/bin/pio ~/.local/bin/pio
ln -s ~/.platformio/penv/bin/piodebuggdb ~/.local/bin/piodebuggdb
```

Criar um diretório para o desenvolvimentos dos estudos utilizando a PlatformIO:
```
mkdir platformio-learn
```

## SCD4x

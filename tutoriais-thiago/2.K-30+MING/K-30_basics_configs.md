Para fazer as configurações iniciais, siga os seguintes passos:

**Boa prática**, realizar as atualizações do Raspberry antes de começar. Utilize o código:
```
sudo apt update && sudo apt upgrade
```
E posteriormente reinicie o Reaspberry:
```
sudo reboot
```

Crie uma pasta para realizar as configurações:

```
mkdir k-30
```
**Boa prática** é criar um diretório para as configurações utilizadas no Python separado do diretório das configurações do Arduino.

# Instalar o pacote para o Arduino

Para o Raspberry poder identificar o Arduíno, é necessário baixar os pacotes específicos. Para este caso, estamos utilizando o Arduino Nano, logo, utilizaremos os pacotes relacionados à ele.

Primeiro, encontre a porta USB que está sendo utilizada pelo Arduino:
```
ls /dev/ttyUSB*
```
Caso esteja somente o Arduino conectado, deverá aparecer algo como:
```
/dev/ttyUSB0
```

## Instalar o pacote para o Arduino Nano

Utilize o seguinte código:
```
sudo apt install minicom
minicom -b 9600 -o -D /dev/ttyUSB0
```

O seguinte comando mostra os dados recebidos:
```
dmesg | tail
```
Para sair do **minicom**, precione *Crtl + A* e depois *X*.

# Instalar o Python no Raspberry

Agora vamos instalar o Python3 no Raspberry, pois através dele iremos poder ler e salvar os dados do sensor. Para instalar o Python, utilize o seguinte código:
```
sudo apt update
sudo apt install python3-serial
```
Para ter certeza de que houve a instalação, verifique a versão do Python instalada, através do seguinte código:
```
python3 -c "import serial; print(serial.__version__)"
```

# Instalar o IDE do Arduino no Raspberry

Esta etapa permite carregar códigos no Arduino através do IDE.
Utilize o seguinte código:
```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

## Colocar o PATH:
```
mkdir -p ~/.local/bin
mv ~/bin/arduino-cli ~/.local/bin/
```

Edite o .bashrc:
```
micro ~/.bashrc
```

Adicione ao final do código:
```
export PATH="$HOME/.local/bin:$PATH"
```

Após salvar e sair, recarregue:
```
source ~/.bashrc
```

Para verificar se a instalação foi bem sucedida, visualize a versão:
```
arduino-cli version
```
Deverá aparecer algo como:
```
arduino-cli Version: 0.35.x
```

## Configurar a placa

Iniciar a congiguração:
```
arduino-cli config init
```

Atualizar os índices:
```
arduino-cli core update-index
arduino-cli core install arduino:avr
```

Detectar a placa:
```
arduino-cli board list
```

Pode ser observada as opções disponíveis também:
```
arduino-cli board listall nano
```

Dentro do diretório das configurações para o Arduino, crie o seguinte arquivo:
```
micro k-30-arduino.ino
```
Então cole o código: (codes/k-30+Arduino.cpp)




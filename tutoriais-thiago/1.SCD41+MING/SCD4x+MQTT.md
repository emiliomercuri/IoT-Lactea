# Integração entre a PlatformIO + sensor de CO2 SCD4x + MQTT, utilizando o microcontrolador D1 Mini - ESP8266

## Instalando a PlatformIO

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

Fazer um reboot do Raspberry com:
```
sudo reboot
```
Para ver se a PlatformIO está instalada corretamente, basta executar:
```
pio
```

Criar um diretório para o desenvolvimento dos estudos utilizando a PlatformIO:
```
mkdir platformio-learn
```

## D1 Mini ESP38266
Primeiramente iremos verificar se o Raspberry está identificando o microcontrolador ESP38266. Para tanto, criamos uma diretório específico para este teste:
```
mkdir wemos-blink
```
Neste diretório, criamos um arquivo de inicialização com as informações da placa D1 mini e das bibliotecas necessárias. Para criar o arquivo, usamos o código:
```
micro platformio.ini
```
E digitamos o seguinte texto:
```
[env:d1_mini]
platform = espressif8266
board = d1_mini
framework = arduino
```
Para salvar e fechar, teclamos *Ctrl+S* e *Ctrl+Q*.

Ainda no diretório **wemos-blink**, criamos o diretório *src* e o arquivo *main*, com os seguintes códigos:
```
mkdir src
```
Dentro do diretório *src*, criamos o arquivo de texto:
```
micro main.cpp
```
Neste arquivo, colocamos o seguinte [código:] (https://github.com/emiliomercuri/IoT-Lactea/blob/main/codes/wemo-d1_mini-blink.cpp)




## SCD4x


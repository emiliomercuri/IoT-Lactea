# Guia de Instalação: Raspberry Pi OS Lite (64-bit) para Monitoramento Ambiental

Este tutorial detalha o processo de instalação do sistema operacional em um **Raspberry Pi 3B+**, configurado para atuar em uma rede de sensores IoT (monitoramento de NO2, O3 e Material Particulado).

## 1. Instalação do Raspberry Pi Imager
O primeiro passo é baixar a ferramenta oficial de escrita de imagem da Raspberry Pi:
* **Link para Download:** [raspberrypi.com/software](https://www.raspberrypi.com/software/)

## 2. Configuração do Sistema Operativo e Dispositivo
Ao abrir o Imager, siga estas seleções:

1.  **Escolher Dispositivo:** Selecione `Raspberry Pi 3`.
2.  **Escolher SO:** Vá em `Raspberry Pi OS (other)` -> `Raspberry Pi OS Lite (64-bit)`.
    > **Nota Técnica:** A versão **Lite** não possui interface gráfica. Para um projeto de sensores que rodará scripts Python e MQTT 24/7, isso é ideal, pois economiza memória RAM e processamento, aumentando a estabilidade do sistema. A arquitetura **64 bits** permite o uso de bibliotecas de análise de dados mais modernas.
3.  **Escolher Armazenamento:** Selecione o seu cartão SD (ex: 16GB).

## 3. Personalização do Sistema (OS Customization)
Após clicar em "Seguinte", selecione **"EDITAR DEFINIÇÕES"**. Esta etapa é crucial para o funcionamento "headless" (sem monitor).

### Hostname
* Defina como: `lactea3-daniel`
* **Por que mudar o Hostname?** Em projetos de IoT com múltiplos dispositivos, o Hostname permite que você encontre o Raspberry na rede digitando `lactea3-daniel.local` em vez de precisar descobrir o endereço IP variável do dispositivo.

### Utilizador e Localização
* Defina um **nome de utilizador** e uma **palavra-passe** segura.
* Em **Localização**, certifique-se de que o fuso horário e o layout do teclado estão corretos (importante para que os logs de dados dos sensores tenham o carimbo de tempo/timestamp correto).

### Serviços e Conectividade
* **Wi-Fi:** Pode ser ignorado se você for utilizar o cabo Ethernet. O cabo oferece uma conexão mais estável para transmissão de dados de sensores.
* **SSH:** Ative o SSH e selecione "Utilizar autenticação por palavra-passe". Isso permite que você acesse o terminal do Raspberry de qualquer outro computador na mesma rede.
* **Raspberry Pi Connect:** Ative esta opção para acesso remoto simplificado.
    1. Clique em "Abrir Raspberry Pi Connect".
    2. Você será levado ao navegador para gerar um **Token de autenticação**.
    3. Cole o token (ex: `rpuak_...`) no campo correspondente no Imager.

## 4. Gravação da Imagem
1.  Clique em **"GRAVAR"**.
2.  Um aviso aparecerá informando que **todos os dados existentes no cartão SD serão apagados**.
3.  Confirme em **"SIM"** (ou "EU COMPREENDO, APAGAR E GRAVAR").
4.  Aguarde o processo de gravação e a verificação final.

---
*Tutorial gerado para documentação de Iniciação Científica - 2026.*

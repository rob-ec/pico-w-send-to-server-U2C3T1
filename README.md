# Pico W com Servidor Node.js - Leitura de Joystick e Botões

Este projeto demonstra a comunicação entre um microcontrolador Raspberry Pi Pico W e um servidor Node.js. O Pico W lê os estados de dois botões e de um joystick, e envia esses dados para um servidor que os armazena em um banco de dados SQLite.

## Autor

* **Aluno:** Robson Mesquita Gomes
* **Matrícula:** 202510110980872
* **Mentora:** Gabriela

## 🎯 Objetivos do Projeto

O objetivo principal é realizar a leitura dos status dos botões e da posição do joystick da placa Bitdoglab e visualizar esses dados em um servidor. Para isso, o projeto foi dividido em duas partes principais:

1.  **Sistema Embarcado (Pico W):** Responsável pela leitura dos componentes de hardware.
2.  **Servidor (Node.js):** Responsável por receber, armazenar e exibir os dados.

## 📁 Link do Repositório

Todos os códigos e arquivos do projeto estão disponíveis no GitHub:
[https://github.com/rob-ec/pico-w-send-to-server-U2C3T1.git](https://github.com/rob-ec/pico-w-send-to-server-U2C3T1.git)

## 🛠️ Arquitetura e Funcionamento

### Servidor

O servidor foi desenvolvido em Node.js e atua como uma API simples. Ele utiliza o SQLite para armazenar os dados recebidos do Pico W.

* **Banco de Dados:** A tabela `logs` armazena os seguintes campos: `id`, `timestamp`, `button1`, `button2`, `joystick_pressed`, `joystick_x` e `joystick_y`.

* **Endpoints da API:**
    * `GET /`: Retorna uma mensagem "Hello World" para verificação de status.
    * `POST /log`: Armazena os dados recebidos via POST.
    * `GET /logs`: Retorna todos os registros armazenados no banco de dados.
    * `GET /get-log`: Endpoint criado para receber os dados do Pico W via GET, simplificando o envio pelo microcontrolador.

### Sistema Embarcado (Pico W)

O código embarcado, escrito em C/C++, é responsável por:

1.  **Inicializar** os pinos GPIO para os botões e o conversor analógico-digital (ADC) para o joystick.
2.  **Conectar-se** a uma rede Wi-Fi.
3.  **Ler** o estado dos botões e os valores dos eixos X e Y do joystick em um loop contínuo.
4.  **Enviar** os dados coletados para o endpoint `/get-log` do servidor a cada 1000ms através de uma requisição HTTP GET.

## 🚀 Como Executar

### Pré-requisitos

* Node.js e npm instalados.
* Ambiente de desenvolvimento para Raspberry Pi Pico configurado (Pico SDK, CMake, etc.).
* Placa Bitdoglab ou hardware equivalente com botões e joystick.

### Configuração do Servidor

1.  Clone o repositório:
    ```bash
    git clone [https://github.com/rob-ec/pico-w-send-to-server-U2C3T1.git](https://github.com/rob-ec/pico-w-send-to-server-U2C3T1.git)
    cd pico-w-send-to-server-U2C3T1/server
    ```
2.  Instale as dependências:
    ```bash
    npm install
    ```
3.  Inicie o servidor:
    ```bash
    npm start
    ```
    O servidor estará rodando em `http://localhost:3000` e no terminal também mostrará em qual IP da rede está rodando.

### Configuração do Sistema Embarcado

1.  Abra o diretório do código do Pico W.
2.  **Importante:** Altere o IP do servidor no arquivo `main.c` para o IP da máquina onde o servidor Node.js está rodando:
    ```c
    #define SERVER_HOST "SEU_IP_AQUI"
    ```
3.  **Importante:** Configure as credenciais da sua rede Wi-Fi no arquivo `CMakeLists.txt` ou em um arquivo `secrets.h` separado.
    ```cmake
    set(WIFI_SSID "NOME_DA_SUA_REDE")
    set(WIFI_PASSWORD "SENHA_DA_SUA_REDE")
    ```
4.  Compile e envie o código para o Raspberry Pi Pico W.

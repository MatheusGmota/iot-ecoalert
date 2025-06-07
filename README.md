# Projeto EcoAlert

A ideia do projeto consiste em fornecer status climáticos de diferentes regiões, e alertas de eventos extremos de acordo com os dados recebidos de nossos sensores IoT, que capturam dados de diversos lugares. Assim ajudando a população a se preparar contra esses eventos. Nosso IoT é capaz de capturar dados como: umidade do ar, temperatura, nivel da água de rios (para prevenir contra enchentes iminentes), e capaz de enviar esses dados a nossa API e também fornecer dados ao Thinger.io, onde é possivel visualizar os dados em dashboards personalizados.

## 👩‍👦‍👦 Equipe
* Felipe Seiki Hashiguti - RM98985
* Lucas Corradini Silveira - RM555118
* Matheus Gregorio Mota - RM557254

## Tecnologias Utilizadas

* **ESP32:** Um microcontrolador versátil com Wi-Fi e Bluetooth, atuando como o **Gateway Principal** que recebe os dados (simulados) das âncoras e os envia para a nuvem.
* **Thinger.io:** Plataforma de IoT que permite conectar dispositivos, visualizar dados em tempo real, criar dashboards personalizados e configurar alertas. É usada no projeto para armazenar e monitorar remotamente os dados coletados pelos sensores.
* **PlatformIO (VS Code Extension):** Um ambiente de desenvolvimento integrado para embarcados que facilita a compilação e o gerenciamento de bibliotecas para o ESP32.
* **Wokwi (VS Code Extension):** Um simulador de hardware online que permite rodar o código do ESP32 e ver o circuito em ação sem a necessidade de um dispositivo físico.
* **Sensores:**
  * DHT (Digital Humidity and Temperature): Sensor que mede temperatura e umidade do ambiente.
  * Ultrasonic Distance Sensor: Sensor que mede a distância de objetos próximos, útil para detecção de presença ou obstáculos. 

## Como Rodar o Projeto

### Pré-requisitos

* **VS Code:** Instale o Visual Studio Code.
* **PlatformIO IDE Extension:** Instale a extensão "PlatformIO IDE" no VS Code.
* **Wokwi for PlatformIO Extension:** Instale a extensão "Wokwi for PlatformIO" no VS Code.
* **Conta no** [http://Thinger.io](http://Thinger.io "smartCard-inline") **:**
  * Crie uma conta gratuita em [https://thinger.io](https://thinger.io "smartCard-inline") .
  * Crie um novo dispositivo na plataforma.
  * Copie as credenciais do dispositivo (Device ID, Username e Device Token) — você precisará incluí-las no arquivo `arduino_secrets.h` para autenticação e envio de dados.
### Configuração do Projeto

1.  **Clone ou Baixe o Projeto:**
    ```bash
    git clone https://github.com/MatheusGmota/iot-ecoalert.git
    cd iot-ecoalert
    ```

### Rodando a Simulação

1.  **Compilar o Projeto:**
    * No VS Code, na barra inferior do PlatformIO, clique no ícone de **"Build" (o visto)** ou use `Ctrl+Alt+B`.

2.  **Iniciar o Simulador Wokwi:**
    * Clique no ícone verde de play
    * Pressione `F1` (ou `Ctrl+Shift+P`) para abrir a Paleta de Comandos.
    * Digite `Wokwi: Start Simulator` e selecione-o.

4.  **Interagir com a Simulação:**
    * A janela do simulador Wokwi aparecerá no VS Code. Nela, você verá o ESP32 e os botões.
    * No painel inferior da janela do simulador Wokwi, clique na aba **"Serial Monitor"**. Você verá as mensagens de inicialização do ESP32.
    * Selecione os sensores e escolha a temperatura, umidade e nivel do sensor;

## 🧠 Estrutura e Lógica do Código

### Inicialização de sensores e periféricos:

```c++
DHT dht(DHTPIN, DHTTYPE);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
```
- Inicializa os objetos dos sensores e do [http://Thinger.io](http://Thinger.io "smartCard-inline")  com as credenciais fornecidas.

### Conexão Wi-Fi e [http://Thinger.io](http://Thinger.io "smartCard-inline") :
```c++
thing.add_wifi(SSID, SSID_PASSWORD);
```
- Conecta o ESP32 ao Wi-Fi para envio de dados.

### Leitura de temperatura e umidade:
```c++
float temp = dht.readTemperature(); 
float hum = dht.readHumidity(); 
```
- Captura dados do sensor DHT22. Se falhar, mostra erro no terminal.

### Leitura do sensor ultrassônico (nível de água):
```c++
unsigned int uS = sonar.ping();
float distanciaCm = sonar.convert_cm(uS);
```
- Mede a distância do sensor até a superfície da água.

### Cálculo da porcentagem de nível da água:
```c++
nivelAguaCm = ALTURA_SENSOR_ACIMA_DA_AGUA_VAZIA - distanciaCm;
porcentagemNivel = (nivelAguaCm / PROFUNDIDADE_TOTAL_RECIPIENTE) * 100.0;
```
- Calcula a altura da coluna d’água e sua porcentagem relativa ao recipiente.

### Acionamento de alertas:
```c++
if (temp > 39 || temp < -40 || hum > 70 || hum < 20) {
  ledcWrite(0, 128);  // Buzzer ativo
  digitalWrite(LED1, HIGH);  // LED aceso
}
```
- Se os valores estiverem fora do intervalo ideal, o buzzer e o LED são ativados intermitentemente.

### Envio de dados para o [http://Thinger.io](http://Thinger.io "smartCard-inline") :
```c++
thing.write_bucket("meu_bucket", data);
```
- Envia os dados como `pson` para um bucket no [http://Thinger.io](http://Thinger.io "smartCard-inline") .

### Envio de dados para a API Java:
```c++
HTTPClient http;
http.begin(JAVA_API_URL);
http.POST(jsonBody);
```
- Os dados também são enviados em formato JSON para uma API externa, simulando uma integração com backend.

## Fluxos com Node-RED
Este projeto não utiliza o Node-RED. Toda a comunicação entre dispositivos, gateway e dashboards é feita diretamente via ESP32 e integração com Thinger.io e API externa.

## 📷 Ilustrações do Projeto

### Circuito no Simulador Wokwi
![image](https://github.com/user-attachments/assets/988550e4-9b3a-45b8-86fc-88d76ce779a6)

### Monitor Serial da Simulação
![image](https://github.com/user-attachments/assets/2b4b76e0-e711-4471-8b06-cee1172d56c2)



/*
 * Programa revisado para CubeCell HTCC-AB01 con sensores DHT22 y MQ-2
 * Envía datos de temperatura, humedad y gas mediante LoRaWAN
 * Compatible con decodificador Node-RED
 */

#include "LoRaWanMinimal_APP.h"
#include "Arduino.h"
#include "DHT.h"

// ⚠️ IMPORTANTE: AT SUPPORT DEBE ESTAR EN OFF
#define LORAWAN_AT_SUPPORT OFF

// Definición de pines
#define DHT_PIN GPIO1    // DHT22 en GPIO1
#define MQ2_PIN ADC      // Pin del sensor MQ-2 (analógico)
#define LED_PIN GPIO2    // Pin del LED (opcional, si lo usas)

// Tipo de sensor DHT22
#define DHTTYPE DHT22

// Intervalo de envío
uint32_t TX_INTERVAL = 30000;  // 30 segundos

// Variables
float temperatura;
float humedad;
int valorGas;
static uint8_t counter = 0;

// Inicializar DHT
DHT dht(DHT_PIN, DHTTYPE);

// ================= TTN KEYS =================

static uint8_t appEui[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t devEui[] = {
  0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x51, 0xE2
};

static uint8_t appKey[] = {
  0x74, 0x73, 0x3B, 0x8E, 0x18, 0x00, 0x2F, 0x0E,
  0xEC, 0xD2, 0xE0, 0xB9, 0x70, 0xDF, 0x95, 0x62
};

// --------------------------------------------

// Máscara de canales LoRaWAN
uint16_t userChannelsMask[6] = { 
  0xFF00, // Sub-band 2: channels 8-15
  0x0000,
  0x0000,
  0x0000,
  0x0002, // 500kHz channel 65 for sub-band 2
  0x0000
};

// Sleep
TimerEvent_t sleepTimer;
bool sleepTimerExpired;

static void wakeUp() {
  sleepTimerExpired = true;
}

static void lowPowerSleep(uint32_t sleeptime) {
  sleepTimerExpired = false;
  TimerInit(&sleepTimer, &wakeUp);
  TimerSetValue(&sleepTimer, sleeptime);
  TimerStart(&sleepTimer);
  while (!sleepTimerExpired) lowPowerHandler();
  TimerStop(&sleepTimer);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando sistema de monitoreo con DHT22 y MQ-2 con LoRaWAN");

  pinMode(MQ2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  Serial.println("Calentando sensor MQ-2...");
  delay(20000);
  Serial.println("Sensor MQ-2 listo");

  // Definir la región LoRaWAN (Asegúrate de definir la región correcta)
  #define ACTIVE_REGION LORAMAC_REGION_US915  // Usa la región correspondiente a tu red LoRaWAN

  // Configuración LoRaWAN
  LoRaWAN.begin(LORAWAN_CLASS, ACTIVE_REGION);  // LoRaWAN.CLASS se configura por defecto
  LoRaWAN.setAdaptiveDR(true);

  // Intentar conexión a LoRaWAN
  while (1) {
    Serial.print("Conectando a LoRaWAN... ");
    LoRaWAN.joinOTAA(appEui, appKey, devEui);

    if (!LoRaWAN.isJoined()) {
      Serial.println("FALLÓ. Reintentando en 30s...");
      lowPowerSleep(30000);
    } else {
      Serial.println("CONECTADO EXITOSAMENTE");
      break;
    }
  }
}

void loop() {
  counter++;

  // Leer datos de los sensores
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature();
  valorGas = analogRead(MQ2_PIN);

  // Verificación en el Serial Monitor
  Serial.println("\n--- Lecturas ---");

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error lectura DHT22");
  } else {
    Serial.printf("Humedad: %.1f %% | ", humedad);
    Serial.printf("Temperatura: %.1f C\n", temperatura);
  }

  Serial.print("Gas MQ-2: ");
  Serial.println(valorGas);

  // Umbral para gas MQ-2 (700)
  if (valorGas > 700) {
    digitalWrite(LED_PIN, HIGH);  // Activar alerta si supera el umbral
  } else {
    digitalWrite(LED_PIN, LOW);  // Apagar alerta si no supera el umbral
  }

  // Umbral de temperatura (40°C)
  if (temperatura > 40) {
    // Activar alerta si la temperatura supera los 40°C
    Serial.println("¡ALERTA! Temperatura alta detectada.");
  }

  // Umbral de humedad (30%)
  if (humedad < 30) {
    // Activar alerta si la humedad es baja (condiciones secas)
    Serial.println("¡ALERTA! Humedad baja detectada.");
  }

  // Enviar los datos a LoRaWAN
  uint8_t buffer[6];

  int16_t t = temperatura * 10;
  buffer[0] = t >> 8;
  buffer[1] = t & 0xFF;

  uint16_t h = humedad * 10;
  buffer[2] = h >> 8;
  buffer[3] = h & 0xFF;

  buffer[4] = valorGas >> 8;
  buffer[5] = valorGas & 0xFF;

  // Confirmar que estamos enviando el paquete
  Serial.printf("Enviando paquete #%d\n", counter);
  LoRaWAN.send(6, buffer, 6, true);

  // Imprimir que entró en el ciclo de dormir
  Serial.printf("Durmiendo %d s...\n", TX_INTERVAL / 1000);
  lowPowerSleep(TX_INTERVAL);
}

// Función para manejar el downlink (respuesta de LoRaWAN)
void downLinkDataHandle(McpsIndication_t *mcpsIndication) {

  Serial.printf("Mensaje recibido, puerto %d: ", mcpsIndication->Port);
  
  for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++) {
    Serial.printf("%02X ", mcpsIndication->Buffer[i]);
  }
  Serial.println();

  if (mcpsIndication->BufferSize > 0) {
    switch (mcpsIndication->Buffer[0]) {

      case 0x01: {
        uint16_t newInterval = (mcpsIndication->Buffer[1] << 8) |
                                mcpsIndication->Buffer[2];
        TX_INTERVAL = newInterval * 1000;
        Serial.printf("Nuevo intervalo: %d segundos\n", newInterval);
      } break;

      case 0x02:
        Serial.println("Reinicio remoto...");
        delay(1000);
        CySoftwareReset();
        break;

      default:
        Serial.println("Comando no reconocido");
        break;
    }
  }
}
# LoRaSanturban
Sistema IoT para Detección Temprana de Incendios Forestales mediante Monitoreo Ambiental y Comunicación LoRaWAN

## Descripción

LoRaSanturban es un prototipo de sistema IoT desarrollado como apoyo a un proyecto de tesis enfocado en la detección temprana de condiciones asociadas a incendios forestales.  

El sistema integra sensores ambientales con comunicación LoRaWAN de largo alcance y bajo consumo energético, permitiendo el monitoreo remoto de variables críticas en tiempo real.

## Objetivo

Diseñar e implementar un sistema de monitoreo ambiental basado en IoT capaz de identificar condiciones de riesgo relacionadas con incendios forestales y generar alertas automáticas para facilitar una respuesta oportuna.

## Arquitectura del Sistema

Sensores (DHT22, MQ-2)  
↓  
CubeCell HTCC-AB01  
↓  
Red LoRaWAN (US915)  
↓  
The Things Network (TTN)  
↓  
Node-RED  
↓  
Sistema de alertas (Telegram)

## Variables Monitoreadas

- Temperatura
- Humedad relativa
- Concentración de gas

## Tecnologías Utilizadas

- CubeCell HTCC-AB01
- LoRaWAN
- The Things Network (TTN)
- Node-RED
- Telegram Bot API
- Arduino Framework

## Funcionamiento General

El dispositivo captura periódicamente datos ambientales y los transmite a través de LoRaWAN.  
Node-RED procesa la información, evalúa umbrales definidos y clasifica el nivel de riesgo.  
Cuando se detectan condiciones críticas, el sistema envía alertas automáticas con control de frecuencia para evitar notificaciones redundantes.

## Estado del Proyecto

Prototipo funcional en fase de validación experimental como parte del trabajo de investigación.

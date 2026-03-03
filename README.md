# MuOnlineHttpApi

**Descripción:**  
Este proyecto añade un API HTTP para Mu Online que registra eventos de PvP en tiempo real, específicamente los kills y deaths de jugadores, y los expone en formato JSON para su consumo en un frontend o dashboard web. Es independiente de la base de datos del servidor y funciona solo como un sistema de notificaciones en memoria.

---

## Archivos modificados

- `GameServer/User.cpp`  
  - Se añadió la llamada al API en la función `gObjUserDie` para enviar los datos del killer y dead.  
- `GameServer/GameMain.cpp`  
  - Se añadieron las inicializaciones necesarias para que el `HttpApi` funcione al iniciar el servidor.  
- `GameServer/HttpApi.h` y `GameServer/HttpApi.cpp`  
  - Código del API HTTP en C++ usando sockets nativos de Windows.  
  - Mantiene un límite máximo de 100 eventos en memoria.  

> Nota: Solo se modificaron partes específicas de User.cpp y GameMain.cpp. Otros devs pueden adaptar estas modificaciones a su propio source si difiere la estructura de objetos.

---
### Características
* **Registro de eventos PvP en tiempo real**: captura los kills y deaths entre jugadores directamente en memoria del GameServer.
* **Límite máximo de 100 eventos** para mantener bajo consumo de memoria y evitar sobrecarga.
* **Independiente de la base de datos del servidor**: no altera ni depende de la DB principal del MU Online.
* **Exposición vía HTTP/JSON**: los eventos pueden consultarse desde un frontend, panel de control o sistema de notificaciones.
* **Asíncrono y no bloqueante**: el envío de datos al API no interfiere con la ejecución normal del GameServer.
* **Compatible con múltiples modos de juego**: respeta los eventos de mapas especiales, PvP, KillAll, TvT y GvG.
* **Registro interno de logs**: cada evento de kill/death se registra en el log del servidor para auditoría y debug.